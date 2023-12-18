#!/bin/sh

set echo off
mkdir -p build
cmake -S . -B build
cmake --build build
cd build
make -j32
./Tests
# ctest -R Guccigedon_* -j 32
