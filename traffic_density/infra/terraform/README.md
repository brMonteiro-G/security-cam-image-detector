# Traffic Detection System - Terraform Infrastructure

This directory contains Terraform configurations to provision AWS infrastructure for the Security Camera Image Detector system.

## 📋 Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     Traffic Detection System                     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │   C++ Application │
                    │   (main_exec)     │
                    └──────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │   AWS SNS Topic   │
                    │   (FIFO)          │
                    └──────────────────┘
                              │
            ┌─────────────────┼─────────────────┐
            ▼                 ▼                 ▼
    ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
    │    Email     │  │     SMS      │  │  SQS Queue   │
    │ Subscription │  │ Subscription │  │  (Optional)  │
    └──────────────┘  └──────────────┘  └──────────────┘
                                                │
                                                ▼
                                        ┌──────────────┐
                                        │    Lambda    │
                                        │  Processing  │
                                        │  (Optional)  │
                                        └──────────────┘
```

## 🚀 Quick Start

### 1. Prerequisites

- [Terraform](https://www.terraform.io/downloads) >= 1.0
- [AWS CLI](https://aws.amazon.com/cli/) configured
- AWS account with appropriate permissions

### 2. Configure Variables

```bash
# Copy example variables file
cp terraform.tfvars.example terraform.tfvars

# Edit with your values
vi terraform.tfvars
```

### 3. Initialize Terraform

```bash
terraform init
```

### 4. Plan Infrastructure

```bash
terraform plan
```

### 5. Deploy Infrastructure

```bash
terraform apply
```

### 6. Save Credentials

```bash
# Generate .env file with credentials
terraform output -raw env_file_content > ../../.env

# View secret access key (if needed)
terraform output -raw aws_secret_access_key
```

## 📁 File Structure

```
terraform/
├── main.tf                    # Main infrastructure resources
├── variables.tf               # Input variable definitions
├── outputs.tf                 # Output values
├── terraform.tfvars.example   # Example variable values
├── backend.tf.example         # Remote state configuration
├── .gitignore                 # Git ignore patterns
└── README.md                  # This file
```

## 🔧 Configuration Options

### Basic Configuration (terraform.tfvars)

```hcl
aws_region  = "us-east-1"
environment = "dev"

notification_emails = [
  "your-email@example.com"
]

enable_cloudwatch = true
enable_sqs_queue  = false
enable_s3_storage = false
```

### Advanced Features

#### Enable SQS Queue for Event Processing
```hcl
enable_sqs_queue = true
```

#### Enable S3 Storage for Images
```hcl
enable_s3_storage    = true
image_retention_days = 30  # Auto-delete after 30 days
```

#### Add SMS Notifications
```hcl
notification_phones = [
  "+15551234567"  # E.164 format
]
```

## 📊 Resources Created

### Core Resources (Always Created)
- **SNS Topic (FIFO)**: Receives traffic alerts
- **IAM User**: Application credentials
- **IAM Policies**: SNS publish permissions
- **Email Subscriptions**: Based on `notification_emails`

### Optional Resources
- **SQS Queue**: Event queue (`enable_sqs_queue = true`)
- **S3 Bucket**: Image storage (`enable_s3_storage = true`)
- **CloudWatch Logs**: Monitoring (`enable_cloudwatch = true`)
- **CloudWatch Alarms**: High traffic alerts

## 🔐 Security Best Practices

### 1. Secure State File

**Option A: S3 Backend (Recommended)**
```bash
# Create S3 bucket for state
aws s3 mb s3://your-terraform-state-bucket --region us-east-1

# Enable versioning
aws s3api put-bucket-versioning \
  --bucket your-terraform-state-bucket \
  --versioning-configuration Status=Enabled

# Create DynamoDB table for state locking
aws dynamodb create-table \
  --table-name terraform-state-lock \
  --attribute-definitions AttributeName=LockID,AttributeType=S \
  --key-schema AttributeName=LockID,KeyType=HASH \
  --billing-mode PAY_PER_REQUEST

# Configure backend
cp backend.tf.example backend.tf
# Edit backend.tf with your bucket name
terraform init -migrate-state
```

### 2. Rotate Access Keys

```bash
# Create new key
terraform apply -replace=aws_iam_access_key.traffic_app_key

# Update .env file
terraform output -raw env_file_content > ../../.env
```

### 3. Least Privilege IAM

The IAM user only has permissions to:
- Publish to the specific SNS topic
- Upload to the specific S3 bucket (if enabled)

## 📝 Outputs

### View All Outputs
```bash
terraform output
```

### Specific Outputs
```bash
# SNS Topic ARN
terraform output sns_topic_arn

# Access Key ID
terraform output aws_access_key_id

# Secret Access Key (sensitive)
terraform output -raw aws_secret_access_key

# Complete .env file
terraform output -raw env_file_content
```

## 🧪 Testing

### Test SNS Publishing

```bash
# Get topic ARN
TOPIC_ARN=$(terraform output -raw sns_topic_arn)

# Publish test message
aws sns publish \
  --topic-arn "$TOPIC_ARN" \
  --message '{"test": "Hello from SNS"}' \
  --message-group-id "test-group" \
  --message-deduplication-id "test-$(date +%s)"
```

### Verify SQS Queue (if enabled)

```bash
# Get queue URL
QUEUE_URL=$(terraform output -raw sqs_queue_url)

# Receive messages
aws sqs receive-message --queue-url "$QUEUE_URL"
```

## 🔄 Common Operations

### Update Infrastructure

```bash
# Modify terraform.tfvars
vi terraform.tfvars

# Preview changes
terraform plan

# Apply changes
terraform apply
```

### Add Email Subscription

```bash
# Edit terraform.tfvars
notification_emails = [
  "existing@example.com",
  "new@example.com"  # Add new email
]

# Apply
terraform apply
```

### Destroy Infrastructure

```bash
# Preview destruction
terraform plan -destroy

# Destroy all resources
terraform destroy
```

## 💰 Cost Estimation

### Minimal Configuration (No optional features)
- SNS: ~$0.50 per million requests
- Email: First 1,000 free, then $2.00 per 100,000
- **Estimated**: $0.01 - $0.10/month for testing

### With Optional Features
- SQS: $0.40 per million requests
- S3: $0.023 per GB/month
- CloudWatch: $0.50 per GB ingested
- **Estimated**: $1 - $10/month depending on usage

## 🐛 Troubleshooting

### "Error creating SNS topic: AlreadyExists"
```bash
# Import existing topic
terraform import aws_sns_topic.traffic_alerts arn:aws:sns:us-east-1:123456789:topic-name.fifo
```

### Email subscription not confirmed
Check your inbox (including spam folder) for AWS SNS confirmation email.

### Invalid credentials
```bash
# Verify AWS CLI configuration
aws sts get-caller-identity

# Check Terraform AWS provider
terraform plan
```

### State lock errors
```bash
# Force unlock (use with caution)
terraform force-unlock LOCK_ID
```

## 📚 Additional Resources

- [Terraform AWS Provider Docs](https://registry.terraform.io/providers/hashicorp/aws/latest/docs)
- [AWS SNS Documentation](https://docs.aws.amazon.com/sns/)
- [Terraform Best Practices](https://www.terraform.io/docs/cloud/guides/recommended-practices/index.html)

## 🆘 Support

For issues specific to this infrastructure:
1. Check [AWS Service Health Dashboard](https://status.aws.amazon.com/)
2. Review CloudWatch logs (if enabled)
3. Open an issue on GitHub

---

**Last Updated**: January 2026  
**Version**: 1.0  
**Maintained by**: UFABC Security Cam Team
