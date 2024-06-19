#!/bin/bash

# This script is used to update the perlin noise source code in the project.
make -C rsc/perlin_noise lib
mv rsc/perlin_noise/perlin_noise.a rsc/lib_deps/libperlin_noise.a