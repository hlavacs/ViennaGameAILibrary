cmake_minimum_required (VERSION 3.28.0)

project (
  ViennaGameAILibrary
  VERSION 1.0
  DESCRIPTION "A C++ library containing game AI algorithms."
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_COLOR_DIAGNOSTICS ON)

include_directories("include")
option(VGAI_BUILD_DEMOS "Build the graphical demonstrations" ON)
if(VGAI_BUILD_DEMOS)
  add_subdirectory(Demo)
endif()
include(CTest)
if(BUILD_TESTING)
  add_executable(vgai_path_regression tests/path_regression.cpp)
  target_include_directories(vgai_path_regression PRIVATE include)
  add_test(NAME vgai_path_regression COMMAND vgai_path_regression)
  add_executable(vgai_mcts_regression tests/mcts_regression.cpp)
  target_include_directories(vgai_mcts_regression PRIVATE include Demo/src)
  add_test(NAME vgai_mcts_regression COMMAND vgai_mcts_regression)
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started.")

  add_custom_target(doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
    COMMENT "Generating API documentation with Doxygen..."
    VERBATIM)
else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the Doxygen documentation!")
endif()
