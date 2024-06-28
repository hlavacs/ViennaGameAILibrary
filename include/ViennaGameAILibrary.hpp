#pragma once

#include <iostream>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

namespace VGAIL
{
	constexpr float PI = 3.14159265358979f;

	typedef uint32_t ui32;
	typedef uint64_t ui64;
	typedef int32_t i32;
	typedef float f32;

#ifdef NDEBUG
#define VGAIL_ASSERT(x, msg) { }
#else
#define VGAIL_ASSERT(x, msg) { if(!(x)) { std::cout << "Assertion Failed: " << msg << std::endl; __debugbreak(); } }
#endif

	struct Vec2ui
	{
		ui32 x, y;

		Vec2ui()
			: x(0)
			, y(0)
		{}

		Vec2ui(ui32 x, ui32 y)
			:x(x), y(y) {}

		Vec2ui(ui32 val)
			: x(val), y(val) {}

		bool operator==(const Vec2ui& other)
		{
			return x == other.x && y == other.y;
		}
	};

	std::ostream& operator<<(std::ostream& os, const Vec2ui& vec)
	{
		return os << vec.x << ", " << vec.y;
	}

	struct Vec2f
	{
		f32 x, y;

		Vec2f()
			: x(0.0f)
			, y(0.0f)
		{}

		Vec2f(f32 x, f32 y)
			: x(x), y(y) {}

		Vec2f(f32 val)
			: x(val), y(val) {}

		Vec2f operator-(const Vec2f& other)
		{
			return Vec2f{ x - other.x, y - other.y };
		}

		Vec2f operator+(const Vec2f& other)
		{
			return Vec2f{ x + other.x, y + other.y };
		}

		Vec2f operator/(const Vec2f& other)
		{
			return Vec2f{ x / other.x, y / other.y };
		}

		Vec2f operator*(f32 other)
		{
			return Vec2f{ x * other, y * other };
		}

		Vec2f operator/(f32 other)
		{
			return Vec2f{ x / other, y / other };
		}

		bool operator==(const Vec2f& other)
		{
			return x == other.x && y == other.y;
		}

		f32 getMagnitude()
		{
			return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
		}

		void setMagnitude(f32 value)
		{
			f32 mag = getMagnitude();
			if (mag <= 0.0f)
			{
				std::cout << "mag <= 0" << std::endl;
				return;
			}

			f32 scaleFactor = value / mag;
			x *= scaleFactor;
			y *= scaleFactor;
		}

		void normalize()
		{
			f32 mag = getMagnitude();
			if (mag < 0.0f)
			{
				std::cout << "mag < 0" << std::endl;
				return;
			}
			x /= mag;
			y /= mag;
		}
	};

	std::ostream& operator<<(std::ostream& os, const Vec2f& vec)
	{
		return os << vec.x << ", " << vec.y;
	}

