FROM debian:stable-slim AS builder

LABEL description="Build container - gspfd"

ARG PROXY
ENV http_proxy=$PROXY
ENV https_proxy=$PROXY

RUN apt-get update
RUN apt-get -y install --no-install-recommends libboost-all-dev clang=1:7.0-47 cmake=3.13.4-1 ninja-build=1.8.2-1 build-essential=12.6 git=1:2.20.1-2 make=4.2.1-1.2 curl=7.64.0-4 autoconf=2.69-11 libtool=2.4.6-9 pkg-config=0.29-6

RUN mkdir -p /usr/local/src/gspd
COPY . /usr/local/src/gspd

WORKDIR /usr/local/src/gspd
RUN git submodule update --recursive --init

RUN rm -rf /usr/local/src/gspd/build
RUN mkdir -p /usr/local/src/gspd/build
WORKDIR /usr/local/src/gspd/build

RUN cmake ../ -GNinja -DCMAKE_BUILD_TYPE=Release && ninja


FROM debian:stable-slim AS runtime

LABEL description="Application container - gspfd"

RUN mkdir -p /usr/local/gspd
RUN groupadd -r gspd && useradd -r -s /bin/false -g gspd gspd
RUN chown gspd:gspd -R /usr/local/gspd
USER gspd
COPY --from=builder /usr/local/src/gspd/build/gspd /usr/local/gspd/
WORKDIR /usr/local/gspd

CMD ["gspd"]
