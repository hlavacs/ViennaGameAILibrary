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

1. **Data structures and data types**

*Vienna Game AI Library* uses two custom made vectors: ```Vec2ui``` and ```Vec2f```. Both represent 2D vectors, with ```Vec2ui``` made of unsigned integers, and ```Vec2f``` of floats.
The navigation mesh uses ```Vec2ui```, while Boids use ```Vec2f``` due to the different calculations that are needed. Other data structures used in the library are ```std::vector``` and ```std::unordered_map```.

The data types used in the library are: ```uint32_t```, ```uint64_t```, ```int32_t``` and ```float```. Each has a typedef declaration to make the code more readable. (```uint32_t``` -> ```ui32```, ```uint64_t``` -> ```ui64```, ```int32_t``` -> ```i32```, ```float``` -> ```f32```).

There are custom structs also defined in the library. The ```NavMesh``` class is represented by a ```std::vector``` of ```NodeData``` objects. If geometric preprocessing is used for path finding, the ```Region``` struct is also used to store the nodes inside each region, and  ```RegionList``` to manage all regions.

2. **Path finding**

Demo example: *Demo/src/demo_Pathfinding.cpp*

- Create a navigation mesh

To generate it randomly, pass the desired width and height, as well as a percentage which will limit the amount of obstacles spawned within the navigation mesh. To load it from a file, simply call the constructor with the file path as the argument.
```
	// Generate it randomly
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 30.0f);

	// Or load it from a file
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh("Demo/res/navmesh.txt");
```
The navmesh file can be created either manually or by the game itself. The file has the following structure:
```
	navmeshWidth
	navmeshHeight
	owwwwoowwowww...
```
```navmeshWidth``` and ```navmeshHeight``` need to be positive integers. The third line describes the pattern of the navigation mesh, where ```w``` is a walkable area and ```o``` is an obstacle. Make sure that the number of characters are equal to ```navmeshWidth``` * ```navmeshHeight```.

There is also the option to save a randomly generated navmesh. This can be done by calling ```saveToFile(const std::string& filepath)``` (*ViennaGameAILibrary* : 453-466) and by passing a filepath. If the file doesn't exist, it will be created automatically.

- Create start and end positions for the A* algorithm

The start and end positions need to be ```Vec2ui```. Inside the application, they can be converted to screen coordinates by multiplying to a number of your choice. (see the demo for pathfinding for more details, specifically calculations that use ```gridStride```)
``` 
	VGAIL::Vec2ui startPosition = VGAIL::Vec2ui(1, 1);
	VGAIL::Vec2ui endPosition = VGAIL::Vec2ui(15, 15);
```
- Optional: start geometric preprocessing
 
 While A* ensures finding the shortest path, it sometimes can be quite slow. In video games, it is preferred that the path is retrieved as fast as possible, so if that means that the path returned is not the shortest one, it will most likely not have a big impact on the game. A path that is found fast but is not necessarily the shortest from all options, is called an optimal path. Geometric preprocessing ensures finding an optimal path, but not necessarily the shortest one.

This process can be called while setting up the application (so before the game loop) in order to perform all calculations before the application starts. It can be done in two ways: with single or multiple threads. The call for this process is as follows:
```
	void preprocess(bool multithreading = false, ui32 numThreads = 4)
```
The boolean specifies whether to use multithreading (default: false), and ```numThreads``` is the number of threads needed to run in parallel (default: 4).

This process will work on the Regions defined when the navmesh is created ( see *ViennaGameAILibrary* : 514-537). The number of regions depends on the navmesh size, and by default they are set to each contain 9 * 9 nodes (9 on the *x* axis, 9 on the *y* axis). Depending on the navmesh size, this can be changed accordingly to maximize performance. If multithreading is used, each thread receives ```totalNumberOfRegions / numThreads``` regions.

The following picture shows how the regions would look like on top of the demo for path finding. Each orange square represents a region.

![Displaying regions on navmesh](/assets/pathfinding.jpg)

During this process, the ```AStar``` method is called to calculate the distance between each nodes and each region. Therefore, at the end of the process, the ```m_adjList``` from the ```NavMesh``` class will contain the best path from one node to a region at ```m_adjList[nodeIndex][regionID]```. 

- Use A* algorithm to find the most optimal path
```
	// Without geometric preprocessing
	std::vector<VGAIL::Vec2ui> path = navmesh->findPath(startPosition, endPosition);

	// Using geometric preprocessing and/or multithreading
	std::vector<VGAIL::Vec2ui> path = navmesh->findPreprocessedPath(startPosition, endPosition);
```
```findPath()``` calculates the path by using A*, while ```findPreprocessedPath()``` retrieves the stored path if geometric preprocessing has been done. 

3. **Decision trees**

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
4. **State machines**

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

5. **Steering behaviors**

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

6. **Flocking**

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
