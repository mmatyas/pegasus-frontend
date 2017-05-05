FROM ubuntu:16.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    make


RUN useradd -ms /bin/bash ci

USER ci
WORKDIR /build
