# Main Terraform configuration for Traffic Detection System
# This creates SNS topics, subscriptions, and IAM resources

terraform {
  required_version = ">= 1.0"
  
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }
}

provider "aws" {
  region  = var.aws_region  

  profile = "maisumdia-pipe" ## change it later 
  
  default_tags {
    tags = {
      Project     = "SecurityCamImageDetector"
      Environment = var.environment
      ManagedBy   = "Terraform"
      Owner       = "UFABC"
    }
  }
}

# Data source to get current AWS account ID
data "aws_caller_identity" "current" {}

# SNS Topic for Traffic Alerts (FIFO)
resource "aws_sns_topic" "traffic_alerts" {
  name                        = "${var.project_name}-traffic-alerts-${var.environment}.fifo"
  display_name                = "Traffic Density Alerts - ${var.environment}"
  fifo_topic                  = true
  content_based_deduplication = false  # We're setting MessageDeduplicationId manually
  
  tags = {
    Name = "Traffic Alerts Topic"
  }
}

# SNS Topic Policy
resource "aws_sns_topic_policy" "traffic_alerts_policy" {
  arn = aws_sns_topic.traffic_alerts.arn

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowPublishFromApplication"
        Effect = "Allow"
        Principal = {
          AWS = aws_iam_user.traffic_app_user.arn
        }
        Action = [
          "SNS:Publish"
        ]
        Resource = aws_sns_topic.traffic_alerts.arn
      }
    ]
  })
}

# # Email Subscription (requires manual confirmation)
# resource "aws_sns_topic_subscription" "email_subscription" {
#   count = length(var.notification_emails)
  
#   topic_arn = aws_sns_topic.traffic_alerts.arn
#   protocol  = "email"
#   endpoint  = var.notification_emails[count.index]
# }



# Optional: SQS Queue for message processing
resource "aws_sqs_queue" "traffic_events" {
  count = var.enable_sqs_queue ? 1 : 0
  
  name                      = "${var.project_name}-traffic-events-${var.environment}.fifo"
  fifo_queue                = true
  content_based_deduplication = false
  delay_seconds             = 0
  max_message_size          = 262144
  message_retention_seconds = 86400  # 1 day
  receive_wait_time_seconds = 10
  
  tags = {
    Name = "Traffic Events Queue"
  }
}

# SQS Queue Policy
resource "aws_sqs_queue_policy" "traffic_events_policy" {
  count = var.enable_sqs_queue ? 1 : 0
  
  queue_url = aws_sqs_queue.traffic_events[0].id

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowSNSToSendMessage"
        Effect = "Allow"
        Principal = {
          Service = "sns.amazonaws.com"
        }
        Action   = "SQS:SendMessage"
        Resource = aws_sqs_queue.traffic_events[0].arn
        Condition = {
          ArnEquals = {
            "aws:SourceArn" = aws_sns_topic.traffic_alerts.arn
          }
        }
      }
    ]
  })
}

# SNS to SQS Subscription
resource "aws_sns_topic_subscription" "sqs_subscription" {
  count = var.enable_sqs_queue ? 1 : 0
  
  topic_arn = aws_sns_topic.traffic_alerts.arn
  protocol  = "sqs"
  endpoint  = aws_sqs_queue.traffic_events[0].arn
}

# IAM User for Application
resource "aws_iam_user" "traffic_app_user" {
  name = "${var.project_name}-app-user-${var.environment}"
  path = "/applications/"

  tags = {
    Name        = "Traffic Detection App User"
    Description = "User for C++ application to publish SNS messages"
  }
}

# IAM Policy for SNS Publishing
resource "aws_iam_user_policy" "traffic_app_sns_publish" {
  name = "${var.project_name}-sns-publish-policy"
  user = aws_iam_user.traffic_app_user.name

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowSNSPublish"
        Effect = "Allow"
        Action = [
          "sns:Publish"
        ]
        Resource = aws_sns_topic.traffic_alerts.arn
      }
    ]
  })
}

# Access Keys for Application User
resource "aws_iam_access_key" "traffic_app_key" {
  user = aws_iam_user.traffic_app_user.name
}

# Optional: CloudWatch Log Group for monitoring
resource "aws_cloudwatch_log_group" "traffic_logs" {
  count = var.enable_cloudwatch ? 1 : 0
  
  name              = "/aws/traffic-detection/${var.environment}"
  retention_in_days = var.log_retention_days

  tags = {
    Name = "Traffic Detection Logs"
  }
}

# # Optional: CloudWatch Metric Alarm for high traffic
# resource "aws_cloudwatch_metric_alarm" "high_traffic_alarm" {
#   count = var.enable_cloudwatch ? 1 : 0
  
