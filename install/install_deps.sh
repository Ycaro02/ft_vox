#!/bin/bash

source ./rsc/sh/color.sh

create_dir() {
	if [ ! -d "$1" ]; then
		mkdir -p $1
		display_color_msg ${LIGHT_CYAN} "Creating $1"
	fi
}

install_cglm() {
	display_color_msg ${MAGENTA} "Loading cglm ..."
	./install/load_glm.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "cglm installed"
}

install_glfw3() {
	display_color_msg ${MAGENTA} "Loading libglfw3 ..."
	./install/clone_compile_glfw3.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "libglfw3 installed"
}

install_tinycthread() {
	display_color_msg ${MAGENTA} "Loading tinycthread ..."
	./install/install_tinycthread.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "tinycthread installed"
}

install_freetype() {
	display_color_msg ${MAGENTA} "Loading freetype ..."
	./install/install_freetype.sh >/dev/null 2>&1
	display_color_msg ${GREEN} "freetype installed"
}

install_deps() {
	if [ ! -d "rsc/deps/cglm" ]; then
		install_cglm
	fi

	# deps needed for glfw3
	# apt install libwayland-dev libxkbcommon-dev xorg-dev
	# Cmake mandatory too
	if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
		install_glfw3
	fi

	if [ ! -d "rsc/deps/tinycthread" ]; then
		install_tinycthread
	fi

	# deps needed for freetype 
	# libtool [libtool dependency for autogen.sh])
	if [ ! -d "rsc/deps/freetype" ]; then
		install_freetype
	fi
}

# CGLM CHECK: rsc/deps/cglm/.cglm_installed
# GLFW3 CHECK: rsc/lib_deps/libglfw3.a
# TINYCTHREAD CHECK: rsc/lib_deps/libstatic_tinycthread.a
# FREETYPE CHECK: rsc/lib_deps/libstaticfreetype.a

check_deps() {
	if [ ! -f "install/.deps_checked" ]; then
		display_color_msg ${MAGENTA} "Dependencies not checked, checking ..."
	fi

	if [ ! -f "rsc/deps/cglm/.cglm_installed" ]; then
		display_color_msg ${YELLOW} "cglm check fail, remove and reinstalling ..." 
		rm -rf rsc/deps/cglm/
		install_cglm
	fi

	if [ ! -f "rsc/lib_deps/libglfw3.a" ]; then
		display_color_msg ${YELLOW} "libglfw3 check fail, remove and reinstalling ..."
		rm -rf rsc/deps/glfw
		install_glfw3
	fi

	if [ ! -f "rsc/lib_deps/libstatic_tinycthread.a" ]; then
		display_color_msg ${YELLOW} "tinycthread check fail, remove and reinstalling ..."
		rm -rf rsc/deps/tinycthread
		install_tinycthread
	fi

	if [ ! -f "rsc/lib_deps/libstaticfreetype.a" ]; then
		display_color_msg ${YELLOW} "freetype check fail, remove and reinstalling ..."
		rm -rf rsc/deps/freetype
		install_freetype
	fi
}

create_dir "rsc/deps"
create_dir "rsc/lib_deps"
install_deps
check_deps