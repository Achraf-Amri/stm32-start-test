FROM ubuntu:24.04

RUN apt update && apt install -y \
    build-essential \
    make \
    git \
    cppcheck \
    ruby-full \
    python3 \
    python3-pip \
    curl \
    && rm -rf /var/lib/apt/lists/*

RUN gem install ceedling --no-document \
    && pip3 install --break-system-packages esptool

WORKDIR /project
COPY . .
CMD ["make"]
