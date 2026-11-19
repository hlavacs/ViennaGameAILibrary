@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem Builds the Vienna Game AI Library (VGAI) on Windows with CMake and Ninja,
rem then runs the regression tests. The library itself is one header
rem (include\ViennaGameAILibrary.hpp), so "building" means: the regression
rem tests, the raylib demos, and optionally the Doxygen documentation.
rem
rem Usage: build_windows.cmd [debug|release] [--clean] [--no-demos] [--no-tests]
rem                          [--docs] [--clang] [--run [name]]
rem
rem Requires: Visual Studio 2022 or newer with the C++ workload and the
rem           "C++ CMake tools for Windows" (they provide cl, CMake, CTest and
rem           Ninja). The demos also need Git once, to fetch the raylib
rem           submodule. --clang needs the "C++ Clang tools for Windows"
rem           component or another clang++.exe on PATH. --docs needs Doxygen.
rem From PowerShell: .\build_windows.cmd debug

pushd "%~dp0" || exit /b 1
set "REPO_ROOT=%CD%"
set "SCRIPT_NAME=%~nx0"
set "VARIANT=release"
set "CLEAN=0"
set "BUILD_DEMOS=1"
set "RUN_TESTS=1"
set "BUILD_DOCS=0"
set "USE_CLANG=0"
set "RUN_DEMO=0"
set "RUN_NAME="
set "STAGE=Prerequisite checks"

:parse
if "%~1"=="" goto done_parse
if /I "%~1"=="debug" (set "VARIANT=debug"& goto next_arg)
if /I "%~1"=="release" (set "VARIANT=release"& goto next_arg)
if /I "%~1"=="--clean" (set "CLEAN=1"& goto next_arg)
if /I "%~1"=="--no-demos" (set "BUILD_DEMOS=0"& goto next_arg)
if /I "%~1"=="--no-tests" (set "RUN_TESTS=0"& goto next_arg)
if /I "%~1"=="--docs" (set "BUILD_DOCS=1"& goto next_arg)
if /I "%~1"=="--clang" (set "USE_CLANG=1"& goto next_arg)
if /I "%~1"=="--run" (set "RUN_DEMO=1"& goto run_arg)
if /I "%~1"=="-h" goto help
if /I "%~1"=="--help" goto help
echo Unknown argument: "%~1"
call :usage
goto fail
:next_arg
shift
goto parse
:run_arg
rem --run may be followed by the name of a demo. A word that starts with "-",
rem or debug or release, is the next option and not a name.
shift
if "%~1"=="" goto done_parse
set "RUN_NAME=%~1"
if "%RUN_NAME:~0,1%"=="-" (set "RUN_NAME="& goto parse)
if /I "%RUN_NAME%"=="debug" (set "RUN_NAME="& goto parse)
if /I "%RUN_NAME%"=="release" (set "RUN_NAME="& goto parse)
goto next_arg

:done_parse
if /I "%VARIANT%"=="debug" (set "CONFIG=Debug") else (set "CONFIG=Release")
rem One build directory per configuration and compiler, so they never mix.
set "BUILD_DIR=%REPO_ROOT%\build\%VARIANT%-windows"
if "%USE_CLANG%"=="1" set "BUILD_DIR=%BUILD_DIR%-clang"
if "%RUN_DEMO%"=="1" if "%BUILD_DEMOS%"=="0" (
    echo --run starts a demo, so it cannot be combined with --no-demos.
    goto fail
)

rem --- Always refresh MSVC and prefer Visual Studio's CMake and Ninja over other PATH entries ---
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto no_vswhere
set "VSINSTALL="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL goto no_vs
echo Initializing MSVC environment from "%VSINSTALL%" ...
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 goto fail
set "PATH=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

rem --- Compiler: x64 MSVC by default, clang++ with --clang ---
set "CXX_NAME=cl.exe"
set "C_NAME=cl.exe"
if "%USE_CLANG%"=="0" goto compiler_chosen
set "CXX_NAME=clang++.exe"
set "C_NAME=clang.exe"
rem Prefer the clang that ships with Visual Studio, if that component is installed.
if exist "%VSINSTALL%\VC\Tools\Llvm\x64\bin\clang++.exe" set "PATH=%VSINSTALL%\VC\Tools\Llvm\x64\bin;%PATH%"
:compiler_chosen
set "COMPILER_ARGS=-DCMAKE_CXX_COMPILER=%CXX_NAME%"
rem Only raylib is written in C, so a C compiler is named only when the demos are built.
if "%BUILD_DEMOS%"=="1" set "COMPILER_ARGS=%COMPILER_ARGS% -DCMAKE_C_COMPILER=%C_NAME%"

