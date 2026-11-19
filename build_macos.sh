#!/usr/bin/env bash
set -euo pipefail

# Builds the Vienna Game AI Library (VGAI) on macOS with CMake, then runs the
# regression tests. The library itself is one header
# (include/ViennaGameAILibrary.hpp), so "building" means: the regression
# tests, the raylib demos, and optionally the Doxygen documentation.
#
# Usage: ./build_macos.sh [debug|release] [--clean] [--no-demos] [--no-tests]
#                         [--docs] [--run [name]]
#
# Requires: the Xcode Command Line Tools (xcode-select --install) and CMake
#           3.28 or newer (brew install cmake). Ninja is used if it is installed
#           (brew install ninja), otherwise Make. The demos also need Git once,
#           to fetch the raylib submodule. --docs needs Doxygen.
#
# Compiler: the MCTS part of the library uses std::jthread from C++20. The
#           script tries Apple's clang++ first, also with the option
#           -fexperimental-library, and then the clang++ of Homebrew LLVM
#           (brew install llvm). It uses the first one that can build and run a
#           small test program. CXX selects another compiler, LLVM_ROOT another
#           LLVM installation.

cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

JOBS="${CMAKE_BUILD_PARALLEL_LEVEL:-$(sysctl -n hw.ncpu 2>/dev/null || printf '8')}"
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
  printf '  --no-demos      Do not build the raylib demos. Git and raylib are then not needed.\n'
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

if [ "$(uname -s)" != "Darwin" ]; then
  fail 'This script is for macOS. Use build_linux.sh on Linux and build_windows.cmd on Windows.'
fi

VARIANT_LOWER="$(printf '%s' "$VARIANT" | tr '[:upper:]' '[:lower:]')"
BUILD_DIR="build/${VARIANT_LOWER}-macos"

# --- Tools ---
xcode-select -p >/dev/null 2>&1 || fail 'The Xcode Command Line Tools were not found. Install them with: xcode-select --install'
command -v cmake >/dev/null 2>&1 || fail 'cmake not found. Install it with: brew install cmake'
if [ "$RUN_TESTS" -eq 1 ]; then
  command -v ctest >/dev/null 2>&1 || fail 'ctest not found. It is installed together with CMake.'
fi
# Ninja is preferred. Make is used if Ninja is not installed.
if command -v ninja >/dev/null 2>&1; then
  GENERATOR="Ninja"
elif command -v make >/dev/null 2>&1; then
  GENERATOR="Unix Makefiles"
else
  fail 'Neither ninja nor make was found. Install Ninja with: brew install ninja'
fi

# --- Compiler ---
# The MCTS part of the library uses std::jthread from C++20. Not every clang on
# macOS has it switched on. Each candidate has to build and run this program.
CHECK_DIR="$(mktemp -d)"
trap 'rm -rf "$CHECK_DIR"' EXIT
cat > "$CHECK_DIR/check.cpp" <<'CHECK'
#include <thread>
int main() { std::jthread t([] {}); return 0; }
CHECK

CXX_BIN=""
EXTRA_CXX_FLAGS=""
EXTRA_LINK_FLAGS=""

# try_compiler <compiler> <compile flags> <link flags>
# Sets CXX_BIN and the two flag variables if the test program builds and runs.
try_compiler() {
  [ -n "$CXX_BIN" ] && return 0
  command -v "$1" >/dev/null 2>&1 || return 0
  # The flags are split into words on purpose, so they are not quoted here.
  # shellcheck disable=SC2086
  if "$1" -std=c++20 $2 "$CHECK_DIR/check.cpp" $3 -o "$CHECK_DIR/check" >/dev/null 2>&1 && "$CHECK_DIR/check"; then
    CXX_BIN="$1"
    EXTRA_CXX_FLAGS="$2"
    EXTRA_LINK_FLAGS="$3"
  fi
  return 0
}

if [ -n "${CXX:-}" ]; then
  # The user chose a compiler. Only this one is tried.
  try_compiler "$CXX" "" ""
  try_compiler "$CXX" "-fexperimental-library" ""
  [ -n "$CXX_BIN" ] || fail "The compiler \"$CXX\" cannot build a C++20 program with std::jthread."
else
  # 1. Apple's clang++ from the Xcode Command Line Tools.
  try_compiler clang++ "" ""
  try_compiler clang++ "-fexperimental-library" ""
  # 2. clang++ of Homebrew LLVM. It brings its own C++ library, which has to be found at link time and at run time.
  if [ -z "$CXX_BIN" ]; then
    LLVM_PREFIX="${LLVM_ROOT:-}"
    if [ -z "$LLVM_PREFIX" ] && command -v brew >/dev/null 2>&1; then
      LLVM_PREFIX="$(brew --prefix llvm 2>/dev/null || true)"
    fi
    if [ -z "$LLVM_PREFIX" ] || [ ! -x "$LLVM_PREFIX/bin/clang++" ]; then
      for candidate in /opt/homebrew/opt/llvm /usr/local/opt/llvm; do
        if [ -x "$candidate/bin/clang++" ]; then
          LLVM_PREFIX="$candidate"
          break
        fi
      done
    fi
    if [ -n "$LLVM_PREFIX" ] && [ -x "$LLVM_PREFIX/bin/clang++" ]; then
      LLVM_LINK="-L$LLVM_PREFIX/lib/c++ -Wl,-rpath,$LLVM_PREFIX/lib/c++"
      try_compiler "$LLVM_PREFIX/bin/clang++" "" ""
      try_compiler "$LLVM_PREFIX/bin/clang++" "" "$LLVM_LINK"
      try_compiler "$LLVM_PREFIX/bin/clang++" "-fexperimental-library" "$LLVM_LINK"
    fi
  fi
  if [ -z "$CXX_BIN" ]; then
    fail 'No compiler was found that can build a C++20 program with std::jthread. Install a current one with: brew install llvm'
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
# Flags that the compiler check found necessary. They apply to C++ only. raylib is
# written in C and Objective-C and is built with the default C compiler of macOS.
if [ -n "$EXTRA_CXX_FLAGS" ]; then
  CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="$EXTRA_CXX_FLAGS")
fi
if [ -n "$EXTRA_LINK_FLAGS" ]; then
  CMAKE_ARGS+=(-DCMAKE_EXE_LINKER_FLAGS="$EXTRA_LINK_FLAGS")
fi
if [ "$BUILD_DEMOS" -eq 1 ]; then
  CMAKE_ARGS+=(-DVGAI_BUILD_DEMOS=ON)
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
  command -v doxygen >/dev/null 2>&1 || fail '--docs was given, but doxygen was not found. Install it with: brew install doxygen'
  CMAKE_ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=OFF)
else
  CMAKE_ARGS+=(-DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=ON)
fi

printf 'Configuring %s build with %s and %s%s ...\n' "$VARIANT" "$GENERATOR" "$CXX_BIN" "${EXTRA_CXX_FLAGS:+ $EXTRA_CXX_FLAGS}"
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
