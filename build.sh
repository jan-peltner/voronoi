#!/bin/sh

set -ex

mkdir -p bin

cc -Wall -Wextra -o bin/voronoi src/main.c -lraylib -lm
