#!/usr/bin/env bash
set -euo pipefail

# Builds the Vienna Game AI Library (VGAI) on Linux with CMake, then runs the
# regression tests. The library itself is one header
# (include/ViennaGameAILibrary.hpp), so "building" means: the regression
# tests, the raylib demos, and optionally the Doxygen documentation.
#
# Usage: ./build_linux.sh [debug|release] [--clean] [--no-demos] [--no-tests]
#                         [--docs] [--run [name]]
#
# Requires: CMake 3.28 or newer, a C++20 compiler (GCC 10 or newer, or Clang),
#           and Ninja or Make. The demos also need Git once, to fetch the raylib
#           submodule, and the X11 and OpenGL development packages (the script
#           names them if they are missing). --docs needs Doxygen.
#           CXX and CC select other compilers, for example: CXX=clang++ CC=clang

cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

JOBS="${CMAKE_BUILD_PARALLEL_LEVEL:-$(nproc 2>/dev/null || printf '8')}"
VARIANT="Release"
CLEAN=0
BUILD_DEMOS=1
RUN_TESTS=1
BUILD_DOCS=0
RUN_DEMO=0
RUN_NAME=""

usage() {
  printf 'Usage: %s [debug|release] [--clean] [--no-demos] [--no-tests] [--docs] [--run [name]]\n\n' "$0"
  printf '  debug, release  Build configuration. Default: release.\n'
  printf '  --clean         Delete the build directory of this configuration first.\n'
  printf '  --no-demos      Do not build the raylib demos. Git, raylib and X11 are then not needed.\n'
  printf '  --no-tests      Do not build and run the regression tests.\n'
  printf '  --docs          Also generate the Doxygen documentation in docs/html.\n'
  printf '  --run [name]    Start a demo after a successful build, for example --run mcts.\n'
  printf '                  Without a name a menu with all demos is shown.\n\n'
  printf 'All demos are built. ./run_demo.sh starts them without building.\n'
}

fail() {
  printf '\n%s\n' "$1" >&2
  exit 1
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    debug|Debug|DEBUG) VARIANT="Debug" ;;
    release|Release|RELEASE) VARIANT="Release" ;;
    --clean) CLEAN=1 ;;
    --no-demos) BUILD_DEMOS=0 ;;
    --no-tests) RUN_TESTS=0 ;;
    --docs) BUILD_DOCS=1 ;;
    --run)
      RUN_DEMO=1
      # --run may be followed by the name of a demo. A word that starts with "-",
      # or debug or release, is the next option and not a name.
      if [ "$#" -gt 1 ]; then
        case "$2" in
          -*|debug|Debug|DEBUG|release|Release|RELEASE) ;;
          *) RUN_NAME="$2"; shift ;;
        esac
      fi
      ;;
    -h|--help) usage; exit 0 ;;
    *) printf 'Unknown argument: "%s"\n' "$1" >&2; usage >&2; exit 1 ;;
  esac
  shift
done

if [ "$RUN_DEMO" -eq 1 ] && [ "$BUILD_DEMOS" -eq 0 ]; then
  fail '--run starts a demo, so it cannot be combined with --no-demos.'
fi

VARIANT_LOWER="$(printf '%s' "$VARIANT" | tr '[:upper:]' '[:lower:]')"
BUILD_DIR="build/${VARIANT_LOWER}-linux"

# --- Tools ---
command -v cmake >/dev/null 2>&1 || fail 'cmake not found. Install it with your package manager.'
if [ "$RUN_TESTS" -eq 1 ]; then
  command -v ctest >/dev/null 2>&1 || fail 'ctest not found. It is installed together with CMake.'
fi
# Ninja is preferred. Make is used if Ninja is not installed.
if command -v ninja >/dev/null 2>&1; then
  GENERATOR="Ninja"
elif command -v make >/dev/null 2>&1; then
  GENERATOR="Unix Makefiles"
else
  fail 'Neither ninja nor make was found. Install one of them, for example: sudo apt install ninja-build'
fi

# --- Compiler: CXX and CC if they are set, otherwise the default compilers ---
CXX_BIN="${CXX:-c++}"
CC_BIN="${CC:-cc}"
command -v "$CXX_BIN" >/dev/null 2>&1 || fail "C++ compiler \"$CXX_BIN\" not found. Install g++ or clang, or set CXX."

# The MCTS part of the library uses std::jthread from C++20. Compile and run a
# tiny program to see early, and with a clear message, whether this compiler has it.
CHECK_DIR="$(mktemp -d)"
trap 'rm -rf "$CHECK_DIR"' EXIT
cat > "$CHECK_DIR/check.cpp" <<'CHECK'
#include <thread>
int main() { std::jthread t([] {}); return 0; }
CHECK
if ! "$CXX_BIN" -std=c++20 -pthread "$CHECK_DIR/check.cpp" -o "$CHECK_DIR/check" >/dev/null 2>&1 || ! "$CHECK_DIR/check"; then
  fail "The compiler \"$CXX_BIN\" cannot build a C++20 program with std::jthread. Use GCC 10 or newer, or a current Clang, for example: CXX=g++-13 $0"
fi

