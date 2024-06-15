#!/bin/bash

mkdir -p rsc/deps && cd rsc/deps && git clone https://github.com/freetype/freetype.git && cd freetype\
&& sh autogen.sh\
&& ./configure && make
