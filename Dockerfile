# filepath: c:\work\libcpp\Dockerfile
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source code
COPY . .

# Build the project
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja \
    && cmake --build build --parallel $(nproc) \
    && cmake --build build --target test

# Runtime image
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy built libraries and binaries
COPY --from=builder /workspace/build/lib* /usr/local/lib/
COPY --from=builder /workspace/build/bin* /usr/local/bin/

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Create non-root user
RUN useradd -m -s /bin/bash libcpp
USER libcpp

WORKDIR /home/libcpp

CMD ["/bin/bash"]