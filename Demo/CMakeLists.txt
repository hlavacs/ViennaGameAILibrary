add_subdirectory(raylib)

### Demo files
add_executable(Demo src/demo_Pathfinding.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_StateMachine.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_DecisionTree.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_Flocking.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_SeekAndFlee.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_PursueAndEvade.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_Arrive.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/SteeringBehaviors/demo_Wander.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo src/demo_SteeringBehaviors.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)

### Test files
# add_executable(Demo testing/test_Pathfinding.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo testing/test_SeekAndFlee.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo testing/test_PursueAndEvade.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo testing/test_Arrive.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo testing/test_WanderAndFace.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
# add_executable(Demo testing/test_Flocking.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)

target_link_libraries(Demo PRIVATE raylib)
