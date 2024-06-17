#!/bin/bash

mkdir -p src/tinycthread

git clone https://github.com/tinycthread/tinycthread.git rsc/deps/tinycthread\
	&& gcc -o ttmp_thread.o -c rsc/deps/tinycthread/source/tinycthread.c\
	&& ar rcs rsc/lib_deps/libstatic_tinycthread.a ttmp_thread.o\
	&& rm ttmp_thread.o\
	&& cp rsc/deps/tinycthread/source/tinycthread.h libft/tinycthread.h\
