#!/bin/bash

# This script is used to update the perlin noise source code in the project.
make -s -C rsc/perlin_noise lib && mkdir -p rsc/lib_deps && mv rsc/perlin_noise/perlin_noise.a rsc/lib_deps/libperlin_noise.a