#!/bin/bash

set echo on
mkdir -p build
mkdir -p bin
rm -rf bin/*
cmake -B build
cd build
make
cd ..
cp build/Guccigedon bin/Guccigedon
./bin/Guccigedon