# --- The demos need raylib, and raylib needs the X11 and OpenGL development files ---
if [ "$BUILD_DEMOS" -eq 1 ]; then
  command -v "$CC_BIN" >/dev/null 2>&1 || fail "C compiler \"$CC_BIN\" not found. raylib is written in C. Install gcc or clang, or set CC."
  MISSING=""
  for header in X11/Xlib.h X11/extensions/Xrandr.h X11/extensions/Xinerama.h X11/Xcursor/Xcursor.h X11/extensions/XInput2.h GL/gl.h; do
    if ! printf '#include <%s>\n' "$header" | "$CC_BIN" -E -x c - >/dev/null 2>&1; then
      MISSING="$MISSING $header"
    fi
  done
  if [ -n "$MISSING" ]; then
    printf '\nraylib needs these development files, which were not found:%s\n' "$MISSING" >&2
    printf 'Install them, for example:\n' >&2
    printf '  Debian, Ubuntu: sudo apt install libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev libgl1-mesa-dev\n' >&2
    printf '  Fedora:         sudo dnf install libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel mesa-libGL-devel\n' >&2
    printf '  Arch:           sudo pacman -S libx11 libxrandr libxi libxcursor libxinerama mesa\n' >&2
    printf 'Or build without the demos: %s --no-demos\n' "$0" >&2
    exit 1
  fi
fi

# A download that was interrupted or that failed can leave a module directory
# without HEAD in .git/modules. Git then stops every later attempt with
# "could not get a repository handle for submodule". Remove these leftovers,
# so that Git starts the download again. Only done in an ordinary checkout,
# where .git is a directory.
repair_raylib() {
  [ -f .git/HEAD ] || return 0
  [ -f .git/modules/Demo/raylib/HEAD ] && return 0
  if [ -d .git/modules/Demo/raylib ]; then
    printf 'Removing the leftovers of an incomplete raylib download ...\n'
    rm -rf .git/modules/Demo/raylib
  fi
  # Git writes a small link file Demo/raylib/.git. Without the module directory it points nowhere.
  if [ -f Demo/raylib/.git ]; then
    rm -f Demo/raylib/.git
  fi
  return 0
}

if [ "$BUILD_DEMOS" -eq 1 ] && [ ! -f Demo/raylib/CMakeLists.txt ]; then
  command -v git >/dev/null 2>&1 || fail "Demo/raylib is empty and git was not found. Install Git, or build without the demos: $0 --no-demos"
  repair_raylib
  printf 'Demo/raylib is empty. Downloading the raylib submodule, this can take a minute ...\n'
  # A shallow download fetches only the one commit that this repository uses.
  if ! git submodule update --init --depth 1 --progress -- Demo/raylib; then
    printf 'The shallow download failed. Trying a full download ...\n'
    repair_raylib
    git submodule update --init --progress -- Demo/raylib ||
      fail "The raylib submodule could not be fetched. See the Git messages above, or build without the demos: $0 --no-demos"
  fi
  [ -f Demo/raylib/CMakeLists.txt ] || fail 'Demo/raylib still has no CMakeLists.txt after the submodule update.'
fi

# --- Build directory ---
if [ "$CLEAN" -eq 1 ] && [ -d "$BUILD_DIR" ]; then
  printf 'Removing %s ...\n' "$BUILD_DIR"
  rm -rf "$BUILD_DIR"
elif [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
  # CMake cannot change the generator of an existing build directory.
  if ! grep -q "^CMAKE_GENERATOR:INTERNAL=${GENERATOR}\$" "$BUILD_DIR/CMakeCache.txt"; then
    printf 'The existing build directory was made with another generator; recreating %s.\n' "$BUILD_DIR"
    rm -rf "$BUILD_DIR"
  fi
fi

CMAKE_ARGS=(-DCMAKE_BUILD_TYPE="$VARIANT" -DCMAKE_CXX_COMPILER="$CXX_BIN")
if [ "$BUILD_DEMOS" -eq 1 ]; then
  # Only raylib is written in C, so a C compiler is named only when the demos are built.
  CMAKE_ARGS+=(-DVGAI_BUILD_DEMOS=ON -DCMAKE_C_COMPILER="$CC_BIN")
else
  CMAKE_ARGS+=(-DVGAI_BUILD_DEMOS=OFF)
fi
if [ "$RUN_TESTS" -eq 1 ]; then
  CMAKE_ARGS+=(-DBUILD_TESTING=ON)
else
  CMAKE_ARGS+=(-DBUILD_TESTING=OFF)
fi
# CMakeLists.txt adds the Doxygen target to every build as soon as Doxygen is
# installed. That is slow and rewrites docs/, so it is switched off unless
# --docs is given.
if [ "$BUILD_DOCS" -eq 1 ]; then
  command -v doxygen >/dev/null 2>&1 || fail '--docs was given, but doxygen was not found. Install it with your package manager.'
  CMAKE_ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=OFF)
else
  CMAKE_ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON)
fi

printf 'Configuring %s build with %s and %s ...\n' "$VARIANT" "$GENERATOR" "$CXX_BIN"
cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" "${CMAKE_ARGS[@]}" || fail 'CMake configuration failed.'

cmake --build "$BUILD_DIR" --parallel "$JOBS" || fail 'Compilation and linking failed.'

if [ "$RUN_TESTS" -eq 1 ]; then
  ctest --test-dir "$BUILD_DIR" --output-on-failure || fail 'Tests failed.'
fi

printf '\n%s build complete: %s\n' "$VARIANT" "$BUILD_DIR"
if [ "$BUILD_DEMOS" -eq 1 ]; then
  printf 'Demos: %s/Demo. Start one with ./run_demo.sh or with --run.\n' "$BUILD_DIR"
fi
if [ "$BUILD_DOCS" -eq 1 ]; then
  printf 'Documentation: docs/html/index.html\n'
fi

if [ "$RUN_DEMO" -eq 1 ]; then
  # run_demo.sh starts the named demo, or shows a menu if no name was given.
  # It takes the programs from the build directory of this run.
  export VGAI_DEMO_DIR="$BUILD_DIR/Demo"
  if [ -n "$RUN_NAME" ]; then
    bash "$REPO_ROOT/run_demo.sh" "$RUN_NAME"
  else
    bash "$REPO_ROOT/run_demo.sh"
  fi
fi
