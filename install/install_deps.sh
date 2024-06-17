#!/bin/bash

source ./rsc/sh/color.sh

if [ ! -d "rsc/deps" ]; then
	mkdir rsc/deps
	display_color_msg ${LIGHT_CYAN} "Installing dependencies"
fi

if [ ! -d "rsc/lib_deps" ]; then
	mkdir rsc/lib_deps
fi



if [ ! -d "rsc/deps/cglm" ]; then
	display_color_msg ${MAGENTA} "Loading cglm ..."
	./install/load_glm.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "cglm installed"
fi


# deps needed for glfw3
# apt install libwayland-dev libxkbcommon-dev xorg-dev
# Cmake mandatory too
if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
	display_color_msg ${MAGENTA} "Loading libglfw3 ..."
	./install/clone_compile_glfw3.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "libglfw3 installed"
fi

if [ ! -d "rsc/deps/tinycthread" ]; then
	display_color_msg ${MAGENTA} "Loading tinycthread ..."
	./install/install_tinycthread.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "tinycthread installed"
fi

# deps needed for freetype 
# libtool [libtool dependency for autogen.sh])
if [ ! -d "rsc/deps/freetype" ]; then
	display_color_msg ${MAGENTA} "Loading freetype ..."
	./install/install_freetype.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "freetype installed"
fi