#   alarm_name          = "${var.project_name}-high-traffic-${var.environment}"
#   comparison_operator = "GreaterThanThreshold"
#   evaluation_periods  = 2
#   metric_name         = "NumberOfMessagesPublished"
#   namespace           = "AWS/SNS"
#   period              = 300  # 5 minutes
#   statistic           = "Sum"
#   threshold           = 50
#   alarm_description   = "This metric monitors SNS message volume"
  
#   dimensions = {
#     TopicName = aws_sns_topic.traffic_alerts.name
#   }

#   alarm_actions = [aws_sns_topic.traffic_alerts.arn]
# }

# S3 Bucket for storing captured images (optional)
resource "aws_s3_bucket" "traffic_images" {
  count = var.enable_s3_storage ? 1 : 0
  
  bucket = "${var.project_name}-images-${var.environment}-${data.aws_caller_identity.current.account_id}"

  tags = {
    Name = "Traffic Camera Images"
  }
}

# S3 Bucket Versioning
resource "aws_s3_bucket_versioning" "traffic_images_versioning" {
  count = var.enable_s3_storage ? 1 : 0
  
  bucket = aws_s3_bucket.traffic_images[0].id
  
  versioning_configuration {
    status = "Enabled"
  }
}

# S3 Bucket Lifecycle
resource "aws_s3_bucket_lifecycle_configuration" "traffic_images_lifecycle" {
  count = var.enable_s3_storage ? 1 : 0
  
  bucket = aws_s3_bucket.traffic_images[0].id

  rule {
    id     = "expire-old-images"
    status = "Enabled"

    filter {
      prefix = ""  # Apply to all objects
    }

    expiration {
      days = var.image_retention_days
    }
  }
}

# S3 Bucket Public Access Block
resource "aws_s3_bucket_public_access_block" "traffic_images_block" {
  count = var.enable_s3_storage ? 1 : 0
  
  bucket = aws_s3_bucket.traffic_images[0].id

  block_public_acls       = true
  block_public_policy     = true
  ignore_public_acls      = true
  restrict_public_buckets = true
}

# IAM Policy for S3 Access (if enabled)
resource "aws_iam_user_policy" "traffic_app_s3_access" {
  count = var.enable_s3_storage ? 1 : 0
  
  name = "${var.project_name}-s3-access-policy"
  user = aws_iam_user.traffic_app_user.name

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowS3Upload"
        Effect = "Allow"
        Action = [
          "s3:PutObject",
          "s3:GetObject",
          "s3:ListBucket"
        ]
        Resource = [
          aws_s3_bucket.traffic_images[0].arn,
          "${aws_s3_bucket.traffic_images[0].arn}/*"
        ]
      }
    ]
  })
}

# ================================
# Lambda Function Resources
# ================================

# ECR Repository for Lambda Container Images
resource "aws_ecr_repository" "traffic_detector" {
  count = var.enable_lambda ? 1 : 0
  
  name                 = "${var.project_name}-lambda-${var.environment}"
  image_tag_mutability = "MUTABLE"
  
  image_scanning_configuration {
    scan_on_push = true
  }
  
  tags = {
    Name = "Traffic Detector Lambda Container"
  }
}

# ECR Lifecycle Policy
resource "aws_ecr_lifecycle_policy" "traffic_detector_lifecycle" {
  count = var.enable_lambda ? 1 : 0
  
  repository = aws_ecr_repository.traffic_detector[0].name

  policy = jsonencode({
    rules = [
      {
        rulePriority = 1
        description  = "Keep last 5 images"
        selection = {
          tagStatus     = "any"
          countType     = "imageCountMoreThan"
          countNumber   = 5
        }
        action = {
          type = "expire"
        }
      }
    ]
  })
}

# IAM Role for Lambda Function
resource "aws_iam_role" "lambda_execution" {
  count = var.enable_lambda ? 1 : 0
  
  name = "${var.project_name}-lambda-role-${var.environment}"
  
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Action = "sts:AssumeRole"
        Effect = "Allow"
        Principal = {
          Service = "lambda.amazonaws.com"
        }
      }
    ]
  })
  
  tags = {
    Name = "Lambda Execution Role"
  }
}

# Lambda Basic Execution Policy
resource "aws_iam_role_policy_attachment" "lambda_basic" {
  count = var.enable_lambda ? 1 : 0
  
  role       = aws_iam_role.lambda_execution[0].name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole"
}

