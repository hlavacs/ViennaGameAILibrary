# ViennaGameAILibrary
A library containing game AI algorithms.

## Project structure
- *Demo*
	- [raylib](https://github.com/raysan5/raylib)
	- *src* - Source folder containing the code for the demos
	- *res* - Resource folder containing assets
	- *docs* - Code documentation
	- ```CMakeLists.txt```
- *include* - ```ViennaGameAILibrary.hpp```
- ```build_demo_win.bat``` - Script to build the project on Windows
- ```run_demo_win.bat``` - Script to run the project on Windows
- ```CMakeLists.txt```
- ```README.md```

## Setup - Windows
### Prerequisites
- [Doxygen](https://www.doxygen.nl/index.html)
- [Ninja](https://ninja-build.org/)
- [Msys2](https://www.msys2.org/) - CMake and Clang need to be installed through msys2.
	- CMake (minimum version required: 3.28.1)
 	- Clang

### Build and run the project
1. Clone the project.
2. Run ```git submodule init``` and ```git submodule update``` to fetch raylib.
3. Update the location of ```clang++.exe``` in the ```build_demo_win.bat``` file based on the location of the msys2 folder.
4. Add *msys64/ucrt64/bin* and *doxygen/bin* to the path environment variable.
5. Run ```build_demo_win.bat``` to build the project. The output will be stored in the build folder.

Make sure Clang and Ninja are used by this project. Once the build process starts, the first two lines should be the following:
```
-- Building for: Ninja
-- The CXX compiler identification is Clang 17.0.6 // or any other version
```
6. Run ```run_demo_win.bat``` to start the project.

### To run different demos
In *Demo/```CMakeLists.txt```*, change the path of the .cpp file that you want to run.
```
add_executable(Demo src/demo_PathFinding.cpp ${PROJECT_SOURCE_DIR}/include/ViennaGameAILibrary.hpp)
```

## Code documentation
Documentation is generated using Doxygen. To see it, open ```index.html``` which can be found in the *Demo/docs/html* folder.

## How to use each feature the library provides
> **Note** : When implementing any features, keep in mind that you will need to transform the library coordinates into screen coordinates. The demos contain examples of how to do this.
1. **Path finding**

Demo example: *Demo/src/demo_Pathfinding.cpp*

- Create a navigation mesh
```
	// Generate it randomly with a given percentage for spawning obstacles
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 30.0f);

	// Or load it from a file
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh("Demo/res/navmesh.txt");
```
- Create a start and and end goal
``` 
	VGAIL::Vec2ui startPosition = VGAIL::Vec2ui(1, 1);
	VGAIL::Vec2ui endPosition = VGAIL::Vec2ui(15, 15);
```
- Optional: start geometric preprocessing
```
	navmesh->startPreprocess();
```
- Use A* algorithm to find the most optimal path
```
	// Without geometric preprocessing
	std::vector<VGAIL::Vec2ui> path = navmesh->A_Star(startPosition, endPosition);

	// Using geometric preprocessing and multithreading
	std::vector<VGAIL::Vec2ui> path = navmesh->getPath_multithreading(startPosition, endPosition);
```

2. **Decision trees**

Demo example: *Demo/src/demo_DecisionTree.cpp*

- Create the decision tree
```
	VGAIL::DecisionTree tree;
```

- For each node of the tree (and recursively its children) a custom class needs to be implemented to delegate actions to its children based on some criteria.
```
	// Example: a character makes decisions based on the distance to an enemy
	class isEnemyClose : public VGAIL::DecisionNode
	{
	public:
		isEnemyClose(VGAIL::Vec2f& enemyPos)
			: enemyPos(enemyPos) {}

		~isEnemyClose() {}

		void makeDecision(float dt) override
		{
			float dist = VGAIL::distance(position, enemyPos);
			if (dist <= 100.0f)
			{
				getChild(0).makeDecision(dt);
			}
			else if (dist > 100.0f && dist < 150.0f)
			{
				getChild(1).makeDecision(dt);
			}
			else
			{
				getChild(2).makeDecision(dt);
			}
		}

		VGAIL::Vec2f position;
		VGAIL::Vec2f& enemyPos;
	};
```
- Create the root of the tree
```
	// Example for the code shown above
	VGAIL::DecisionNode& root = tree.createRoot<isEnemyClose>(args);
```
- Add nodes and their children
```
	VGAIL::DecisionNode& node1 = root.addChild<Class1>(args);
	VGAIL::DecisionNode& node2 = root.addChild<Class2>(args);
	...
	VGAIL::DecisionNode& node1_child1 = node1.addChild<ClassX>(args);
	...
```
- In the game loop, update the decision tree
```
	tree.update(deltaTime);
```
3. **State machines**

- Create a state machine
```
	VGAIL::StateMachine stateMachine;
```
- Create states
```
	VGAIL::State* walking = stateMachine.createState();
	VGAIL::State* sleeping = stateMachine.createState();
```
- Create transitions between states
```
	walking->addTransition(sleeping, [&]() {
		return health <= 2;
	});
```
- Implement the state in the update callback
```
	walking->onUpdateCallback = [&](float deltaTime) {
		// do something
	};
```
- If needed, add action on the start and/or end of a state
```
	sleeping->onEnterCallback = [&]() {
		// play sleeping animation
	};
```
- In the game loop, update the state machine
```
	stateMachine.update(deltaTime);
```

4. **Steering behaviors**

The steering behaviors can only be used on ```VGAIL::Boids```.

- Create the boid
```
	VGAIL::Boid* agent = new VGAIL::Boid(position, velocity, maxSpeed);
```
- Calculate the steering force
	- Arrive
	```
	VGAIL::Vec2f steeringForce = agent->arrive(targetPosition, slowRadius, maxAcceleration);
	```
	- Evade
	```
	VGAIL::Vec2f steeringForce = agent->evade(targetBoid, maxAcceleration, maxPrediction);
	```
	- Flee
	```
	VGAIL::Vec2f steeringForce = agent->flee(targetPosition, maxAcceleration);
	```
	- Pursue
	```
	VGAIL::Vec2f steeringForce = agent->pursue(targetBoid, maxAcceleration, maxPrediction);
	```
	- Seek
	```
	VGAIL::Vec2f steeringForce = agent->seek(targetPosition, maxAcceleration);
	```
	- Wander
	```
	VGAIL::Vec2f steeringForce = agent->wander(circleDistance, circleRadius, displaceRange, maxAcceleration);
	```
- Apply the steering force and update the position (can be called from inside and outside of the game loop)
```
	agent->applySteeringForce(steeringForce);
	agent->updatePosition(deltaTime);
```

> The ```getRotationInDegrees()``` method from the Boid class calculates the rotation of the boid in degrees and can be used to steer the boid towards the direction it is moving.

5. **Flocking**

The flocking behavior only works on ```VGAIL::Boid``` instances.

- Create the flock
```
	VGAIL::Flocking* flock = new VGAIL::Flocking();
```
- Set the separation and perception ranges
```
	flock->setRanges(separationRange, perceptionRange);
```
- Add boids to the flock
```
	flock->addBoid(position, velocity, minSpeed, maxSpeed);
```
- In the game loop, update the flock
```
	flock->update(deltaTime, avoidFactor, matchingFactor, centeringFactor);
```

For more information on the arguments taken by each method, please check the code documentation.