# ViennaGameAILibrary
A library containing game AI algorithms.

### Folder structure
- Demo
	- [raylib](https://github.com/raysan5/raylib)
	- src: Source folder containing the code for the demos
	- res: Resource folder containing assets
	- ```CMakeLists.txt```
- docs: Contains the documentation for this project.
- include: ```ViennaGameAILibrary.hpp```
- ```build_demo_win.bat```: Script to build the project on Windows.
- ```run_demo_win.bat```: Script to run the project on Windows.
- ```CMakeLists.txt```
- ```README.md```

### Setup - Windows
#### Prerequisites
- [Doxygen](https://www.doxygen.nl/index.html)
- [Ninja](https://ninja-build.org/)
- [Msys2](https://www.msys2.org/) - CMake and Clang need to be installed through msys2.
	- CMake (minimum version required: 3.28.1)
 	- Clang

#### Build and run the project
1. Clone the project.
2. Run ```git submodule init``` and ```git submodule update``` to fetch raylib.
3. Update the location of ```clang++.exe``` in the ```build_demo_win.bat``` file based on the location of the msys2 folder.
4. Add *msys64/ucrt64/bin* and *doxygen/bin* to the path environment variable.
5. Run ```build_demo_win.bat``` to build the project. The output will be stored in the build folder.
6. Run ```run_demo_win.bat``` to run the project.

#### To run different demos
In Demo/```CMakeLists.txt```, change the path of the cpp file that needs to be run.
```
add_executable(Demo src/demo_PathFinding.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
```

#### Documentation
Documentation is generated using Doxygen. To see it, open ```index.html``` from *docs/html*.