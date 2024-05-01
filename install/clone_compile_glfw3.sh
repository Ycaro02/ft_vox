#!/bin/bash

git clone https://github.com/glfw/glfw.git rsc/deps/glfw && cd rsc/deps && cmake -S glfw -B build_glfw && cd build_glfw && make && cp src/libglfw3.a ../../lib_deps
echo "GLFW3 installed"
rm -rf rsc/deps/glfw rsc/deps/build_glfw
echo "GLFW3 cleaned up"
