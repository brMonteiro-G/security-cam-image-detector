# VPC Configuration for Lambda Internet Access
# This VPC allows Lambda to access external URLs (camera streams)
# while maintaining security best practices

# ================================
# VPC and Subnets
# ================================

resource "aws_vpc" "lambda_vpc" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  cidr_block           = var.vpc_cidr
  enable_dns_hostnames = true
  enable_dns_support   = true
  
  tags = {
    Name = "${var.project_name}-vpc-${var.environment}"
  }
}

# Internet Gateway for public subnet
resource "aws_internet_gateway" "main" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  vpc_id = aws_vpc.lambda_vpc[0].id
  
  tags = {
    Name = "${var.project_name}-igw-${var.environment}"
  }
}

# Public Subnet (for NAT Gateway)
resource "aws_subnet" "public" {
  count = var.enable_lambda_vpc ? 2 : 0
  
  vpc_id                  = aws_vpc.lambda_vpc[0].id
  cidr_block              = cidrsubnet(var.vpc_cidr, 8, count.index)
  availability_zone       = data.aws_availability_zones.available.names[count.index]
  map_public_ip_on_launch = true
  
  tags = {
    Name = "${var.project_name}-public-subnet-${count.index + 1}-${var.environment}"
  }
}

# Private Subnet (for Lambda)
resource "aws_subnet" "private" {
  count = var.enable_lambda_vpc ? 2 : 0
  
  vpc_id            = aws_vpc.lambda_vpc[0].id
  cidr_block        = cidrsubnet(var.vpc_cidr, 8, count.index + 100)
  availability_zone = data.aws_availability_zones.available.names[count.index]
  
  tags = {
    Name = "${var.project_name}-private-subnet-${count.index + 1}-${var.environment}"
  }
}

# Data source for availability zones
data "aws_availability_zones" "available" {
  state = "available"
}

# ================================
# NAT Gateway (for outbound internet access from Lambda)
# ================================

# Elastic IP for NAT Gateway
resource "aws_eip" "nat" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  domain = "vpc"
  
  tags = {
    Name = "${var.project_name}-nat-eip-${var.environment}"
  }
  
  depends_on = [aws_internet_gateway.main]
}

# NAT Gateway in public subnet
resource "aws_nat_gateway" "main" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  allocation_id = aws_eip.nat[0].id
  subnet_id     = aws_subnet.public[0].id
  
  tags = {
    Name = "${var.project_name}-nat-${var.environment}"
  }
  
  depends_on = [aws_internet_gateway.main]
}

# ================================
# Route Tables
# ================================

# Public Route Table
resource "aws_route_table" "public" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  vpc_id = aws_vpc.lambda_vpc[0].id
  
  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.main[0].id
  }
  
  tags = {
    Name = "${var.project_name}-public-rt-${var.environment}"
  }
}

# Private Route Table (routes through NAT Gateway)
resource "aws_route_table" "private" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  vpc_id = aws_vpc.lambda_vpc[0].id
  
  route {
    cidr_block     = "0.0.0.0/0"
    nat_gateway_id = aws_nat_gateway.main[0].id
  }
  
  tags = {
    Name = "${var.project_name}-private-rt-${var.environment}"
  }
}

# Associate public subnets with public route table
resource "aws_route_table_association" "public" {
  count = var.enable_lambda_vpc ? 2 : 0
  
  subnet_id      = aws_subnet.public[count.index].id
  route_table_id = aws_route_table.public[0].id
}

# Associate private subnets with private route table
resource "aws_route_table_association" "private" {
  count = var.enable_lambda_vpc ? 2 : 0
  
  subnet_id      = aws_subnet.private[count.index].id
  route_table_id = aws_route_table.private[0].id
}

# ================================
# Security Group for Lambda
# ================================

resource "aws_security_group" "lambda_sg" {
  count = var.enable_lambda_vpc ? 1 : 0
  
  name        = "${var.project_name}-lambda-sg-${var.environment}"
  description = "Security group for Lambda function - allows outbound HTTPS"
  vpc_id      = aws_vpc.lambda_vpc[0].id
  
  # Outbound rule - allow all traffic (Lambda needs to reach SNS, camera URLs, etc.)
  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
    description = "Allow all outbound traffic"
  }
  
  tags = {
    Name = "${var.project_name}-lambda-sg-${var.environment}"
  }
}

# ================================
# VPC Endpoints (optional - reduce NAT costs)
# ================================

# VPC Endpoint for SNS (reduces NAT Gateway costs)
resource "aws_vpc_endpoint" "sns" {
  count = var.enable_lambda_vpc && var.enable_vpc_endpoints ? 1 : 0
  
  vpc_id              = aws_vpc.lambda_vpc[0].id
  service_name        = "com.amazonaws.${var.aws_region}.sns"
  vpc_endpoint_type   = "Interface"
  subnet_ids          = aws_subnet.private[*].id
  security_group_ids  = [aws_security_group.lambda_sg[0].id]
  private_dns_enabled = true
  
  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect    = "Allow"
        Principal = "*"
        Action    = "sns:*"
        Resource  = "*"
      }
    ]
  })
  
  tags = {
    Name = "${var.project_name}-sns-endpoint-${var.environment}"
  }
}

# VPC Endpoint for S3 (Gateway type - free!)
resource "aws_vpc_endpoint" "s3" {
  count = var.enable_lambda_vpc && var.enable_vpc_endpoints && var.enable_s3_storage ? 1 : 0
  
  vpc_id            = aws_vpc.lambda_vpc[0].id
  service_name      = "com.amazonaws.${var.aws_region}.s3"
  vpc_endpoint_type = "Gateway"
  route_table_ids   = [aws_route_table.private[0].id]
  
  tags = {
    Name = "${var.project_name}-s3-endpoint-${var.environment}"
  }
}

# VPC Endpoint for CloudWatch Logs
resource "aws_vpc_endpoint" "logs" {
  count = var.enable_lambda_vpc && var.enable_vpc_endpoints ? 1 : 0
  
  vpc_id              = aws_vpc.lambda_vpc[0].id
  service_name        = "com.amazonaws.${var.aws_region}.logs"
  vpc_endpoint_type   = "Interface"
  subnet_ids          = aws_subnet.private[*].id
  security_group_ids  = [aws_security_group.lambda_sg[0].id]
  private_dns_enabled = true
  
  tags = {
    Name = "${var.project_name}-logs-endpoint-${var.environment}"
  }
}