for %%T in (%CXX_NAME% cmake.exe ninja.exe) do (
    where %%T >nul 2>nul
    if errorlevel 1 (
        echo %%T not found. Install the C++ workload and the C++ CMake tools in Visual Studio Installer.
        if "%USE_CLANG%"=="1" echo For --clang also install the component "C++ Clang tools for Windows".
        goto fail
    )
)
if "%RUN_TESTS%"=="1" (
    where ctest.exe >nul 2>nul
    if errorlevel 1 (
        echo ctest.exe not found. Install the C++ CMake tools in Visual Studio Installer, or use --no-tests.
        goto fail
    )
)

rem --- The demos need raylib, which is a Git submodule in Demo\raylib ---
if "%BUILD_DEMOS%"=="1" if not exist "%REPO_ROOT%\Demo\raylib\CMakeLists.txt" (
    set "STAGE=Fetching the raylib submodule"
    call :fetch_raylib
    if errorlevel 1 goto fail
)

rem Debug programs need the debug runtime DLLs of MSVC and of the Windows SDK.
if /I "%CONFIG%"=="Debug" call :add_debug_runtime

if "%CLEAN%"=="1" if exist "%BUILD_DIR%" (
    set "STAGE=Build directory cleanup"
    echo Removing "%BUILD_DIR%" ...
    rmdir /s /q "%BUILD_DIR%"
    if exist "%BUILD_DIR%" goto fail
)

set "DEMO_ARG=-DVGAI_BUILD_DEMOS=ON"
if "%BUILD_DEMOS%"=="0" set "DEMO_ARG=-DVGAI_BUILD_DEMOS=OFF"
set "TEST_ARG=-DBUILD_TESTING=ON"
if "%RUN_TESTS%"=="0" set "TEST_ARG=-DBUILD_TESTING=OFF"
rem CMakeLists.txt adds the Doxygen target to every build as soon as Doxygen is
rem installed. That is slow and rewrites docs\, so it is switched off unless
rem --docs is given.
set "DOCS_ARG=-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON"
if "%BUILD_DOCS%"=="1" set "DOCS_ARG=-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=OFF"

rem --- Configure with Ninja (single configuration) ---
set "STAGE=CMake configuration"
cmake -S "%REPO_ROOT%" -B "%BUILD_DIR%" -G Ninja ^
    -DCMAKE_BUILD_TYPE=%CONFIG% ^
    %COMPILER_ARGS% %DEMO_ARG% %TEST_ARG% %DOCS_ARG%
if errorlevel 1 goto fail

if "%BUILD_DOCS%"=="1" (
    findstr /C:"DOXYGEN_EXECUTABLE:FILEPATH=DOXYGEN_EXECUTABLE-NOTFOUND" "%BUILD_DIR%\CMakeCache.txt" >nul 2>nul
    if not errorlevel 1 (
        set "STAGE=Doxygen documentation"
        echo --docs was given, but CMake did not find Doxygen. Install it from https://www.doxygen.nl
        goto fail
    )
)

set "STAGE=Compilation and linking"
cmake --build "%BUILD_DIR%"
if errorlevel 1 goto fail

if "%RUN_TESTS%"=="1" (
    set "STAGE=Tests after successful compilation and linking"
    ctest --test-dir "%BUILD_DIR%" --output-on-failure
    if errorlevel 1 goto fail
)

echo.
echo %CONFIG% build complete: "%BUILD_DIR%"
if "%BUILD_DEMOS%"=="1" echo Demos: "%BUILD_DIR%\Demo". Start one with .\run_demo_win.bat or with --run.
if "%BUILD_DOCS%"=="1" echo Documentation: "%REPO_ROOT%\docs\html\index.html"

if "%RUN_DEMO%"=="1" (
    set "STAGE=Running the demo"
    rem run_demo_win.bat starts the named demo, or shows a menu if no name was given.
    rem It takes the programs from the build directory of this run.
    set "VGAI_DEMO_DIR=%BUILD_DIR%\Demo"
    call "%REPO_ROOT%\run_demo_win.bat" %RUN_NAME%
    if errorlevel 1 goto fail
)
popd
exit /b 0

:help
call :usage
popd
exit /b 0

