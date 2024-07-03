add_subdirectory(raylib)

# add_executable(Demo src/demo_PathFinding.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_StateMachine.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_DecisionTree.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
add_executable(Demo src/SteeringBehaviors/demo_Flocking.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_SeekAndFlee.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_PursueAndEvade.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_Arrive.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_Wander.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_SteeringBehaviors.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)

target_link_libraries(Demo PRIVATE raylib)
# target_compile_definitions(Demo PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res/") 
