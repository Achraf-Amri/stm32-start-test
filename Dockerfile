FROM ubuntu:24.04

# Mode non-interactif pour éviter les blocages sur tzdata
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    git \
    cppcheck \
    ruby-full \
    python3 \
    python3-pip \
    python3-esptool \
    curl \
    && rm -rf /var/lib/apt/lists/*

RUN gem install ceedling --no-document

WORKDIR /project
COPY . .
CMD ["make"]
