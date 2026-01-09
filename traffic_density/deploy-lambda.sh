#!/bin/bash
set -euo pipefail

# Lambda Deployment Script for Traffic Detection
# This script builds the Docker image and deploys to AWS Lambda

# Configuration
AWS_REGION="${AWS_REGION:-us-east-1}"
AWS_PROFILE="${AWS_PROFILE:-maisumdia-pipe}"
PROJECT_NAME="${PROJECT_NAME:-security-cam-detector}"
ENVIRONMENT="${ENVIRONMENT:-dev}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get ECR repository URL from Terraform
get_ecr_url() {
    cd infra/terraform
    ECR_URL=$(terraform output -raw ecr_repository_url 2>/dev/null)
    if [ -z "$ECR_URL" ] || [ "$ECR_URL" = "null" ]; then
        log_error "ECR repository not found. Run 'terraform apply' first."
        exit 1
    fi
    cd ../..
    echo "$ECR_URL"
}

# Get Lambda function name from Terraform
get_lambda_name() {
    cd infra/terraform
    LAMBDA_NAME=$(terraform output -raw lambda_function_name 2>/dev/null)
    if [ -z "$LAMBDA_NAME" ] || [ "$LAMBDA_NAME" = "null" ]; then
        log_error "Lambda function not found. Run 'terraform apply' first."
        exit 1
    fi
    cd ../..
    echo "$LAMBDA_NAME"
}

# Main deployment flow
main() {
    log_info "Starting Lambda deployment process..."
    
    # Get infrastructure details
    ECR_REPO_URL=$(get_ecr_url)
    LAMBDA_FUNCTION=$(get_lambda_name)
    
    log_info "ECR Repository: $ECR_REPO_URL"
    log_info "Lambda Function: $LAMBDA_FUNCTION"
    
    # Step 1: Login to ECR
    log_info "Logging into Amazon ECR..."
    aws ecr get-login-password --region "$AWS_REGION" --profile "$AWS_PROFILE" | \
        docker login --username AWS --password-stdin "$ECR_REPO_URL"
    
    # Step 2: Build Docker image
    log_info "Building Docker image for Lambda..."
    cd ..
    
    # the parameter provenance is added to fix this issue https://stackoverflow.com/questions/65608802/cant-deploy-container-image-to-lambda-function

    docker build -t  "${PROJECT_NAME}-lambda:latest" -f traffic_density/Dockerfile.lambda . --provenance=false
    
    # Step 3: Tag image
    log_info "Tagging Docker image..."
    docker tag "${PROJECT_NAME}-lambda:latest" "${ECR_REPO_URL}:latest"
    
    # Step 4: Push to ECR
    log_info "Pushing image to ECR (this may take a few minutes)..."
    docker push "${ECR_REPO_URL}:latest"
    
    # Step 5: Update Lambda function
    log_info "Updating Lambda function code..."
    aws lambda update-function-code \
        --function-name "$LAMBDA_FUNCTION" \
        --image-uri "${ECR_REPO_URL}:latest" \
        --region "$AWS_REGION" \
        --profile "$AWS_PROFILE" > /dev/null
    
    # Wait for update to complete
    log_info "Waiting for Lambda update to complete..."
    aws lambda wait function-updated \
        --function-name "$LAMBDA_FUNCTION" \
        --region "$AWS_REGION" \
        --profile "$AWS_PROFILE"
    
    log_info "✅ Deployment completed successfully!"
    
    # Show next steps
    echo ""
    echo "========================================="
    echo "🚀 NEXT STEPS"
    echo "========================================="
    echo "1. Test the Lambda function:"
    echo "   aws lambda invoke \\"
    echo "     --function-name $LAMBDA_FUNCTION \\"
    echo "     --payload '{\"avenue_name\": \"Avenida dos Estados\", \"mode\": \"test\"}' \\"
    echo "     --region $AWS_REGION \\"
    echo "     --profile $AWS_PROFILE \\"
    echo "     response.json"
    echo ""
    echo "2. View logs:"
    echo "   aws logs tail /aws/lambda/$LAMBDA_FUNCTION --follow \\"
    echo "     --region $AWS_REGION --profile $AWS_PROFILE"
    echo ""
    echo "3. Check EventBridge schedule:"
    echo "   aws events list-rules --region $AWS_REGION --profile $AWS_PROFILE"
    echo "========================================="
}

# Run main function
main "$@"
