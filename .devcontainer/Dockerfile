# Start from openSUSE Leap
FROM --platform=linux/amd64 opensuse/tumbleweed:latest

# Specify arguments for optional tools installation
ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Install development tools and dependencies
RUN zypper refresh && \
    zypper install -y \
    make \
    gcc \
    gcc-c++ \
    cmake \
    git \
    curl \
    tar \
    gzip \
    libstdc++-devel \
    clang \
    awk \
    && zypper clean -a


# Install or reinstall cmake from source if specified
COPY ./reinstall-cmake.sh /tmp/
RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Verify cmake version
RUN cmake --version

# Set the working directory
WORKDIR /workspace

# [Optional] Uncomment to install additional packages if needed
# RUN zypper install -y <your-package-list-here>