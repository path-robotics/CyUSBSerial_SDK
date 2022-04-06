# syntax = docker/dockerfile:1.0-experimental
FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt -qq update \
 && apt -qq install -y --no-install-recommends --autoremove \
    gnupg \
    build-essential \
    ca-certificates \
    curl \
    libusb-1.0-0-dev \
 && rm -rf /var/lib/apt/lists/*

RUN curl -sLo - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
 && echo 'deb https://apt.kitware.com/ubuntu/ bionic main' > /etc/apt/sources.list.d/kitware.list \
 && apt -qq -y update \
 && apt -qq -y install --no-install-recommends cmake=3.18.* cmake-data=3.18.* \
 && apt-mark hold cmake cmake-data