	f32 randomFloat(f32 min, f32 max)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<f32> distribution(min, max);
		return distribution(rng);
	}

	f32 randomInt(i32 min, i32 max)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> distribution(min, max);
		return distribution(rng);
	}

	f32 distance(const Vec2f& v1, const Vec2f& v2)
	{
		f32 distX = v1.x - v2.x;
		f32 distY = v1.y - v2.y;
		f32 dist = std::pow(distX, 2) + std::pow(distY, 2);

		return std::sqrt(dist);
	}

	struct Region
	{
		ui32 regionID;
		std::vector<ui32> nodes;
	};

	struct RegionList
	{
		std::vector<Region*> regions;
		ui32 sizeX, sizeY;

		RegionList(ui32 sizeX, ui32 sizeY)
			: sizeX(sizeX)
			, sizeY(sizeY)
		{
			for (ui32 y = 0; y < sizeY; y++)
			{
				for (ui32 x = 0; x < sizeX; x++)
				{
					Region* region = new Region();
					region->regionID = x + y * sizeX;
					regions.push_back(region);
				}
			}
		}

		~RegionList()
		{
			for (ui32 i = 0; i < regions.size(); i++)
			{
				delete regions[i];
			}
		}

		ui32 getRegionID(Vec2ui pos)
		{
			return pos.x + pos.y * sizeX;
		}
	};

	enum NodeState
	{
		OBSTRUCTABLE,
		WALKABLE
	};

	struct NodeData
	{
		Vec2ui pos;
		f32 g, h;
		ui32 regionID;
		NodeState state;

		f32 f() const
		{
			return g + h;
		}

		NodeData(Vec2ui pos)
			: pos(pos)
			, g(INFINITY)
			, h(INFINITY)
			, state(NodeState::WALKABLE)
		{}

		bool operator==(const NodeData& other)
		{
			return pos == other.pos && g == other.g && h == other.h && regionID == other.regionID && state == other.state;
		}
	};

	struct NodeDataComparator
	{
		bool operator()(const NodeData& node1, const NodeData& node2) const {
			return node1.f() > node2.f();
		}
	};

	class NavMesh
	{
	public:
		NavMesh(ui32 width, ui32 height, f32 obstaclePercentage, f32 regionLengthOnX = 9.0f, f32 regionLengthOnY = 9.0f)
			: m_width(width)
			, m_height(height)
		{
			for (ui32 y = 0; y < m_height; y++)
			{
				for (ui32 x = 0; x < m_width; x++)
				{
					NodeData node(Vec2ui{ x, y });

					auto val = randomFloat(0.0f, 100.0f);
					if (obstaclePercentage != 0.0f && val <= obstaclePercentage)
					{
						node.state = NodeState::OBSTRUCTABLE;
					}

					m_nodes.emplace_back(node);
				}
			}

			m_adjList.resize(m_nodes.size(), {});
			m_neighbors.resize(m_nodes.size(), {});

			for (ui32 i = 0; i < m_nodes.size(); i++)
			{
				setNeighbors(i);
			}

			createRegions(regionLengthOnX, regionLengthOnY);
		}

		NavMesh(const std::string& filepath, f32 regionLengthOnX = 9.0f, f32 regionLengthOnY = 9.0f)
		{
			std::ifstream stream(filepath);
			std::string navmeshData;

			if (stream.is_open())
			{
				std::string line;
				getline(stream, line);
				m_width = std::stoi(line);

				getline(stream, line);
				m_height = std::stoi(line);

				stream >> navmeshData;
			}
			else
			{
				std::cout << "Could not read file " << filepath << std::endl;
				return;
			}

			for (ui32 y = 0; y < m_height; y++)
			{
				for (ui32 x = 0; x < m_width; x++)
				{
					NodeData node(Vec2ui{ x, y });

					char state = navmeshData[getIndex(Vec2ui{ x, y })];
					if (state == 'w')
					{
						node.state = NodeState::WALKABLE;
					}
					else
					{
						node.state = NodeState::OBSTRUCTABLE;
					}

					m_nodes.emplace_back(node);
				}
			}

			m_adjList.resize(m_nodes.size(), {});
			m_neighbors.resize(m_nodes.size(), {});

			for (ui32 i = 0; i < m_nodes.size(); i++)
			{
				setNeighbors(i);
			}

			createRegions(regionLengthOnX, regionLengthOnY);
		}

		~NavMesh() {}

		void preprocess(bool multithreading = false, ui32 numThreads = 4 )
		{
			m_isPreprocessed = true;
			if(multithreading)
			{
				std::vector<std::thread> threads;

				ui32 regionsPerThread = m_regions->regions.size() / numThreads;

				for (ui32 i = 0; i < numThreads; i++)
				{
					threads.push_back(std::thread(&NavMesh::preprocessWorker, this, i, numThreads, m_nodes, m_regions->regions));
				}

				for (ui32 i = 0; i < threads.size(); i++)
				{
					threads[i].join();
				}
			}
			else
			{
				for (ui32 i = 0; i < m_nodes.size(); i++)
				{
					std::cout << "Processing node " << i << "/" << m_nodes.size() - 1 << std::endl;

					if (m_nodes[i].state == NodeState::OBSTRUCTABLE)
						continue;

					for (Region* R : m_regions->regions)
					{
						if (m_nodes[i].regionID == R->regionID)
							continue;

						std::vector<std::vector<Vec2ui>> paths;

						// Calculate distances from N to all nodes in region R
						for (ui32 O_tentative : R->nodes)
						{
							std::vector<Vec2ui> path = findPath(m_nodes[i].pos, m_nodes[O_tentative].pos);

							if (path.size() > 0)
							{
								paths.push_back(path);
							}
						}

						// No paths found
						if (paths.size() == 0)
						{
							continue;
						}

						// Find most optimal path from N to R
						ui32 bestPathIndex = 0;
						for (ui32 j = 1; j < paths.size(); j++)
						{
							if (paths[j].size() < paths[bestPathIndex].size())
							{
								bestPathIndex = j;
							}
						}

						// Add best path to R to the node N
						m_adjList[i][R->regionID] = paths[bestPathIndex];
					}
				}
			}
		}
		
		std::vector<Vec2ui> findPath(Vec2ui startPos, Vec2ui endPos)
		{
			return AStar(startPos, endPos, m_nodes);
		}

		std::vector<Vec2ui> findPreprocessedPath(Vec2ui start, Vec2ui target)
		{
			NodeData& startNode = getNode(start);
			NodeData& targetNode = getNode(target);

			ui32 startRegionID = startNode.regionID;
			ui32 targetRegionID = targetNode.regionID;

			if (startRegionID == targetRegionID)
			{
				return findPath(start, target);
			}

			// Get precomputed path from start node to target region
			if (m_adjList[getIndex(startNode.pos)].size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}

			ui32 startNodeIndex = getIndex(startNode.pos);
			std::vector<Vec2ui> pathToRegion = m_adjList[startNodeIndex][targetRegionID];

			if (pathToRegion.size() == 0)
			{
				std::cout << "No path found to region!" << std::endl;
				return {};
			}

			// Calculate shortest path between node O and target node
			Vec2ui regionStartNode = pathToRegion[pathToRegion.size() - 1];
			if (regionStartNode == target)
			{
				// Reached destination
				return pathToRegion;
			}

			std::vector<Vec2ui> pathWithinRegion = findPath(regionStartNode, target);
			if (pathWithinRegion.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			pathToRegion.insert(pathToRegion.end(), pathWithinRegion.begin(), pathWithinRegion.end());

			return pathToRegion;			
		} 

		void saveToFile(const std::string& filepath)
		{
			std::ofstream stream(filepath);
			stream << m_width << '\n';
			stream << m_height << '\n';
			for (NodeData node : m_nodes)
			{
				if (node.state == NodeState::WALKABLE)
					stream << "w";
				else
					stream << "o";
			}
			stream.close();
		}

		void setObstructable(Vec2ui obstaclePos)
		{
			ui32 index = getIndex(obstaclePos);
			if (m_nodes[index].state != NodeState::OBSTRUCTABLE)
			{
				m_nodes[index].state = NodeState::OBSTRUCTABLE;
			}
		}

		void setWalkable(Vec2ui pos)
		{
			ui32 index = getIndex(pos);
			if (m_nodes[index].state != NodeState::WALKABLE)
			{
				m_nodes[index].state = NodeState::WALKABLE;
			}
		}

		ui32 getWidth()
		{
			return m_width;
		}

		ui32 getHeight()
		{
			return m_height;
		}

		Vec2ui get2DCoordinates(ui32 index)
		{
			ui32 x = index % m_width;
			ui32 y = (index - x) / m_width;
			return Vec2ui(x, y);
		}

		ui32 getIndex(Vec2ui pos)
		{
			return pos.x + pos.y * m_width;
		}

		NodeData& getNode(Vec2ui pos)
		{
			return m_nodes[getIndex(pos)];
		}

	private:
		void createRegions(f32 regionLengthOnX, f32 regionLengthOnY)
		{
			ui32 numRegionsX = std::ceil(static_cast<f32>(m_width) / regionLengthOnX);
			ui32 numRegionsY = std::ceil(static_cast<f32>(m_height) / regionLengthOnY);

			m_regions = new RegionList(numRegionsX, numRegionsY);

			for (ui32 y = 0; y < m_height; y++)
			{
				ui32 yIndex = std::floor(static_cast<f32>(y) / regionLengthOnY);

				for (ui32 x = 0; x < m_width; x++)
				{
					ui32 xIndex = std::floor(static_cast<f32>(x) / regionLengthOnX);

					ui32 regionID = m_regions->getRegionID(Vec2ui(xIndex, yIndex));

					NodeData& node = getNode(Vec2ui(x, y));
					node.regionID = regionID;

					m_regions->regions[regionID]->nodes.push_back(getIndex(node.pos));
				}
			}
		}

		void setNeighbors(ui32 nodeIndex)
		{
			NodeData& node = m_nodes[nodeIndex];
			f32 x = node.pos.x;
			f32 y = node.pos.y;

			for (i32 v = -1; v <= 1; ++v)
			{
				for (i32 u = -1; u <= 1; ++u)
				{
					if (u == 0 && v == 0) // Current node
						continue;

					if (x + u >= 0 && y + v >= 0 &&
						x + u < m_width && y + v < m_height)
					{
						m_neighbors[nodeIndex].push_back(getIndex(Vec2ui(x + u, y + v)));
					}
				}
			}
		}

		f32 manhattan(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 distX = std::abs(static_cast<f32>(v1.x) - static_cast<f32>(v2.x));
			f32 distY = std::abs(static_cast<f32>(v1.y) - static_cast<f32>(v2.y));

			return distX + distY;
		}

		f32 euclidean(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 distX = static_cast<f32>(v1.x) - static_cast<f32>(v2.x);
			f32 distY = static_cast<f32>(v1.y) - static_cast<f32>(v2.y);
			f32 dist = std::pow(distX, 2) + std::pow(distY, 2);

			return std::sqrt(dist);
		}

		void preprocessWorker(ui32 threadID, ui32 numThreads, std::vector<NodeData> nodes, std::vector<Region*> regionList)
		{
			ui32 startIndex = threadID;

			for (ui32 regionIndex = startIndex; regionIndex < regionList.size(); regionIndex += numThreads)
			{
				std::cout << "Preprocessing region " << regionIndex << "/" << regionList.size() << std::endl;
				for (ui32 i = 0; i < regionList[regionIndex]->nodes.size(); i++)
				{
					ui32 nodeIndex = regionList[regionIndex]->nodes[i];

					if (nodes[nodeIndex].state == NodeState::OBSTRUCTABLE)
						continue;

					for (Region* R : regionList)
					{
						if (nodes[nodeIndex].regionID == R->regionID)
							continue;

						std::vector<std::vector<Vec2ui>> paths;

						// Calculate distances from N to all nodes in region R
						for (ui32 O_tentative : R->nodes)
						{
							std::vector<NodeData> nodes = m_nodes;
							std::vector<Vec2ui> path = AStar(nodes[nodeIndex].pos, nodes[O_tentative].pos, nodes);

							if (path.size() > 0)
							{
								paths.push_back(path);
							}
						}

						// No paths found
						if (paths.size() == 0)
						{
							continue;
						}

						// Find most optimal path from N to R
						ui32 bestPathIndex = 0;
						for (ui32 j = 1; j < paths.size(); j++)
						{
							if (paths[j].size() < paths[bestPathIndex].size())
							{
								bestPathIndex = j;
							}
						}

						// Add best path to R to the node N
						m_adjList[nodeIndex][R->regionID] = paths[bestPathIndex];
					}
				}
			}
		}

		std::vector<Vec2ui> AStar(Vec2ui startPos, Vec2ui endPos, std::vector<NodeData>& nodes)
		{
			std::vector<i32> parents(nodes.size(), -1);

			for (NodeData& node : nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::priority_queue<NodeData, std::vector<NodeData>, NodeDataComparator> openSet;

			ui32 startNodeIndex = getIndex(startPos);
			ui32 endNodeIndex = getIndex(endPos);

			nodes[startNodeIndex].g = 0.0f;

			openSet.push(nodes[startNodeIndex]);

			while (openSet.size() > 0)
			{
				ui32 currentIndex = getIndex(openSet.top().pos);

				while (openSet.size() > 0 && currentIndex == getIndex(openSet.top().pos))
				{
					openSet.pop();
				}

				if (currentIndex == endNodeIndex)
				{
					std::vector<Vec2ui> shortestPath;

					while (currentIndex != -1)
					{
						shortestPath.push_back(nodes[currentIndex].pos);
						currentIndex = parents[currentIndex];
					}

					std::reverse(shortestPath.begin(), shortestPath.end());
					return shortestPath;
				}

				std::vector<ui32> neighbors = m_neighbors[currentIndex];
				for (ui32 neighborIndex : neighbors)
				{
					NodeData& neighbor = nodes[neighborIndex];
					if (neighbor.state == NodeState::OBSTRUCTABLE)
						continue;

					f32 tentativeG = nodes[currentIndex].g + euclidean(neighbor.pos, nodes[currentIndex].pos);

					if (tentativeG < neighbor.g)
					{
						parents[neighborIndex] = getIndex(nodes[currentIndex].pos);
						neighbor.g = tentativeG;
						neighbor.h = euclidean(neighbor.pos, nodes[endNodeIndex].pos);

						openSet.push(nodes[neighborIndex]);
					}
				}
			}

			return {};
		}

	private:
		bool m_isPreprocessed = false;
		ui32 m_width, m_height;
		RegionList* m_regions;
		std::vector<NodeData> m_nodes;
		std::vector<std::vector<ui32>> m_neighbors;
		std::vector<std::unordered_map<ui32, std::vector<Vec2ui>>> m_adjList;
	};

	class State;
	class Transition
	{
	public:
		Transition(State* target, std::function<bool()> callback)
		{
			targetState = target;
			onCheckCallback = callback;
		}

		~Transition() = default;

		std::function<bool()> onCheckCallback;
		State* targetState = nullptr;
	};

	class State
	{
	public:
		State() {}

		~State() {
			for (Transition* transition : m_transitions)
			{
				delete transition;
			}
		}

		std::function<void()> onEnterCallback;
		std::function<void()> onExitCallback;
		std::function<void(float)> onUpdateCallback;

		void addTransition(State* targetState, std::function<bool()> callback)
		{
			m_transitions.push_back(new Transition(targetState, callback));
		}

		std::vector<Transition*> getTransitions()
		{
			return m_transitions;
		}

	private:
		std::vector<Transition*> m_transitions;
	};

	class StateMachine
	{
	public:
		StateMachine() {}

		~StateMachine()
		{
			for (State* state : m_states)
			{
				delete state;
			}
		}

		State* createState()
		{
			State* state = new State();
			m_states.push_back(state);
			return state;
		}

		void update(float delta)
		{
			if (m_states.size() <= 0)
				return;

			if (!m_currentState)
			{
				m_currentState = m_states[0];

				if (m_currentState->onEnterCallback)
				{
					m_currentState->onEnterCallback();
				}
			}

			Transition* activeTransition = nullptr;
			for (Transition* transition : m_currentState->getTransitions())
			{
				if (transition->onCheckCallback && transition->onCheckCallback())
				{
					activeTransition = transition;
					break;
				}
			}

			if (activeTransition)
			{
				if (m_currentState->onExitCallback)
				{
					m_currentState->onExitCallback();
				}
				m_currentState = activeTransition->targetState;

				if (m_currentState->onEnterCallback)
				{
					m_currentState->onEnterCallback();
				}
			}

			if (m_currentState->onUpdateCallback)
			{
				m_currentState->onUpdateCallback(delta);
			}
		}

		State* getCurrentState()
		{
			return m_currentState;
		}

	private:
		std::vector<State*> m_states;
		State* m_currentState = nullptr;
	};

	class DecisionNode
	{
	public:
		virtual ~DecisionNode()
		{
			for (DecisionNode* node : m_children)
			{
				delete node;
			}
		}

		virtual void makeDecision(float dt) = 0;

		DecisionNode& getChild(VGAIL::ui32 index)
		{
			VGAIL_ASSERT(index < m_children.size(), "Index out of bounds!");
			return *m_children[index];
		}

		template <class T, typename... Args>
		DecisionNode& addChild(Args&& ...args)
		{
			DecisionNode* child = new T(args...);
			m_children.push_back(child);
			return *child;
		}

	public:
		ui32 getChildrenSize()
		{
			return m_children.size();
		}

	private:
		std::vector<DecisionNode*> m_children;
	};

	class DecisionTree
	{
	public:
		DecisionTree() {}

		~DecisionTree()
		{
			resetTree();
		}

		void update(float dt)
		{
			if (m_root)
			{
				m_root->makeDecision(dt);
			}
		}

		template <class T, typename... Args>
		DecisionNode& createRoot(Args&& ...args)
		{
			if (m_root)
			{
				resetTree();
			}

			m_root = new T(args...);
			return *m_root;
		}

	private:
		void resetTree()
		{
			if (m_root)
			{
				delete m_root;
			}
		}

	private:
		DecisionNode* m_root = nullptr;
	};

	class Boid
	{
	public:
		Boid(Vec2f position, Vec2f velocity, f32 maxSpeed, ui32 id = 0)
			: m_position(position)
			, m_velocity(velocity)
			, m_maxSpeed(maxSpeed)
			, m_id(id)
		{}

		~Boid() {}

		void setPosition(Vec2f position)
		{
			m_position = position;
		}

		Vec2f getPosition() const
		{
			return m_position;
		}

		void setVelocity(Vec2f velocity)
		{
			m_velocity = velocity;
		}

		Vec2f getVelocity() const
		{
			return m_velocity;
		}

		ui32 getID() const
		{
			return m_id;
		}

		f32 getRotationInDegrees()
		{
			if (m_velocity.getMagnitude() < 0.01f)
			{
				return m_lastRotation;
			}

			Vec2f target = m_position + m_velocity;
			f32 radians = std::atan2(target.y - m_position.y, target.x - m_position.x);
			m_lastRotation = radians * (180.0f / PI);
			return m_lastRotation;
		}

		void setMinSpeed(f32 minSpeed)
		{
			m_minSpeed = minSpeed;
		}

		void setMaxSpeed(f32 maxSpeed)
		{
			m_maxSpeed = maxSpeed;
		}

		void flocking(f32 dt, f32 separationRange, f32 perceptionRange, f32 avoidFactor, f32 matchingFactor, f32 centeringFactor, const std::vector<Boid*>& flock)
		{
			Vec2f separationVector, alignVector, cohesionVector;
			ui32 neighbors = 0;

			for (Boid* other : flock)
			{
				if (other->getID() != m_id)
				{
					f32 dist = distance(m_position, other->getPosition());

					if (dist < separationRange)
					{
						separationVector = separationVector + (m_position - other->getPosition());
					}
					else if (dist < perceptionRange)
					{
						alignVector = alignVector + other->getVelocity();
						cohesionVector = cohesionVector + other->getPosition();
						neighbors++;
					}
				}
			}

			if (neighbors > 0)
			{
				alignVector = alignVector / static_cast<float>(neighbors);
				cohesionVector = cohesionVector / static_cast<float>(neighbors);

				m_velocity = m_velocity + (alignVector - m_velocity) * matchingFactor + (cohesionVector - m_position) * centeringFactor;
			}

			m_velocity = m_velocity + (separationVector * avoidFactor);

			f32 speed = m_velocity.getMagnitude();

			if (speed < m_minSpeed)
			{
				m_velocity = (m_velocity / speed) * m_minSpeed;
			}

			if (speed > m_maxSpeed)
			{
				m_velocity = (m_velocity / speed) * m_maxSpeed;
			}

			m_position = m_position + m_velocity * dt;
		}

		Vec2f separation(f32 separationRange, f32 avoidFactor, const std::vector<Boid*>& flock)
		{
			Vec2f separationVector;

			for (Boid* other : flock)
			{
				f32 dist = distance(m_position, other->getPosition());

				if (other->getID() != m_id && dist < separationRange)
				{
					separationVector = m_position - other->getPosition();
				}
			}

			return separationVector * avoidFactor;
		}

		Vec2f align(f32 perceptionRange, f32 matchingFactor, const std::vector<Boid*>& flock)
		{
			Vec2f alignVector;
			ui32 neighbors = 0;

			for (Boid* other : flock)
			{
				f32 dist = distance(m_position, other->getPosition());

				if (m_id != other->getID() && dist < perceptionRange)
				{
					alignVector = alignVector + other->getVelocity();
					neighbors++;
				}
			}

			if (neighbors > 0)
			{
				alignVector = alignVector / neighbors;
			}

			return (alignVector - m_velocity) * matchingFactor;
		}

		Vec2f cohesion(f32 perceptionRange, f32 centeringFactor, const std::vector<Boid*>& flock)
		{
			Vec2f cohesionVector;
			ui32 neighbors = 0;

			for (Boid* other : flock)
			{
				f32 dist = distance(m_position, other->getPosition());
				if (m_id != other->getID() && dist < perceptionRange)
				{
					cohesionVector = cohesionVector + other->getPosition();
					neighbors++;
				}
			}

			if (neighbors > 0)
			{
				cohesionVector = cohesionVector / neighbors;
			}

			return (cohesionVector - m_velocity) * centeringFactor;
		}

		Vec2f seek(Vec2f targetPosition, f32 maxAcceleration)
		{
			Vec2f steeringForce = targetPosition - m_position;
			steeringForce.normalize();
			steeringForce = steeringForce * maxAcceleration;

			return steeringForce;
		}

		Vec2f flee(Vec2f targetPosition, f32 maxAcceleration)
		{
			Vec2f steeringForce = m_position - targetPosition;
			steeringForce.normalize();
			steeringForce = steeringForce * maxAcceleration;

			return steeringForce;
		}

		Vec2f pursue(const Boid* target, f32 maxAcceleration, f32 maxPrediction)
		{
			f32 speed = m_velocity.getMagnitude();
			Vec2f direction = target->getPosition() - m_position;
			f32 distance = direction.getMagnitude();

			if (distance <= 0.01f)
			{
				return Vec2f{};
			}

			f32 prediction;
			if (speed <= distance / maxPrediction)
				prediction = maxPrediction;
			else
				prediction = distance / speed;

			Vec2f newPosition = target->getPosition() + target->getVelocity() * prediction;
			return seek(newPosition, maxAcceleration);
		}

		Vec2f evade(const Boid* target, f32 maxAcceleration, f32 maxPrediction)
		{
			f32 speed = m_velocity.getMagnitude();
			Vec2f direction = target->getPosition() - m_position;
			f32 distance = direction.getMagnitude();

			if (distance <= 0.01f)
			{
				return Vec2f{};
			}

			f32 prediction;
			if (speed <= distance / maxPrediction)
				prediction = maxPrediction;
			else
				prediction = distance / speed;

			Vec2f newPosition = target->getPosition() + target->getVelocity() * prediction;
			return flee(newPosition, maxAcceleration);
		}

		Vec2f arrive(Vec2f targetPosition, f32 slowRadius, f32 maxAcceleration)
		{
			Vec2f desiredVelocity = targetPosition - m_position;
			f32 distance = desiredVelocity.getMagnitude();

			if (distance <= 0.01f)
			{
				return Vec2f{};
			}

			desiredVelocity.normalize();

			if (distance > slowRadius)
			{
				desiredVelocity = desiredVelocity * m_maxSpeed;
			}
			else
			{
				desiredVelocity = desiredVelocity * m_maxSpeed * distance / slowRadius;
			}

			Vec2f steeringForce = desiredVelocity - m_velocity;

			if (steeringForce.getMagnitude() > maxAcceleration)
			{
				steeringForce.normalize();
				steeringForce = steeringForce * maxAcceleration;
			}

			return steeringForce;
		}

		Vec2f wander(f32 circleDistance, f32 circleRadius, f32 displaceRange, f32 maxAcceleration)
		{
			if (m_velocity.getMagnitude() <= 0.01f)
			{
				return Vec2f{};
			}

			Vec2f desired = m_velocity;
			desired.setMagnitude(circleDistance);
			desired = desired + m_position;

			f32 theta = m_theta + getRotationInDegrees();

			f32 x = circleRadius * cos(theta);
			f32 y = circleRadius * sin(theta);

			desired = desired + Vec2f{ x, y };

			Vec2f steeringForce = desired - m_position;
			steeringForce.normalize();

			m_theta = m_theta + randomFloat(-displaceRange, displaceRange);

			return steeringForce * maxAcceleration;
		}

		void applySteeringForce(Vec2f steeringForce)
		{
			m_velocity = m_velocity + steeringForce;
		}

		void updatePosition(f32 dt)
		{
			if (m_velocity.getMagnitude() > m_maxSpeed)
			{
				m_velocity = (m_velocity / m_velocity.getMagnitude()) * m_maxSpeed;
			}

			if (m_velocity.getMagnitude() <= 0.01f)
			{
				m_velocity = 0.0f;
			}

			m_position = m_position + m_velocity * dt;
		}

	private:
		ui32 m_id;
		f32 m_lastRotation = 0.0f;
		f32 m_theta = PI / 2.0f;
		f32 m_minSpeed = 1.0f, m_maxSpeed = 5.0f;
		Vec2f m_position, m_velocity;
	};

	class Flocking
	{
	public:
		Flocking() {}

		~Flocking()
		{
			for (ui32 i = 0; i < boids.size(); i++)
			{
				delete boids[i];
			}
		}

		void addBoid(Vec2f position, Vec2f velocity, f32 minSpeed, f32 maxSpeed)
		{
			Boid* boid = new Boid(position, velocity, maxSpeed, boids.size());
			boid->setMinSpeed(minSpeed);
			boids.push_back(boid);
		}

		void setRanges(f32 separationRange, f32 perceptionRange)
		{
			m_separationRange = separationRange;
			m_perceptionRange = perceptionRange;
		}

		void update(f32 dt, f32 avoidFactor, f32 matchingFactor, f32 centeringFactor)
		{
			for (Boid* boid : boids)
			{
				boid->flocking(dt, m_separationRange, m_perceptionRange, avoidFactor, matchingFactor, centeringFactor, boids);
			}
		}

		std::vector<Boid*> boids;

	private:
		f32 m_separationRange = 0.0f, m_perceptionRange = 0.0f;
	};
}
