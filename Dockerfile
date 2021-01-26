FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update \
 && apt install -y --no-install-recommends \
    gnupg \
    build-essential \
    ca-certificates \
    curl \
    libusb-1.0-0-dev \
 && rm -rf /var/lib/apt/lists/*

RUN curl -sLo - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
 && echo 'deb https://apt.kitware.com/ubuntu/ bionic main' > /etc/apt/sources.list.d/kitware.list \
 && apt update \
 && apt install -y --no-install-recommends cmake=3.18.* cmake-data=3.18.* \
 && apt-mark hold cmake cmake-data

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
