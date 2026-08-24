FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-arm-none-eabi \
    make \
    git \
    cppcheck \
    ruby-full \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
