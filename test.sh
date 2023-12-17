#!/bin/sh

set echo off
mkdir -p build
cmake -S . -B build
cmake --build build
cd build
make Tests -j32
ctest -R Guccigedon_* -j 32
