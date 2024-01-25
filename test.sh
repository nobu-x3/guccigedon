#!/bin/sh

set echo off
mkdir -p build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cd build
make -j32
./Tests
# ctest -R Guccigedon_* -j 32
