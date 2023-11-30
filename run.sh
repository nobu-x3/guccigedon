#!/bin/sh

set echo off
mkdir -p build
mkdir -p bin
cp -rf assets build/
rm -rf bin/*
cmake -B build
cd build
make Shaders -j32
make -j32
# cd ..
# cp build/Guccigedon bin/Guccigedon
./Guccigedon
