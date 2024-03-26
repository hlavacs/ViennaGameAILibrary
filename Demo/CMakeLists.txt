add_subdirectory(raylib)

add_executable(Demo src/main.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
target_link_libraries(Demo PRIVATE raylib)
# target_compile_definitions(Demo PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/")
