if not defined BUILD_TYPE set BUILD_TYPE="Debug"
mkdir mingw
cd mingw
mkdir "%BUILD_TYPE%"
cd "%BUILD_TYPE%"
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ../../../
mingw32-make run
cd ../../
pause