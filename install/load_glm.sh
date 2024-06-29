#!/bin/bash

mkdir -p rsc/deps && cd rsc/deps\
 && git clone https://github.com/recp/cglm.git\
 && cd cglm && mkdir build && cd build \
 && cmake .. && make && touch ../.cglm_installed\