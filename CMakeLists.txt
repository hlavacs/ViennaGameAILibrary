cmake_minimum_required (VERSION 3.28.0)

project (
  ViennaGameAILibrary
  VERSION 1.0
  DESCRIPTION "A library containing game AI algorithms."
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_COLOR_DIAGNOSTICS ON)

include_directories("include")
add_subdirectory(Demo)

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
