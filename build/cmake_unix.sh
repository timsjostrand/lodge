#!/bin/bash -e
mkdir unix || true
cd unix
cmake -G "Unix Makefiles" ../../
make
cd ..
