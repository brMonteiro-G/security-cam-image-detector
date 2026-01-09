# AWS Lambda Deployment Guide

This guide explains how to deploy the C++ traffic detection application to AWS Lambda using Docker container images.

## Architecture Overview

```
Camera Stream → Lambda (Container) → Process Image → Publish to SNS → Email Notifications
                    ↓                      ↓
                CloudWatch Logs      S3 (Optional)
```

## Prerequisites

1. **Terraform Infrastructure Deployed**
   ```bash
   cd traffic_density/infra/terraform
   terraform apply
   ```

2. **Docker Installed**
   ```bash
   docker --version  # Should be 20.10+
   ```

3. **AWS CLI Configured**
   ```bash
   aws configure --profile maisumdia-pipe
   aws sts get-caller-identity --profile maisumdia-pipe
   ```

## Quick Start

### Option 1: Automated Deployment (Recommended)

```bash
cd traffic_density
./deploy-lambda.sh
```

This script will:
- Login to ECR
- Build Docker image optimized for Lambda
- Push image to ECR
- Update Lambda function
- Show testing instructions

### Option 2: Manual Deployment

#### Step 1: Get ECR Repository URL

```bash
cd infra/terraform
terraform output ecr_repository_url
# Example: 222634361357.dkr.ecr.us-east-1.amazonaws.com/security-cam-detector-lambda-dev
```

#### Step 2: Login to ECR

```bash
aws ecr get-login-password --region us-east-1 --profile maisumdia-pipe | \
  docker login --username AWS --password-stdin <ECR_REPO_URL>
```

#### Step 3: Build Lambda Docker Image

```bash
cd /Users/gabe/projects/ufabc/security-cam-image-detector
docker build -t security-cam-detector-lambda -f traffic_density/Dockerfile.lambda .
```

**Build time:** ~15-20 minutes (downloads OpenCV, AWS SDK, YOLO models)

#### Step 4: Tag and Push Image

```bash
ECR_URL=$(cd traffic_density/infra/terraform && terraform output -raw ecr_repository_url)
docker tag security-cam-detector-lambda:latest ${ECR_URL}:latest
docker push ${ECR_URL}:latest
```

**Push time:** ~5-10 minutes (depends on image size, ~500MB)

#### Step 5: Update Lambda Function

```bash
LAMBDA_NAME=$(cd traffic_density/infra/terraform && terraform output -raw lambda_function_name)

aws lambda update-function-code \
  --function-name ${LAMBDA_NAME} \
  --image-uri ${ECR_URL}:latest \
  --region us-east-1 \
  --profile maisumdia-pipe
```

## Testing

### Test Lambda Manually

```bash
LAMBDA_NAME=$(cd traffic_density/infra/terraform && terraform output -raw lambda_function_name)

aws lambda invoke \
  --function-name ${LAMBDA_NAME} \
  --payload '{"avenue_name": "Avenida dos Estados", "mode": "test"}' \
  --region us-east-1 \
  --profile maisumdia-pipe \
  response.json

cat response.json
```

### View Lambda Logs

```bash
aws logs tail /aws/lambda/${LAMBDA_NAME} --follow \
  --region us-east-1 --profile maisumdia-pipe
```

### Check Scheduled Executions

```bash
# View EventBridge rule
aws events describe-rule \
  --name security-cam-detector-schedule-dev \
  --region us-east-1 \
  --profile maisumdia-pipe

# View execution history
aws lambda get-function \
  --function-name ${LAMBDA_NAME} \
  --region us-east-1 \
  --profile maisumdia-pipe
```

## Configuration

### Environment Variables (Set via Terraform)

- `AWS_SNS_TOPIC_ARN`: SNS topic for notifications
- `CAMERA_STREAM_URL`: Camera stream URL to process
- `S3_BUCKET_NAME`: S3 bucket for image storage (optional)
- `ENVIRONMENT`: dev/staging/prod
- `ENABLE_AWS_SNS`: "true" to enable notifications

### Adjust Schedule (EventBridge)

Edit `terraform.tfvars`:

```hcl
lambda_schedule_expression = "rate(10 minutes)"  # Every 10 minutes
# OR
lambda_schedule_expression = "cron(0 * * * ? *)"  # Every hour
```

Then apply:

```bash
cd traffic_density/infra/terraform
terraform apply
```

### Increase Lambda Resources

Edit `terraform.tfvars`:

```hcl
lambda_memory_size       = 3008      # MB (max for CPU optimization)
lambda_timeout           = 900       # seconds (15 min max)
lambda_ephemeral_storage = 2048      # MB for /tmp
```

## Lambda Container Details

### Base Image
- `public.ecr.aws/lambda/provided:al2023` (AWS Lambda Runtime Interface)

