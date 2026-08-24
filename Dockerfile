FROM ubuntu:24.04

# Mode non-interactif pour éviter les blocages
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    git \
    cppcheck \
    ruby-full \
    python3 \
    python3-pip \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Installation d'esptool via pip (le drapeau est requis pour Ubuntu 24.04 / Python 3.12)
RUN pip3 install --break-system-packages esptool

RUN gem install ceedling --no-document

WORKDIR /project
COPY . .
CMD ["make"]
