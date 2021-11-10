FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
 && apt install -y --no-install-recommends \
    gnupg \
    build-essential \
    ca-certificates \
    curl \
    cmake \
    libusb-1.0-0-dev \
 && rm -rf /var/lib/apt/lists/*

COPY license.txt /app/license.txt
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
