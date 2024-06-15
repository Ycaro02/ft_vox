#!/bin/bash

if [ ! -d "rsc/deps" ]; then
	mkdir rsc/deps
fi

if [ ! -d "rsc/lib_deps" ]; then
	mkdir rsc/lib_deps
fi

if [ ! -d "rsc/deps/cglm" ]; then
	./install/load_glm.sh
fi

# deps needed for glfw3
# apt install libwayland-dev libxkbcommon-dev xorg-dev
# Cmake mandatory too
if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
	./install/clone_compile_glfw3.sh
fi

if [ ! -d "rsc/deps/tinycthread" ]; then
	./install/install_tinycthread.sh
fi

# deps needed for freetype 
# libtool [libtool dependency for autogen.sh])
if [ ! -d "rsc/deps/freetype" ]; then
	./install/install_freetype.sh
fi