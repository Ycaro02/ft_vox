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


# sudo apt install libwayland-dev libxkbcommon-dev xorg-dev deps needed for glfw3
# Cmake is mandatory too
if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
	./install/clone_compile_glfw3.sh
fi

