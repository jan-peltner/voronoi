#!/bin/sh

set -ex

mkdir -p bin

cc -Wall -Wextra -o bin/voronoi src/main.c -lraylib -lm

# Copy shaders to bin
cd bin
cp ../src/shaders/*.vert .
cp ../src/shaders/*.frag .
