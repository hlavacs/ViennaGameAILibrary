@echo off

if not exist "build" mkdir build
cd build/
cmake -DCMAKE_CXX_COMPILER="clang++.exe" -DCMAKE_BUILD_TYPE=Debug ../
ninja

pause