# Lambda VPC Execution Policy (if needed for VPC access)
resource "aws_iam_role_policy_attachment" "lambda_vpc" {
  count = var.enable_lambda ? 1 : 0
  
  role       = aws_iam_role.lambda_execution[0].name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSLambdaVPCAccessExecutionRole"
}

# Lambda Custom Policy for SNS and S3
resource "aws_iam_role_policy" "lambda_custom" {
  count = var.enable_lambda ? 1 : 0
  
  name = "${var.project_name}-lambda-policy-${var.environment}"
  role = aws_iam_role.lambda_execution[0].id

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowSNSPublish"
        Effect = "Allow"
        Action = [
          "sns:Publish"
        ]
        Resource = aws_sns_topic.traffic_alerts.arn
      },
      {
        Sid    = "AllowS3Access"
        Effect = "Allow"
        Action = [
          "s3:PutObject",
          "s3:GetObject",
          "s3:ListBucket"
        ]
        Resource = var.enable_s3_storage ? [
          aws_s3_bucket.traffic_images[0].arn,
          "${aws_s3_bucket.traffic_images[0].arn}/*"
        ] : []
      },
      {
        Sid    = "AllowCloudWatchLogs"
        Effect = "Allow"
        Action = [
          "logs:CreateLogGroup",
          "logs:CreateLogStream",
          "logs:PutLogEvents"
        ]
        Resource = "arn:aws:logs:${var.aws_region}:${data.aws_caller_identity.current.account_id}:log-group:/aws/lambda/${var.project_name}-*"
      }
    ]
  })
}

# Lambda Function (using container image)
resource "aws_lambda_function" "traffic_detector" {
  count = var.enable_lambda ? 1 : 0
  
  function_name = "${var.project_name}-detector-${var.environment}"
  role          = aws_iam_role.lambda_execution[0].arn
  
  # Container image configuration
  package_type = "Image"
  image_uri    = "${aws_ecr_repository.traffic_detector[0].repository_url}:latest"
  
  # Resource configuration
  memory_size = var.lambda_memory_size
  timeout     = var.lambda_timeout
  
  ephemeral_storage {
    size = var.lambda_ephemeral_storage
  }
  
  # Environment variables
  environment {
    variables = {
      AWS_SNS_TOPIC_ARN    = aws_sns_topic.traffic_alerts.arn
      CAMERA_STREAM_URL    = var.camera_stream_url
      S3_BUCKET_NAME       = var.enable_s3_storage ? aws_s3_bucket.traffic_images[0].id : ""
      ENVIRONMENT          = var.environment
      ENABLE_AWS_SNS       = "true"
    }
  }
  
  # Logging configuration
  logging_config {
    log_format = "JSON"
    log_group  = "/aws/lambda/${var.project_name}-detector-${var.environment}"
  }
  
  tags = {
    Name = "Traffic Detector Lambda"
  }
  
  # Prevent Terraform from trying to create before image is pushed
  lifecycle {
    ignore_changes = [image_uri]
  }
}

# CloudWatch Log Group for Lambda
resource "aws_cloudwatch_log_group" "lambda_logs" {
  count = var.enable_lambda ? 1 : 0
  
  name              = "/aws/lambda/${var.project_name}-detector-${var.environment}"
  retention_in_days = var.log_retention_days
  
  tags = {
    Name = "Lambda Traffic Detector Logs"
  }
}

# EventBridge Rule to trigger Lambda periodically
resource "aws_cloudwatch_event_rule" "lambda_schedule" {
  count = var.enable_lambda ? 1 : 0
  
  name                = "${var.project_name}-schedule-${var.environment}"
  description         = "Trigger traffic detection Lambda periodically"
  schedule_expression = var.lambda_schedule_expression
  
  tags = {
    Name = "Lambda Schedule Rule"
  }
}

# EventBridge Target (Lambda)
resource "aws_cloudwatch_event_target" "lambda_target" {
  count = var.enable_lambda ? 1 : 0
  
  rule      = aws_cloudwatch_event_rule.lambda_schedule[0].name
  target_id = "TrafficDetectorLambda"
  arn       = aws_lambda_function.traffic_detector[0].arn
  
  input = jsonencode({
    avenue_name = "Avenida dos Estados"
    mode        = "production"
  })
}

# Lambda Permission for EventBridge
resource "aws_lambda_permission" "allow_eventbridge" {
  count = var.enable_lambda ? 1 : 0
  
  statement_id  = "AllowExecutionFromEventBridge"
  action        = "lambda:InvokeFunction"
  function_name = aws_lambda_function.traffic_detector[0].function_name
  principal     = "events.amazonaws.com"
  source_arn    = aws_cloudwatch_event_rule.lambda_schedule[0].arn
}

