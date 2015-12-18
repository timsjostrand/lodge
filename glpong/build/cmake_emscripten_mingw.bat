mkdir emscripten
cd emscripten

@echo off

:: Set the paths to Emscripten.cmake and mingw32-make.exe
set EMSCRIPTEN_TOOLCHAIN="C:/Program Files/Emscripten/emscripten/tag-1.34.6/cmake/Modules/Platform/Emscripten.cmake"
set MINGW_MAKE="C:/MinGW/bin/mingw32-make.exe"

cmake -DTHREADS_PTHREAD_ARG=0 -DEMSCRIPTEN=1 -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN_TOOLCHAIN% -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ../../

::"C:/MinGW/bin/mingw32-make.exe"
cd ..
