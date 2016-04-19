if not defined BUILD_TYPE set BUILD_TYPE="Debug"
mkdir clang
cd clang
mkdir "%BUILD_TYPE%"
cd "%BUILD_TYPE%"
cmake -DCMAKE_CC_COMPILER=clang-cl -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ../../../
make run
cd ../../
pause