# Variables for Traffic Detection Infrastructure

variable "aws_region" {
  description = "AWS region for resources"
  type        = string
  default     = "us-east-1"
}

variable "aws_profile" {
  description = "AWS CLI profile to use"
  type        = string
  default     = "maisumdia-pipe"
}

variable "environment" {
  description = "Environment name (dev, staging, prod)"
  type        = string
  default     = "dev"
  
  validation {
    condition     = can(regex("^(dev|staging|prod)$", var.environment))
    error_message = "Environment must be dev, staging, or prod."
  }
}

variable "project_name" {
  description = "Project name for resource naming"
  type        = string
  default     = "security-cam-detector"
}

variable "notification_emails" {
  description = "List of email addresses to receive traffic alerts"
  type        = list(string)
  default     = []
  
  validation {
    condition     = alltrue([for email in var.notification_emails : can(regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", email))])
    error_message = "All emails must be valid email addresses."
  }
}

variable "notification_phones" {
  description = "List of phone numbers for SMS alerts (E.164 format: +1234567890)"
  type        = list(string)
  default     = []
}

variable "enable_sqs_queue" {
  description = "Enable SQS queue for event processing"
  type        = bool
  default     = false
}

variable "enable_cloudwatch" {
  description = "Enable CloudWatch monitoring and alarms"
  type        = bool
  default     = true
}

variable "log_retention_days" {
  description = "CloudWatch log retention period in days"
  type        = number
  default     = 7
  
  validation {
    condition     = contains([1, 3, 5, 7, 14, 30, 60, 90, 120, 150, 180, 365, 400, 545, 731, 1827, 3653], var.log_retention_days)
    error_message = "Log retention must be a valid CloudWatch retention period."
  }
}

variable "enable_s3_storage" {
  description = "Enable S3 bucket for image storage"
  type        = bool
  default     = false
}

variable "image_retention_days" {
  description = "Number of days to retain images in S3"
  type        = number
  default     = 30
}

variable "tags" {
  description = "Additional tags to apply to resources"
  type        = map(string)
  default     = {}
}

# Lambda Configuration
variable "enable_lambda" {
  description = "Enable Lambda function for traffic detection"
  type        = bool
  default     = true
}

variable "lambda_memory_size" {
  description = "Lambda function memory size in MB"
  type        = number
  default     = 3008  # Max memory for CPU optimization
  
  validation {
    condition     = var.lambda_memory_size >= 512 && var.lambda_memory_size <= 10240
    error_message = "Lambda memory must be between 512 and 10240 MB."
  }
}

variable "lambda_timeout" {
  description = "Lambda function timeout in seconds"
  type        = number
  default     = 900  # 15 minutes (max for Lambda)
  
  validation {
    condition     = var.lambda_timeout >= 3 && var.lambda_timeout <= 900
    error_message = "Lambda timeout must be between 3 and 900 seconds."
  }
}

variable "lambda_schedule_expression" {
  description = "EventBridge schedule expression for Lambda execution (e.g., 'rate(5 minutes)' or 'cron(0 * * * ? *)')"
  type        = string
  default     = "cron(0 6,8,12,18,20 * * ? *)"
}

variable "camera_stream_url" {
  description = "URL of the camera stream to process"
  type        = string
  default     = "http://cameras.ufabc.edu.br/mjpg/video.mjpg"
}

variable "lambda_ephemeral_storage" {
  description = "Lambda ephemeral storage in MB (for temporary files)"
  type        = number
  default     = 2048  # 2GB for image processing
  
  validation {
    condition     = var.lambda_ephemeral_storage >= 512 && var.lambda_ephemeral_storage <= 10240
    error_message = "Lambda ephemeral storage must be between 512 and 10240 MB."
  }
}

# VPC Configuration
variable "enable_lambda_vpc" {
  description = "Enable VPC for Lambda (required for accessing external URLs like camera streams)"
  type        = bool
  default     = true
}

variable "vpc_cidr" {
  description = "CIDR block for VPC"
  type        = string
  default     = "10.0.0.0/16"
}

variable "enable_vpc_endpoints" {
  description = "Enable VPC endpoints for AWS services (reduces NAT Gateway costs)"
  type        = bool
  default     = true
}
