FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
 && apt install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    curl \
    libusb-1.0-0-dev \
 && rm -rf /var/lib/apt/lists/*

ENV CMAKE_VERSION=3.18.3
WORKDIR /tmp/cmake
RUN curl -OL https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
 && chmod +x cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
 && ./cmake-${CMAKE_VERSION}-Linux-x86_64.sh --skip-license --prefix=/usr \
 && rm -r /tmp/cmake

COPY cmake /app/cmake
COPY deb /app/deb
COPY docs /app/docs
COPY modules /app/modules
COPY CMakeLists.txt /app/CMakeLists.txt
COPY CyUSBSerialConfig.cmake.in /app/CyUSBSerialConfig.cmake.in
COPY README.md /app/README.md

WORKDIR /app/build
RUN cmake .. \
 && make -j$(nproc) package \
 && ls *.deb > .deb
