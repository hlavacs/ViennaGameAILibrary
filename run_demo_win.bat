@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem Starts the demos of the Vienna Game AI Library.
rem
rem Usage: run_demo_win.bat [name]
rem   name   one of the demos listed below, with or without the prefix "demo_",
rem          for example: run_demo_win.bat mcts
rem   Without a name the script shows a menu.
rem
rem Build the demos first with build_windows.cmd. The demos are started from
rem the repository root, because they load their images and fonts as
rem Demo\res\... relative to that folder.

pushd "%~dp0" || exit /b 1

rem --- Find the folder with the demo programs ---
rem build_windows.cmd sets VGAI_DEMO_DIR when it starts a demo itself. Otherwise
rem the first build directory that contains the demos is used.
set "DEMO_DIR=%VGAI_DEMO_DIR%"
if defined DEMO_DIR goto have_dir
for %%D in ("build\release-windows\Demo" "build\debug-windows\Demo" "build\release-windows-clang\Demo" "build\debug-windows-clang\Demo" "build\Demo") do call :try_dir "%%~D"
:have_dir
if not defined DEMO_DIR goto not_built
if not exist "%DEMO_DIR%\demo_*.exe" goto not_built

if not "%~1"=="" goto run_named

:menu
echo.
echo Demos of the Vienna Game AI Library  [%DEMO_DIR%]
echo.
echo   Main themes
echo    1  Pathfinding         left click: start, right click: target, R: search
echo    2  DecisionTree        watch the guard and the burglar decide
echo    3  StateMachine        watch the worker change its state
echo    4  SteeringBehaviors   watch chickens, dog and snake; day and night change
echo    5  Flocking            click a box, type a number, press Enter
echo    6  MCTS                Connect Four against the search; keys are shown in the window
echo.
echo   Single steering behaviours
echo    7  SeekAndFlee         arrow keys move the target
echo    8  PursueAndEvade      arrow keys move the target
echo    9  Arrive              left click sets the target
echo   10  Wander              watch
echo.
set "CHOICE="
set /p "CHOICE=Number or name, Enter to quit: "
if not defined CHOICE goto done
if /I "%CHOICE%"=="q" goto done
call :start_demo "%CHOICE%"
goto menu

:run_named
call :start_demo "%~1"
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%

:done
popd
exit /b 0

:not_built
echo The demos were not found. Build them first: .\build_windows.cmd
popd
exit /b 1

:try_dir
if defined DEMO_DIR exit /b 0
if exist "%~1\demo_*.exe" set "DEMO_DIR=%~1"
exit /b 0

:start_demo
rem %1 is a menu number or a demo name with or without the prefix "demo_".
set "DEMO=%~1"
if "%DEMO%"=="1" set "DEMO=Pathfinding"
if "%DEMO%"=="2" set "DEMO=DecisionTree"
if "%DEMO%"=="3" set "DEMO=StateMachine"
if "%DEMO%"=="4" set "DEMO=SteeringBehaviors"
if "%DEMO%"=="5" set "DEMO=Flocking"
if "%DEMO%"=="6" set "DEMO=MCTS"
if "%DEMO%"=="7" set "DEMO=SeekAndFlee"
if "%DEMO%"=="8" set "DEMO=PursueAndEvade"
if "%DEMO%"=="9" set "DEMO=Arrive"
if "%DEMO%"=="10" set "DEMO=Wander"
if /I "%DEMO:~0,5%"=="demo_" set "DEMO=%DEMO:~5%"
if not exist "%DEMO_DIR%\demo_%DEMO%.exe" (
    echo There is no demo "%DEMO%" in "%DEMO_DIR%".
    exit /b 1
)
echo Starting demo_%DEMO% ...
"%DEMO_DIR%\demo_%DEMO%.exe"
exit /b %ERRORLEVEL%
