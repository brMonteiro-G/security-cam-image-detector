# Makefile for Terraform Operations
# Simplifies common Terraform commands

.PHONY: help init plan apply destroy output clean fmt validate

# Default target
help:
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "  Traffic Detection System - Terraform Commands"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo ""
	@echo "Setup:"
	@echo "  make setup      - Configure variables and initialize"
	@echo "  make init       - Initialize Terraform"
	@echo ""
	@echo "Deployment:"
	@echo "  make plan       - Preview infrastructure changes"
	@echo "  make apply      - Deploy infrastructure"
	@echo "  make destroy    - Destroy all infrastructure"
	@echo ""
	@echo "Operations:"
	@echo "  make output     - Show all outputs"
	@echo "  make env        - Generate .env file"
	@echo "  make test       - Test SNS publishing"
	@echo ""
	@echo "Maintenance:"
	@echo "  make fmt        - Format Terraform files"
	@echo "  make validate   - Validate configuration"
	@echo "  make clean      - Clean Terraform cache"
	@echo ""
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Setup configuration
setup:
	@if [ ! -f terraform.tfvars ]; then \
		echo "Creating terraform.tfvars from example..."; \
		cp terraform.tfvars.example terraform.tfvars; \
		echo "✅ Created terraform.tfvars"; \
		echo "⚠️  Please edit terraform.tfvars with your values"; \
	else \
		echo "✅ terraform.tfvars already exists"; \
	fi
	@$(MAKE) init

# Initialize Terraform
init:
	@echo "🔧 Initializing Terraform..."
	@terraform init
	@echo "✅ Terraform initialized"

# Plan changes
plan:
	@echo "📋 Planning infrastructure changes..."
	@terraform plan
	@echo "✅ Plan complete"

# Apply changes
apply:
	@echo "🚀 Deploying infrastructure..."
	@terraform apply
	@echo "✅ Infrastructure deployed"
	@echo ""
	@$(MAKE) instructions

# Destroy infrastructure
destroy:
	@echo "⚠️  WARNING: This will destroy all infrastructure!"
	@echo "Press Ctrl+C to cancel, or"
	@read -p "Type 'yes' to continue: " confirm && [ "$$confirm" = "yes" ]
	@terraform destroy
	@echo "✅ Infrastructure destroyed"

# Show outputs
output:
	@terraform output

# Generate .env file
env:
	@echo "📝 Generating .env file..."
	@terraform output -raw env_file_content > ../../.env
	@echo "✅ .env file created at ../../.env"
	@echo ""
	@echo "⚠️  IMPORTANT: Add .env to .gitignore if not already present"

# Show setup instructions
instructions:
	@terraform output -raw setup_instructions

# Test SNS publishing
test:
	@echo "🧪 Testing SNS publishing..."
	@TOPIC_ARN=$$(terraform output -raw sns_topic_arn); \
	aws sns publish \
		--topic-arn "$$TOPIC_ARN" \
		--message '{"test":"message","timestamp":"'$$(date -u +%Y-%m-%dT%H:%M:%SZ)'"}' \
		--message-group-id "test-group" \
		--message-deduplication-id "test-$$(date +%s)"
	@echo "✅ Test message sent"

# Format Terraform files
fmt:
	@echo "🎨 Formatting Terraform files..."
	@terraform fmt -recursive
	@echo "✅ Files formatted"

# Validate configuration
validate:
	@echo "✅ Validating Terraform configuration..."
	@terraform validate
	@echo "✅ Configuration valid"

# Clean Terraform cache
clean:
	@echo "🧹 Cleaning Terraform cache..."
	@rm -rf .terraform
	@rm -f .terraform.lock.hcl
	@rm -f terraform.tfstate.backup
	@echo "✅ Cache cleaned"
	@echo "⚠️  Run 'make init' to reinitialize"

# Show current state
state:
	@terraform state list

# Quick deployment (plan + apply)
deploy: plan
	@$(MAKE) apply
