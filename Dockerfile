FROM ubuntu:24.04

RUN apt update && apt install -y \
    build-essential \
    gcc-arm-none-eabi \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
COPY . .

CMD ["make"]
