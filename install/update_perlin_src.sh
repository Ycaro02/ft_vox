#!/bin/bash

# This script is used to update the perlin noise source code in the project.
rm -rf src/perlin_noise && mkdir src/perlin_noise
cp rsc/perlin_noise/src/perlin/*.c src/perlin_noise
