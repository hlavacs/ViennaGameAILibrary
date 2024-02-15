# ViennaGameAILibrary
A library containing game AI algorithms.

### Folder structure
- Demo
	- [raylib](https://github.com/raysan5/raylib)
	- res
	- src
- docs: Contains the documentation for this project. Open ```index.html``` from docs/html to see the entire documentation.
- include: ```ViennaGameAILibrary.hpp```
- ```build_demo_win.bat```: Script to build the project on Windows.
- ```run_demo_win.bat```: Script to run the project on Windows.
- ```CMakeLists.txt```
- ```README.md```

### Setup - Windows
#### Prerequisites
- [Doxygen](https://www.doxygen.nl/index.html)
- [Msys2](https://www.msys2.org/) - CMake and Clang need to be installed through msys2.
	- CMake (minimum version required: 3.28.1)
 	- Clang

#### Build and run the project
1. Clone the project.
2. Run ```git submodule init``` and ```git submodule update``` to fetch raylib.
3. Update the location of ```clang++.exe``` in the ```build_demo_win.bat``` file based on the location of the msys2 folder.
2. Add *msys64/ucrt64/bin* and *doxygen/bin* to the path environment variable.
3. Run ```build_demo_win.bat``` to build the project. The output will be stored in the build folder.
4. Run ```run_demo_win.bat``` to run the project.