### Key Components
1. **OpenCV 4.x**: For image processing and DNN
2. **YOLO v3**: Object detection model
3. **AWS SDK C++**: SNS client
4. **Custom Bootstrap**: Lambda Runtime Interface implementation

### Image Size
- **Compressed**: ~200MB
- **Uncompressed**: ~500MB

### Execution Flow
1. Lambda invokes `/var/runtime/bootstrap`
2. Bootstrap fetches event from Runtime API
3. Executes `/opt/traffic_detector` binary
4. Processes camera frame
5. Publishes results to SNS
6. Returns response to Runtime API

## Troubleshooting

### Issue: Image Push Timeout

**Solution:** Use faster internet or enable ECR VPC endpoint

```bash
# Check image size
docker images | grep security-cam-detector-lambda

# Compress layers
docker build --compress -t security-cam-detector-lambda -f traffic_density/Dockerfile.lambda .
```

### Issue: Lambda Timeout (Task timed out after X seconds)

**Solution:** Increase timeout in Terraform

```hcl
lambda_timeout = 900  # 15 minutes (max)
```

### Issue: Out of Memory

**Solution:** Increase memory allocation

```hcl
lambda_memory_size = 3008  # Max memory = more CPU
```

### Issue: Function Code Not Updating

**Solution:** Force image refresh

```bash
aws lambda update-function-code \
  --function-name ${LAMBDA_NAME} \
  --image-uri ${ECR_URL}:latest \
  --region us-east-1 \
  --profile maisumdia-pipe

# Wait for update
aws lambda wait function-updated \
  --function-name ${LAMBDA_NAME} \
  --region us-east-1 \
  --profile maisumdia-pipe
```

### Issue: Camera Stream Connection Failed

**Solution:** Lambda needs internet access. Check:

1. Lambda is **NOT** in VPC (or VPC has NAT Gateway)
2. Security groups allow outbound HTTPS
3. Camera URL is accessible from AWS region

```bash
# Test from Lambda
aws lambda invoke \
  --function-name ${LAMBDA_NAME} \
  --payload '{"mode": "test", "debug": true}' \
  response.json
```

## Cost Estimation

### Lambda Costs (us-east-1)
- **Memory**: 3008 MB
- **Duration**: ~30 seconds per execution
- **Executions**: Every 10 minutes = ~4,320/month

**Monthly Cost:**
- Compute: $0.0000166667 × 3008/1024 × 30 × 4320 = ~$6.50
- Requests: $0.20 per 1M requests × 4320 = ~$0.001
- **Total Lambda**: ~$6.50/month

### ECR Storage
- **Image Size**: 200MB compressed
- **Storage**: $0.10 per GB/month = ~$0.02/month

### SNS Notifications
- **4,320 publishes/month**: $0.50 per 1M = ~$0.002/month
- **Email deliveries**: Free

### Total Estimated Cost
**~$7/month** (with 10-minute scheduling)

Reduce costs by:
- Increasing schedule interval (e.g., every 30 minutes → ~$2/month)
- Using Reserved Concurrency (for predictable workloads)

## Advanced: CI/CD Pipeline

Create GitHub Actions workflow (`.github/workflows/deploy-lambda.yml`):

```yaml
name: Deploy Lambda

on:
  push:
    branches: [main]

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Configure AWS
        uses: aws-actions/configure-aws-credentials@v2
        with:
          aws-access-key-id: ${{ secrets.AWS_ACCESS_KEY_ID }}
          aws-secret-access-key: ${{ secrets.AWS_SECRET_ACCESS_KEY }}
          aws-region: us-east-1
      
      - name: Login to ECR
        run: |
          aws ecr get-login-password --region us-east-1 | \
            docker login --username AWS --password-stdin ${{ secrets.ECR_URL }}
      
      - name: Build and Push
        run: |
          docker build -t lambda -f traffic_density/Dockerfile.lambda .
          docker tag lambda:latest ${{ secrets.ECR_URL }}:latest
          docker push ${{ secrets.ECR_URL }}:latest
      
      - name: Update Lambda
        run: |
          aws lambda update-function-code \
            --function-name ${{ secrets.LAMBDA_NAME }} \
            --image-uri ${{ secrets.ECR_URL }}:latest
```

## Support

For issues:
1. Check CloudWatch Logs first
2. Review Terraform outputs: `terraform output`
3. Test locally with Docker: `docker run -it security-cam-detector-lambda /bin/bash`
4. Open GitHub issue with logs

---

**Next Steps:**
1. Run `./deploy-lambda.sh` to deploy
2. Check SNS email subscriptions and confirm
3. Monitor CloudWatch Logs for executions
4. Adjust schedule as needed
