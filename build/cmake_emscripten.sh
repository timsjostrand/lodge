#!/bin/bash -e

EMSCRIPTEN_TOOLCHAIN="/Users/tim/Code/emscripten/cmake/Modules/Platform/Emscripten.cmake"

#rm -rf emscripten
mkdir emscripten || true
cd emscripten
cmake -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_TOOLCHAIN} -DEMSCRIPTEN=1 -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ../../
make VERBOSE=1
python -m SimpleHTTPServer 8080
cd ..
