# Base image — Ubuntu 22.04
FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install g++ and required tools
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside container
WORKDIR /app

# Copy server files into container
COPY server/server.cpp .
COPY server/auth.h .
COPY server/filemanager.h .
COPY server/filelock.h .

# Compile the server
RUN g++ server.cpp -o server -pthread -lstdc++fs

# Create uploads folder
RUN mkdir -p uploads

# Expose port 9999
EXPOSE 9999

# Run server when container starts
CMD ["./server"]