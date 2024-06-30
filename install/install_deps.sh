#!/bin/bash

source ./rsc/sh/color.sh

# This script install or check all dependencies needed for the project
# It will install cglm, glfw3, tinycthread and freetype
# It will also check if the dependencies are already installed

# cglm is a C math library for graphics programming
# glfw3 is a library for creating windows with OpenGL contexts and managing input
# tinycthread is a small, portable implementation of a C11 threads API
# freetype is a software library that can open and rasterize font files

# Global variables
MODE=""
REDIR=""

display_help() {
	display_color_msg ${LIGHT_CYAN} "Usage: ./install/install_deps.sh [install|check] [-s]"
	display_color_msg ${LIGHT_CYAN} "Options:"
	display_color_msg ${LIGHT_CYAN} "  -s  Silent mode"
}

set_and_check_globale() {
	if [ "$1" == "install" ]; then
		MODE="install"
	elif [ "$1" == "check" ]; then
		MODE="check"
	elif [ "$1" == "-h" ]; then
		display_help
		exit 0
	else 
		display_help
		exit 1
	fi 

	# Check args for silen mode
	if [ "$2" == "-s" ]; then
		REDIR=">/dev/null 2>&1"
	fi
}

create_dir() {
	if [ ! -d "$1" ]; then
		mkdir -p $1
		display_color_msg ${LIGHT_CYAN} "Creating $1"
	fi
}

install_cglm() {
	display_color_msg ${MAGENTA} "Loading cglm ..."
	eval ./install/load_glm.sh ${REDIR}
	display_color_msg ${GREEN} "cglm installed"
}

# deps needed for glfw3
# apt install libwayland-dev libxkbcommon-dev xorg-dev
# Cmake mandatory too
install_glfw3() {
	display_color_msg ${MAGENTA} "Loading libglfw3 ..."
	eval ./install/clone_compile_glfw3.sh ${REDIR}
	display_color_msg ${GREEN} "libglfw3 installed"
}

install_tinycthread() {
	display_color_msg ${MAGENTA} "Loading tinycthread ..."
	eval ./install/install_tinycthread.sh ${REDIR}
	display_color_msg ${GREEN} "tinycthread installed"
}

# deps needed for freetype 
# libtool [libtool dependency for autogen.sh])
install_freetype() {
	display_color_msg ${MAGENTA} "Loading freetype ..."
	eval ./install/install_freetype.sh ${REDIR}
	display_color_msg ${GREEN} "freetype installed"
}

install_deps() {
	if [ ! -d "rsc/deps/cglm" ]; then
		install_cglm
	fi

	if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
		install_glfw3
	fi

	if [ ! -d "rsc/deps/tinycthread" ]; then
		install_tinycthread
	fi

	if [ ! -d "rsc/deps/freetype" ]; then
		install_freetype
	fi
}

check_and_install() {
    local file_to_check="$1"
    local dir_to_remove="$2"
    local install_function="$3"

    if [ ! -f "$file_to_check" ]; then
        display_color_msg "${YELLOW}" "$install_function check fail, removing and reinstalling ..."
        rm -rf "$dir_to_remove"
        eval "$install_function"
    fi
}

check_deps() {
	local cglm_check="rsc/deps/cglm/.cglm_installed"
	local glfw3_check="rsc/lib_deps/libglfw3.a"
	local tinycthread_check="rsc/lib_deps/libstatic_tinycthread.a"
	local freetype_check="rsc/lib_deps/libstaticfreetype.a"

	if [ ! -f "install/.deps_checked" ]; then
		display_color_msg ${MAGENTA} "Dependencies not checked, checking ..."
	fi

	check_and_install ${cglm_check} "rsc/deps/cglm" "install_cglm"
	check_and_install ${glfw3_check} "rsc/deps/glfw" "install_glfw3"
	check_and_install ${tinycthread_check} "rsc/deps/tinycthread" "install_tinycthread"
	check_and_install ${freetype_check} "rsc/deps/freetype" "install_freetype"

	touch install/.deps_checked
}

set_and_check_globale ${@}
create_dir "rsc/deps"
create_dir "rsc/lib_deps"
if [ "$MODE" == "install" ]; then
	install_deps
elif [ "$MODE" == "check" ]; then
	check_deps
fi