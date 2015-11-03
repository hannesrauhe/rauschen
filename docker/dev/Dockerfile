FROM ubuntu:14.04

RUN apt-get -y update && \
    apt-get install -y software-properties-common && \
    apt-add-repository -y ppa:george-edison55/cmake-3.x && \
    apt-add-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get -y update
RUN apt-get install -y libasio-dev g++-4.9 cmake protobuf-compiler protobuf-c-$
RUN git clone https://github.com/hannesrauhe/rauschen.git

