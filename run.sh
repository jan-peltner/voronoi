#!/bin/sh

set -ex

./build.sh && cd bin && ./voronoi
