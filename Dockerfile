FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    make \
    gcc \
    g++ \
    cmake \
    python3 \
    python3-pip \
    libgl1-mesa-dev \
    xorg-dev && \
    rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/python3 /usr/bin/python
# /usr/bin/pip -> /usr/bin/pip3 is already present

COPY requirements.txt /tmp

RUN python -m pip install --upgrade pip \
    pip install -r /tmp/requirements.txt

# todo: add docs: build image, run container
# Powershell
#   docker run --rm -it -v ${PWD}:/code ygci
# bash
#   docker run --rm -it -v $(pwd):/code ygci
