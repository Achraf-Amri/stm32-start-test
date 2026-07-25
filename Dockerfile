FROM ubuntu:24.04
RUN apt update && apt install -y \
    build-essential \
    gcc-arm-none-eabi \
    make \
    git \
    cppcheck \
    ruby-full \
    && rm -rf /var/lib/apt/lists/*

RUN gem install ceedling --no-document

WORKDIR /project
COPY . .
CMD ["make"]

