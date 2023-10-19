#!/bin/bash

set echo on
mkdir -p build
mkdir -p bin
cp -rf assets build/
rm -rf bin/*
cmake -B build
cd build
make Shaders
make
# cd ..
# cp build/Guccigedon bin/Guccigedon
./Guccigedon
