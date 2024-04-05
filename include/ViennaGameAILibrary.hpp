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
	typedef uint32_t ui32;
	typedef uint64_t ui64;
	typedef int32_t i32;
	typedef float f32;

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

		void normalize()
		{
			f32 mag = std::sqrt(x * x + y * y);
			if (mag <= 0.0f)
			{
				std::cout << "mag < 0" << std::endl;
				return;
			}
			x = x / mag;
			y = y / mag;
		}
	};

	std::ostream& operator<<(std::ostream& os, const Vec2f& vec)
	{
		return os << vec.x << ", " << vec.y;
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
		NavMesh(ui32 width, ui32 height, ui32 obstaclePercentage)
			: m_width(width)
			, m_height(height)
		{
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100); // distribution in range [0, 1]

			for (ui32 y = 0; y < m_height; y++)
			{
				for (ui32 x = 0; x < m_width; x++)
				{
					NodeData node(Vec2ui{ x, y });

					auto val = distribution(rng);
					if (val <= obstaclePercentage)
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

			createRegions();
		}

		~NavMesh() {}

		std::vector<Vec2ui> A_Star(Vec2ui startPos, Vec2ui endPos)
		{
			std::vector<i32> parents(m_nodes.size(), -1);

			for (NodeData& node : m_nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::priority_queue<NodeData, std::vector<NodeData>, NodeDataComparator> openSet;

			ui32 startNodeIndex = getIndex(startPos);
			ui32 endNodeIndex = getIndex(endPos);

			openSet.push(m_nodes[startNodeIndex]);

			m_nodes[startNodeIndex].g = 0.0f;

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
						shortestPath.push_back(m_nodes[currentIndex].pos);
						currentIndex = parents[currentIndex];
					}

					std::reverse(shortestPath.begin(), shortestPath.end());
					return shortestPath;
				}

				std::vector<ui32> neighbors = m_neighbors[currentIndex];
				for (ui32 neighborIndex : neighbors)
				{
					NodeData& neighbor = m_nodes[neighborIndex];
					if (neighbor.state == NodeState::OBSTRUCTABLE)
						continue;

					f32 tentativeG = m_nodes[currentIndex].g + euclidean(neighbor.pos, m_nodes[currentIndex].pos);

					if (tentativeG < neighbor.g)
					{
						parents[neighborIndex] = getIndex(m_nodes[currentIndex].pos);
						neighbor.g = tentativeG;
						neighbor.h = euclidean(neighbor.pos, m_nodes[currentIndex].pos);

						openSet.push(m_nodes[neighborIndex]);
					}
				}
			}

			return {};
		}

		void preprocess()
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
						std::vector<Vec2ui> path = A_Star(m_nodes[i].pos, m_nodes[O_tentative].pos);

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

		std::vector<Vec2ui> getPath(Vec2ui start, Vec2ui target)
		{
			NodeData& startNode = getNode(start);
			NodeData& targetNode = getNode(target);

			ui32 startRegionID = startNode.regionID;
			ui32 targetRegionID = targetNode.regionID;

			if (startRegionID == targetRegionID)
			{
				return A_Star(start, target);
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

			std::vector<Vec2ui> pathWithinRegion = A_Star(regionStartNode, target);
			if (pathWithinRegion.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			pathToRegion.insert(pathToRegion.end(), pathWithinRegion.begin(), pathWithinRegion.end());

			return pathToRegion;
		}

		std::vector<Vec2ui> A_Star_multithreading(Vec2ui startPos, Vec2ui endPos)
		{
			std::vector<NodeData> nodes = m_nodes;

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

		void preprocess_multithreading(ui32 threadID, ui32 numThreads, std::vector<NodeData> nodes, std::vector<Region*> regionList)
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
							std::vector<Vec2ui> path = A_Star_multithreading(nodes[nodeIndex].pos, nodes[O_tentative].pos);

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

		void startPreprocess()
		{
			ui32 numThreads = 4;
			std::vector<std::thread> threads;

			ui32 regionsPerThread = m_regions->regions.size() / numThreads;

			for (ui32 i = 0; i < numThreads; i++)
			{
				threads.push_back(std::thread(&NavMesh::preprocess_multithreading, this, i, numThreads, m_nodes, m_regions->regions));
			}

			for (ui32 i = 0; i < threads.size(); i++)
			{
				threads[i].join();
			}
		}

		std::vector<Vec2ui> getPath_multithreading(Vec2ui start, Vec2ui target)
		{
			NodeData& startNode = getNode(start);
			NodeData& targetNode = getNode(target);

			ui32 startRegionID = startNode.regionID;
			ui32 targetRegionID = targetNode.regionID;

			if (startRegionID == targetRegionID)
			{
				return A_Star_multithreading(start, target);
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

			std::vector<Vec2ui> pathWithinRegion = A_Star_multithreading(regionStartNode, target);
			if (pathWithinRegion.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			pathToRegion.insert(pathToRegion.end(), pathWithinRegion.begin(), pathWithinRegion.end());

			return pathToRegion;
		}

		void setObstacle(Vec2ui obstaclePos)
		{
			ui32 index = getIndex(obstaclePos);
			if (m_nodes[index].state == NodeState::OBSTRUCTABLE)
			{
				m_nodes[index].state = NodeState::WALKABLE;
			}
			else
			{
				m_nodes[index].state = NodeState::OBSTRUCTABLE;
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
		void createRegions()
		{
			f32 regionLengthOnX = 9.0f;
			f32 regionLengthOnY = 9.0f;

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

	private:
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

			if(m_currentState->onUpdateCallback)
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
}