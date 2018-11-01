FROM ubuntu:18.04

RUN apt-get update

RUN apt-get install -y build-essential lv2-dev pkg-config cmake git mercurial
RUN apt-get install -y lv2file

RUN mkdir -p /code
WORKDIR /code
