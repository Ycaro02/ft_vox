#!/bin/bash

mkdir src/tinycthread

git clone https://github.com/tinycthread/tinycthread.git rsc/deps/tinycthread\
    && cp rsc/deps/tinycthread/source/* src/tinycthread\
