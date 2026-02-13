# Use the official Ubuntu image with x86-64 architecture
# This is necessary because the lab is designed for x86-64 systems
FROM --platform=linux/amd64 ubuntu:22.04

# Install required packages with retry logic
RUN rm -rf /var/lib/apt/lists/* && \
    apt-get clean && \
    apt-get update -o Acquire::Retries=3 && \
    apt-get install -y --no-install-recommends \
        build-essential \
        python3 \
        python3-pip \
        valgrind && \
    rm -rf /var/lib/apt/lists/*


# Set the working directory
WORKDIR /workspace

# Copy all files into the container
COPY . /workspace

# Default command
CMD ["/bin/bash"]