:usage
echo Usage: .\%SCRIPT_NAME% [debug^|release] [--clean] [--no-demos] [--no-tests] [--docs] [--clang] [--run [name]]
echo.
echo   debug, release  Build configuration. Default: release.
echo   --clean         Delete the build directory of this configuration first.
echo   --no-demos      Do not build the raylib demos. Git and raylib are then not needed.
echo   --no-tests      Do not build and run the regression tests.
echo   --docs          Also generate the Doxygen documentation in docs\html.
echo   --clang         Compile with clang++ instead of MSVC. Uses its own build directory.
echo   --run [name]    Start a demo after a successful build, for example --run mcts.
echo                   Without a name a menu with all demos is shown.
echo.
echo All demos are built. .\run_demo_win.bat starts them without building.
exit /b 0

:no_vswhere
echo vswhere.exe not found. Install Visual Studio with the C++ workload.
goto fail

:no_vs
echo No Visual Studio with the C++ toolchain was found.
goto fail

:fetch_raylib
where git.exe >nul 2>nul
if errorlevel 1 (
    echo Demo\raylib is empty and git.exe was not found.
    echo Install Git, or build without the demos: .\%SCRIPT_NAME% --no-demos
    exit /b 1
)
call :repair_raylib
echo Demo\raylib is empty. Downloading the raylib submodule, this can take a minute ...
rem A shallow download fetches only the one commit that this repository uses.
git -C "%REPO_ROOT%" submodule update --init --depth 1 --progress -- Demo/raylib
if not errorlevel 1 goto raylib_fetched
echo The shallow download failed. Trying a full download ...
call :repair_raylib
git -C "%REPO_ROOT%" submodule update --init --progress -- Demo/raylib
if errorlevel 1 (
    echo The raylib submodule could not be fetched. See the Git messages above,
    echo or build without the demos: .\%SCRIPT_NAME% --no-demos
    exit /b 1
)
:raylib_fetched
if not exist "%REPO_ROOT%\Demo\raylib\CMakeLists.txt" (
    echo Demo\raylib still has no CMakeLists.txt after the submodule update.
    exit /b 1
)
exit /b 0

:repair_raylib
rem A download that was interrupted or that failed can leave a module directory
rem without HEAD in .git\modules. Git then stops every later attempt with
rem "could not get a repository handle for submodule". Remove these leftovers,
rem so that Git starts the download again. Only done in an ordinary checkout,
rem where .git is a directory.
if not exist "%REPO_ROOT%\.git\HEAD" exit /b 0
set "RAYLIB_GITDIR=%REPO_ROOT%\.git\modules\Demo\raylib"
if exist "%RAYLIB_GITDIR%\HEAD" exit /b 0
if exist "%RAYLIB_GITDIR%" (
    echo Removing the leftovers of an incomplete raylib download ...
    rmdir /s /q "%RAYLIB_GITDIR%"
)
rem Git writes a small, hidden link file Demo\raylib\.git. Without the module
rem directory it points nowhere. /a lets del remove a hidden file.
rem The attribute string of a directory starts with d. Only a file is deleted.
set "GITLINK_ATTR="
for %%A in ("%REPO_ROOT%\Demo\raylib\.git") do set "GITLINK_ATTR=%%~aA"
if defined GITLINK_ATTR if /I not "%GITLINK_ATTR:~0,1%"=="d" del /f /q /a "%REPO_ROOT%\Demo\raylib\.git"
exit /b 0

:add_debug_runtime
rem Best effort: these DLLs are not redistributable and are not always on PATH.
rem If they are not found here, Windows may still find them in System32.
set "DEBUG_CRT="
for /d %%V in ("%VSINSTALL%\VC\Redist\MSVC\*") do call :find_debug_crt "%%~fV"
set "DEBUG_UCRT="
for /d %%D in ("%ProgramFiles(x86)%\Windows Kits\10\bin\*") do if exist "%%~fD\x64\ucrt\ucrtbased.dll" set "DEBUG_UCRT=%%~fD\x64\ucrt"
if defined DEBUG_CRT set "PATH=%DEBUG_CRT%;%PATH%"
if defined DEBUG_UCRT set "PATH=%DEBUG_UCRT%;%PATH%"
exit /b 0

:find_debug_crt
rem %1 is one MSVC redistributable version directory. The folder name contains
rem the toolset number, for example Microsoft.VC145.DebugCRT.
for /d %%D in ("%~1\debug_nonredist\x64\Microsoft.VC*.DebugCRT") do if exist "%%~fD\msvcp140d.dll" set "DEBUG_CRT=%%~fD"
exit /b 0

:fail
echo.
echo %STAGE% failed.
popd
exit /b 1
