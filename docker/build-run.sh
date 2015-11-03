#!/bin/bash

docker build -t rauschen .

SRC_PATH="`dirname \"$0\"`"
SRC_PATH="`( cd \"$SRC_PATH/..\" && pwd )`" 
if [ -z "$SRC_PATH" ] ; then
  exit 1
fi

docker run -v $SRC_PATH:/rauschen-src --rm rauschen bash -c "mkdir /rauschen-{release,debug} && cd /rauschen-debug && cmake /rauschen-src && make && bin/rauschen"
