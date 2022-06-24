# syntax=docker/dockerfile:1
ARG PROJECT_REPO
ARG DOCKER_VER_IMG
FROM ${PROJECT_REPO}:thirdparty-${DOCKER_VER_IMG}

ARG DEBIAN_FRONTEND=noninteractive
ARG SONAR_SCANNER_VERSION=4.7.0.2747
ARG SONAR_SCANNER_HOME=/sonar/sonar-scanner-${SONAR_SCANNER_VERSION}-linux
ARG SONAR_SCANNER_OPTS="-server"

ARG SONAR_TOKEN
ARG GIT_BRANCH
ARG PROJECT_VERSION
ARG PROJECT_REPO

RUN apt update -qq && apt install -qq -y --no-install-recommends xmlstarlet unzip libgtest-dev \
 && rm -rf /var/lib/apt/lists/*
RUN cd /usr/src/googletest && cmake . && cmake --build . --target install

WORKDIR /sonar
RUN curl -sSLo build-wrapper-linux-x86.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip \
    && unzip build-wrapper-linux-x86.zip > /dev/null \
    && curl -sSLo sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_SCANNER_VERSION}-linux.zip \
    && unzip -o sonar-scanner.zip > /dev/null

ENV PATH=/sonar/build-wrapper-linux-x86:${PATH}
ENV PATH=${SONAR_SCANNER_HOME}/bin:${PATH}
ENV SONAR_TOKEN=${SONAR_TOKEN}
ENV GIT_BRANCH=${GIT_BRANCH}
ENV PROJECT_VERSION=${PROJECT_VERSION}
ENV PROJECT_REPO=${PROJECT_REPO}
