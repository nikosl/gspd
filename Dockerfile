FROM debian:stable-slim AS builder

LABEL description="Build container - gspfd"

ARG PROXY
ENV http_proxy=$PROXY
ENV https_proxy=$PROXY

RUN apt-get -y update && apt-get -y upgrade
RUN apt-get -y install clang cmake ninja-build build-essential git tar make curl wget autoconf libtool pkg-config

RUN mkdir -p /usr/local/src/gspd
COPY . /usr/local/src/gspd

RUN rm -rf /usr/local/src/gspd/build
RUN mkdir -p /usr/local/src/gspd/build
WORKDIR /usr/local/src/gspd/build

#RUN cmake ../ -GNinja && ninja


# FROM debian:stable-slim AS runtime

# LABEL description="Application container - gspfd"

# RUN apt-get -y update && apt-get -y upgrade
# RUN mkdir -p /usr/local/gspd
# RUN groupadd -r gspd && useradd -r -s /bin/false -g gspd gspd

# COPY --from=builder /usr/local/src/gspd/build/gspd /usr/local/gspd/
# WORKDIR /usr/local/gspd

# CMD gspd
