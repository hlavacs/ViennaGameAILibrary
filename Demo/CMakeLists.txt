add_subdirectory(raylib)

# Every demo is its own program. Each one is built from a single source file.
# Start a demo from the repository root, because the demos load their images
# and fonts as Demo/res/... relative to that folder.
#
# One demo per main theme of the library:
#   demo_Pathfinding         A* and preprocessed paths on a navigation mesh
#   demo_DecisionTree        a guard and a burglar that decide with decision trees
#   demo_StateMachine        a worker controlled by a state machine
#   demo_SteeringBehaviors   chickens, a dog and a snake that use all steering behaviours
#   demo_Flocking            a flock of boids with adjustable factors
#   demo_MCTS                Connect Four against Monte Carlo Tree Search
#
# One demo per single steering behaviour:
#   demo_SeekAndFlee, demo_PursueAndEvade, demo_Arrive, demo_Wander
set(VGAI_DEMOS
  src/demo_Pathfinding.cpp
  src/demo_DecisionTree.cpp
  src/demo_StateMachine.cpp
  src/demo_SteeringBehaviors.cpp
  src/SteeringBehaviors/demo_Flocking.cpp
  src/demo_MCTS.cpp
  src/SteeringBehaviors/demo_SeekAndFlee.cpp
  src/SteeringBehaviors/demo_PursueAndEvade.cpp
  src/SteeringBehaviors/demo_Arrive.cpp
  src/SteeringBehaviors/demo_Wander.cpp
)

foreach(demo_source IN LISTS VGAI_DEMOS)
  # The program is named like its source file, for example demo_Flocking.
  get_filename_component(demo_name ${demo_source} NAME_WE)
  add_executable(${demo_name} ${demo_source} ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
  target_include_directories(${demo_name} PRIVATE src)
  target_link_libraries(${demo_name} PRIVATE raylib)
endforeach()

### Test files
# These programs measure the library and print numbers. They are not built by
# default. To build one, remove the # in front of its line.
# add_executable(test_Pathfinding testing/test_Pathfinding.cpp)
# target_link_libraries(test_Pathfinding PRIVATE raylib)
# add_executable(test_SeekAndFlee testing/test_SeekAndFlee.cpp)
# target_link_libraries(test_SeekAndFlee PRIVATE raylib)
# add_executable(test_PursueAndEvade testing/test_PursueAndEvade.cpp)
# target_link_libraries(test_PursueAndEvade PRIVATE raylib)
# add_executable(test_Arrive testing/test_Arrive.cpp)
# target_link_libraries(test_Arrive PRIVATE raylib)
# add_executable(test_WanderAndFace testing/test_WanderAndFace.cpp)
# target_link_libraries(test_WanderAndFace PRIVATE raylib)
# add_executable(test_Flocking testing/test_Flocking.cpp)
# target_link_libraries(test_Flocking PRIVATE raylib)
# add_executable(test_MCTSTicTacToe testing/test_MCTSTicTacToe.cpp)
# add_executable(test_MCTSConnectFour testing/test_MCTSConnectFour.cpp)
# add_executable(test_MCTSHex testing/test_MCTSHex.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Demo)
