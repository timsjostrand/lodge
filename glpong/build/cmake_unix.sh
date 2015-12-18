#!/bin/bash -e
BUILD_TYPE=${BUILD_TYPE:-"Debug"}
mkdir unix || true
cd unix
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ../../
make
cd ..
