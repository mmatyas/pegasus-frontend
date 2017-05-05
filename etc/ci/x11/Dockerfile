FROM ubuntu:16.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    libgl1-mesa-dev \
    libpng-dev \
    libjpeg-dev \
    libpcre3-dev \
    libharfbuzz-dev


RUN useradd -ms /bin/bash ci

USER ci
WORKDIR /build
