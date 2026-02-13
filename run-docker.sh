#!/bin/bash

# Docker helper script for CMU 15-213 Cache Lab
# This script builds and runs the cache lab in a Docker container
# with volume mounting so changes are reflected in real-time

set -e

# Get the directory where this script is located
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="cachelab-env"
CONTAINER_NAME="cachelab-container"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    print_error "Docker is not installed."
    echo "Please install Docker Desktop for Mac from:"
    echo "https://www.docker.com/products/docker-desktop/"
    exit 1
fi

# Check if Docker daemon is running
if ! docker ps &> /dev/null; then
    print_error "Docker daemon is not running."
    echo "Please start Docker Desktop and try again."
    exit 1
fi

# Build the Docker image (only if it doesn't exist)
if ! docker image inspect "$IMAGE_NAME" &> /dev/null; then
    print_info "Building Docker image '$IMAGE_NAME'..."
    docker build -t "$IMAGE_NAME" "$PROJECT_DIR"
    print_success "Docker image built successfully!"
else
    print_info "Docker image '$IMAGE_NAME' already exists. Skipping build."
fi

# Run the container with volume mount
print_info "Starting Docker container with volume mount..."
print_info "Your workspace is mounted at /workspace"
print_info "Any changes you make locally will be reflected in the container."
echo ""
print_success "Enter 'exit' to quit the container"
echo ""

docker run -it --rm \
    --name "$CONTAINER_NAME" \
    -v "$PROJECT_DIR":/workspace \
    "$IMAGE_NAME" \
    /bin/bash

print_success "Container finished."
