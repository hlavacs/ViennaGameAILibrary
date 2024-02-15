@echo off

if not exist "build" mkdir build
cd build/
cmake -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/clang++.exe" -DCMAKE_BUILD_TYPE=Debug ../
ninja

pause
