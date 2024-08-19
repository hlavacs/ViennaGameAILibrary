/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

/** @file */

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
	constexpr float PI = 3.14159265358979f;			/*!< The value of π computed at compile time */

	typedef uint32_t ui32;
	typedef uint64_t ui64;
	typedef int32_t i32;
	typedef float f32;

	/**
	 * @brief Custom assertion macro.
	 *
	 */
	#ifdef NDEBUG
	#define VGAIL_ASSERT(x, msg) { }
	#else
	#define VGAIL_ASSERT(x, msg) { if(!(x)) { std::cout << "Assertion Failed: " << msg << std::endl; __debugbreak(); } }
	#endif

	 /**
	  * @brief Custom 2D vector of unsigned integers.
	  */
	struct Vec2ui
	{
		ui32 x, y;

		/**
		 * @brief Constructs a new Vec2ui object with default coordinates (0, 0).
		 *
		 */
		Vec2ui()
			: x(0)
			, y(0)
		{}

		/**
		 * @brief Constructs a new Vec2ui object with the specified coordinates.
		 *
		 * @param x
		 * @param y
		 */
		Vec2ui(ui32 x, ui32 y)
			:x(x), y(y) {}

		/**
		 * @brief Constructs a new Vec2ui object with equal coordinates.
		 *
		 * @param val The value to set for both `x` and `y`.
		 */
		Vec2ui(ui32 val)
			: x(val), y(val) {}

		/**
		 * @brief  Checks if this Vec2ui object is equal to another Vec2ui object.
		 *
		 * Compares the `x` and `y` coordinates of two Vec2ui objects for equality.
		 *
		 * @param other Vec2ui object to compare against.
		 * @return `true` if the Vec2ui objects are equal, `false` otherwise.
		 */
		bool operator==(const Vec2ui& other)
		{
			return x == other.x && y == other.y;
		}
	};

	/**
	 * @brief Prints out a Vec2ui object.
	 *
	 * Usage: `std::cout << vec << std::endl;`
	 *
	 * @param os
	 * @param vec
	 * @return std::ostream&
	 */
	std::ostream& operator<<(std::ostream& os, const Vec2ui& vec)
	{
		return os << vec.x << ", " << vec.y;
	}

	/**
	 * @brief Custom 2D vector of floats.
	 */
	struct Vec2f
	{
		f32 x, y;

		/**
		 * @brief Constructs a new Vec2f object with default coordinates (0.0f, 0.0f).
		 *
		 */
		Vec2f()
			: x(0.0f)
			, y(0.0f)
		{}

		/**
		 * @brief Constructs a new Vec2f object with the specified coordinates.
		 *
		 * @param x
		 * @param y
		 */
		Vec2f(f32 x, f32 y)
			: x(x), y(y) {}

		/**
		 * @brief Constructs a new Vec2f object with equal coordinates.
		 *
		 * @param val The value to set for both `x` and `y`.
		 */
		Vec2f(f32 val)
			: x(val), y(val) {}

		/**
		 * @brief Substracts two Vec2f objects and returns a new Vec2f object representing their difference.
		 *
		 * Does not modify the original Vec2f objects. \n
		 * Usage: `Vec2f result = vec1 - vec2;`
		 *
		 * @param other Vec2f to be subtracted.
		 * @return Vec2f The new Vec2f object resulted from the subtraction.
		 */
		Vec2f operator-(const Vec2f& other)
		{
			return Vec2f{ x - other.x, y - other.y };
		}

		/**
		 * @brief Adds two Vec2f objects and returns a new Vec2f object representing their sum.
		 *
		 * Does not modify the original Vec2f objects. \n
		 * Usage: `Vec2f result = vec1 + vec2;`
		 *
		 * @param other Vec2f to be added.
		 * @return Vec2f The new Vec2f object resulted from the addition.
		 */
		Vec2f operator+(const Vec2f& other)
		{
			return Vec2f{ x + other.x, y + other.y };
		}

		/**
		 * @brief Divides two Vec2f objects and returns a new Vec2f object representing their division.
		 *
		 * Does not modify the original Vec2f objects. \n
		 * Usage: `Vec2f result = vec1 / vec2;`
		 *
		 * @param other Vec2f to be divided by.
		 * @return Vec2f The new Vec2f object resulted from the division.
		 */
		Vec2f operator/(const Vec2f& other)
		{
			return Vec2f{ x / other.x, y / other.y };
		}

		/**
		 * @brief Multiplies the Vec2f object by a scalar and returns a new Vec2f object representing the result.
		 *
		 * Does not modify the original Vec2f object. \n
		 * Usage: `Vec2f result = vec * value;`
		 *
		 * @param value The scalar value to multiply by.
		 * @return Vec2f The new Vec2f object resulted from the multiplication.
		 */
		Vec2f operator*(f32 value)
		{
			return Vec2f{ x * value, y * value };
		}

		/**
		 * @brief Divides the Vec2f object by a scalar and returns a new Vec2f object representing the result.
		 *
		 * Does not modify the original Vec2f object. \n
		 * Usage: `Vec2f result = vec / value;`
		 *
		 * @param value The scalar value to divide by.
		 * @return Vec2f The new Vec2f object resulted from the division.
		 */
		Vec2f operator/(f32 value)
		{
			return Vec2f{ x / value, y / value };
		}

		/**
			 * @brief  Checks if this Vec2f object is equal to another Vec2f object.
			 *
			 * Compares the `x` and `y` coordinates of two Vec2f objects for equality.
			 *
			 * @param other Vec2f object to compare against.
			 * @return `true` if the Vec2f objects are equal, `false` otherwise.
			 */
		bool operator==(const Vec2f& other)
		{
			return x == other.x && y == other.y;
		}

		/**
		 * @brief Gets the magnitude (length) of this Vec2f object.
		 *
		 * @return f32 The magnitude of this Vec2f object.
		 */
		f32 getMagnitude()
		{
			return std::sqrt(std::pow(x, 2) + std::pow(y, 2));
		}

		/**
		 * @brief Sets the magnitude (length) of this Vec2f object.
		 *
		 * @param value The value to set as magnitude.
		 */
		void setMagnitude(f32 value)
		{
			f32 mag = getMagnitude();

			if (mag <= 0.0f)
			{
				std::cout << "Magnitude is less than or equal to 0!" << std::endl;
				return;
			}

			f32 scaleFactor = value / mag;
			x *= scaleFactor;
			y *= scaleFactor;
		}

		/**
		 * @brief Normalizes this Vec2f object.
		 *
		 */
		void normalize()
		{
			f32 mag = getMagnitude();

			if (mag <= 0.0f)
			{
				std::cout << "Magnitude is less than or equal to 0!" << std::endl;
				return;
			}

			x /= mag;
			y /= mag;
		}
	};

	/**
	 * @brief Prints out a Vec2f object.
	 *
	 * Usage: `std::cout << vec << std::endl;`
	 *
	 * @param os
	 * @param vec
	 * @return std::ostream&
	 */
	std::ostream& operator<<(std::ostream& os, const Vec2f& vec)
	{
		return os << vec.x << ", " << vec.y;
	}

	/**
	 * @brief Generates a random float value between a range.
	 *
	 * @param min Minimum value of the range.
	 * @param max Maximum value of the range.
	 * @return f32 Randomly generated value.
	 */
	f32 randomFloat(f32 min, f32 max)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<f32> distribution(min, max);
		return distribution(rng);
	}

	/**
	 * @brief Generates a random signed integer value between a range.
	 *
	 * @param min Minimum value of the range.
	 * @param max Maximum value of the range.
	 * @return i32 Randomly generated value.
	 */
	i32 randomInt(i32 min, i32 max)
	{
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> distribution(min, max);
		return distribution(rng);
	}

	/**
	 * @brief Calculates the Euclidean distance between two Vec2f objects.
	 *
	 * @param v1 First Vec2f object.
	 * @param v2 Second Vec2f object.
	 * @return f32 Distance between the two vectors.
	 */
	f32 distance(const Vec2f& v1, const Vec2f& v2)
	{
		f32 distX = v1.x - v2.x;
		f32 distY = v1.y - v2.y;
		f32 dist = std::pow(distX, 2) + std::pow(distY, 2);

		return std::sqrt(dist);
	}

	/**
	 * @brief Custom struct used in geometric preprocessing.
	 *
	 * When geometric preprocessing is used, the navigation mesh is divided into regions, with each region having assigned a set of nodes.
	 */
	struct Region
	{
		ui32 regionID;								/*!< Region's index within RegionList */
		std::vector<ui32> nodes;					/*!< List of assigned nodes */
	};

	/**
	 * @brief Custom struct to manage all regions created for geometric preprocessing.
	 *
	 * During geometric preprocessing, the shortest distance between each region needs to be calculated and stored. In order to be able to loop
	 * through all existing regions, they are all stored in a vector and managed by this class.
	 */
	struct RegionList
	{
		std::vector<Region*> regions;				/*!< 1D array of all regions. */
		ui32 sizeX, sizeY;							/*!< Number of regions on the `x` and `y` axis. */

		/**
		 * @brief Constructs a new RegionList object.
		 *
		 * @param sizeX - Number of regions on the `x` axis.
		 * @param sizeY - Number of regions on the `y` axis.
		 */
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

		/**
		 * @brief Destroys the RegionList object.
		 *
		 */
		~RegionList()
		{
			for (ui32 i = 0; i < regions.size(); i++)
			{
				delete regions[i];
			}
		}

		/**
		 * @brief Gets the index of a region within the 1D array of regions.
		 *
		 * @param pos 2D position of the region.
		 * @return ui32 The index of the region.
		 */
		ui32 getRegionID(Vec2ui pos)
		{
			return pos.x + pos.y * sizeX;
		}
	};

	/**
	 * @brief States of the navmesh nodes.
	 *
	 * Characters can pass through walkable nodes only, and obstructable nodes could be obstacles, walls etc.
	 */
	enum NodeState
	{
		OBSTRUCTABLE,
		WALKABLE
	};

	/**
	 * @brief Custom struct to hold node data.
	 *
	 */
	struct NodeData
	{
		Vec2ui pos;									/*!< 2D position of the node. */
		f32 g;										/*!> The cost of the path from the start node to the current node. */
		f32 h;										/*!< The heuristic function. */
		ui32 regionID;								/*!< The id of the region this node is part of. */
		NodeState state;							/*!< State of this node. */

		/**
		 * @brief Constructs a new NodeData object.
		 *
		 * @param pos 2D position of the node.
		 * @param nodeState State of the node; by default is `walkable`.
		 */
		NodeData(Vec2ui pos, NodeState nodeState = NodeState::WALKABLE)
			: pos(pos)
			, g(INFINITY)
			, h(INFINITY)
			, state(nodeState)
		{}

		/**
		 * @brief Calculates the total cost estimate of the most efficient path from the start node to the end node.
		 *
		 * @return f32 Total cost estimate.
		 */
		f32 f() const
		{
			return g + h;
		}

		/**
		 * @brief Compares this NodeData object to another NodeData object.
		 *
		 * Compares the `regionID`, `state` and `g` and `h` values of two NodeData objects for equality.
		 *
		 * @param other NodeData to compare against.
		 * @return `true` if the NodeData objects are equal, `false` otherwise.
		 */
		bool operator==(const NodeData& other)
		{
			return pos == other.pos && g == other.g && h == other.h && regionID == other.regionID && state == other.state;
		}
	};

	/**
	 * @brief Custom struct that compares two NodeData objects in a priority queue.
	 *
	 */
	struct NodeDataComparator
	{
		/**
		 * @brief Compares two NodeData objects based on their `f()` value.
		 *
		* Used in the A* algorithm when managing the priority queue that contains all nodes to be looked at during the algorithm. \n
		* Whenever a NodeData object is added to the queue, it checks where to put it depending on its `f()` value by comparing it to the other objects in the list.
		 *
		 * @param node1 First node.
		 * @param node2 Second node.
		 * @return `true` if the first NodeData object has a higher `f()` value, `false` otherwise.
		 */
		bool operator()(const NodeData& node1, const NodeData& node2) const
		{
			return node1.f() > node2.f();
		}
	};

	/**
	 * @brief Custom navigation mesh created especially for path finding.
	 *
	 * It can be used without path finding as well.
	 */
	class NavMesh
	{
	public:
		/**
		 * @brief Constructs a new NavMesh object by creating nodes with random states.
		 *
		 * @param width The width of the navigation mesh.
		 * @param height The height of the navigation mesh.
		 * @param obstaclePercentage The percentage to limit the amount of nodes with the state `obstructable` spawned within the navigation mesh.
		 * @param regionLengthOnX The amount of regions on the `x` axis. Only needed for geometric preprocessing; by default it is set to 9.
		 * @param regionLengthOnY Theamount of regions on the `y` axis. Only needed for geometric preprocessing; by default it is set to 9.
		 */
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

		/**
		 * @brief Constructs a new NavMesh object by loading data from a file.
		 *
		 * @param filepath The relative path of the file.
		 * @param regionLengthOnX The amount of regions on the `x` axis. Only needed for geometric preprocessing; by default it is set to 9.
		 * @param regionLengthOnY Theamount of regions on the `y` axis. Only needed for geometric preprocessing; by default it is set to 9.
		 */
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

		/**
		 * @brief Destroys the NavMesh object.
		 *
		 */
		~NavMesh() {}

		/**
		 * @brief Performs geometric preprocessing with optional multithreading.
		 *
		 * @param multithreading Set to `true` to enable multithreading, `false` otherwise.
		 * @param numThreads The number of threads to use if multithreading is enabled.
		 *
		 * a) Multithreaded \n
		 * Each thread receives a part of the already defined regions and is assigned the `preprocessWorker()` method.
		 *
		 * b) Single-threaded \n
		 * For each node, calculate the shortest path to each region and store it at position `m_adjList[nodeIndex][regionID]`,
		 * where `nodeIndex` is the index of the node within `m_nodes` and `regionID` is the ID of the region it found a path to.
		 * `m_adjList` will then be used to retrieve shortest path within regions.
		 */
		void preprocess(bool multithreading = false, ui32 numThreads = 4)
		{
			m_isPreprocessed = true;
			if (multithreading)
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

					// Avoid unavailable nodes
					if (m_nodes[i].state == NodeState::OBSTRUCTABLE)
						continue;

					for (Region* R : m_regions->regions)
					{
						// Ignore the region this node belongs to
						if (m_nodes[i].regionID == R->regionID)
							continue;

						std::vector<std::vector<Vec2ui>> paths;

						// Calculate distances from this node to all nodes in region R
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

						// Find shortest path from this node to region R
						ui32 bestPathIndex = 0;
						for (ui32 j = 1; j < paths.size(); j++)
						{
							if (paths[j].size() < paths[bestPathIndex].size())
							{
								bestPathIndex = j;
							}
						}

						// Store the shortest path
						m_adjList[i][R->regionID] = paths[bestPathIndex];
					}
				}
			}
		}

		/**
		 * @brief Finds the shortest path between two NodeData objects without using geometric preprocessing.
		 *
		 * This method calls the A* algorithm directly and returns the shortest path between two nodes.
		 *
		 * @param start The position of the start node.
		 * @param target The position of the target node.
		 * @return std::vector<Vec2ui> The shortest found path.
		 */
		std::vector<Vec2ui> findPath(Vec2ui start, Vec2ui target)
		{
			return AStar(start, target, m_nodes);
		}

		/**
		 * @brief Finds the stored path between two nodes.
		 *
		 * After geometric preprocessing, the shortest paths between each node to each region is stored at `m_adjList[nodeIndex][regionID]`. \n
		 * 	- If the nodes are in the same region, it simply calls the A* algorithm to find the shortest path between them. \n
		 * 	- If they are in separate regions, it first retrieves the shortest path from the start node to the region in which the
		 * target node exists. If the newly found path's end node is the target, the path is returned. Otherwise, A* will run to find
		 * the path from this end node to the target node and return it, if found.
		 *
		 * @param start The position of the start node.
		 * @param target The position of the target node.
		 * @return std::vector<Vec2ui> The shortest found path.
		 */
		std::vector<Vec2ui> findPreprocessedPath(Vec2ui start, Vec2ui target)
		{
			NodeData& startNode = getNode(start);
			NodeData& targetNode = getNode(target);

			ui32 startRegionID = startNode.regionID;
			ui32 targetRegionID = targetNode.regionID;

			// If start and target nodes are in the same region, call A* to find the shortest path
			if (startRegionID == targetRegionID)
			{
				return findPath(start, target);
			}

			// Check if there is at least one precomputed path from the start node stored in `m_adjList`
			if (m_adjList[getIndex(startNode.pos)].size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}

			ui32 startNodeIndex = getIndex(startNode.pos);
			std::vector<Vec2ui> pathToRegion = m_adjList[startNodeIndex][targetRegionID];

			// Check if there is a precomputed path from the start node to the region in which the target node is
			if (pathToRegion.size() == 0)
			{
				std::cout << "No path found to region!" << std::endl;
				return {};
			}

			// Retrieve the last node from the newly found path and check whether it is the target node
			Vec2ui pathEndNode = pathToRegion[pathToRegion.size() - 1];
			if (pathEndNode == target)
			{
				// Reached destination
				return pathToRegion;
			}

			// If the end node of the path is not the target, get the shortest path between it and the target
			std::vector<Vec2ui> pathWithinRegion = findPath(pathEndNode, target);
			if (pathWithinRegion.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			// Combine the two paths together: (start -> endNode) + (endNode -> target)
			pathToRegion.insert(pathToRegion.end(), pathWithinRegion.begin(), pathWithinRegion.end());

			return pathToRegion;
		}

		/**
		 * @brief Saves the structure of a navigation mesh to a file.
		 *
		 * The file will consist of 3 main lines: the first one will represent the width, the second one will be the height and the third will be the pattern.
		 *
		 * @param filepath The relative path of the file, including the filename; if the file does not exist, it will be created automatically.
		 */
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

		/**
		 * @brief Sets the state of a NodeData object to `obstructable`.
		 *
		 * @param pos The 2D position of the NodeData object.
		 */
		void setObstructable(Vec2ui pos)
		{
			ui32 index = getIndex(pos);
			if (m_nodes[index].state != NodeState::OBSTRUCTABLE)
			{
				m_nodes[index].state = NodeState::OBSTRUCTABLE;
			}
		}

		/**
		 * @brief Sets the state of a NodeData object to `walkable`.
		 *
		 * @param pos The 2D position of the NodeData object.
		 */
		void setWalkable(Vec2ui pos)
		{
			ui32 index = getIndex(pos);
			if (m_nodes[index].state != NodeState::WALKABLE)
			{
				m_nodes[index].state = NodeState::WALKABLE;
			}
		}

		/**
		 * @brief Gets the width of the navigation mesh.
		 *
		 * @return ui32 Width of the navigation mesh.
		 */
		ui32 getWidth()
		{
			return m_width;
		}

		/**
		 * @brief Gets the height of the navigation mesh.
		 *
		 * @return ui32 Height of the navigation mesh.
		 */
		ui32 getHeight()
		{
			return m_height;
		}

		/**
		 * @brief Gets the 2D coordinates of a NodeData object.
		 *
		 * @param index The index of the NodeData object within the navmesh (`m_nodes`).
		 * @return Vec2ui 2D coordinates of the NodeData object.
		 */
		Vec2ui get2DCoordinates(ui32 index)
		{
			ui32 x = index % m_width;
			ui32 y = (index - x) / m_width;
			return Vec2ui(x, y);
		}

		/**
		 * @brief Gets the index of a NodeData object within the navmesh.
		 *
		 * @param pos The 2D coordinates of the NodeData object
		 * @return ui32 The index of the NodeData object.
		 */
		ui32 getIndex(Vec2ui pos)
		{
			return pos.x + pos.y * m_width;
		}

		/**
		 * @brief Gets a NodeData object.
		 *
		 * @param pos The 2D coordinates of the NodeData object.
		 * @return NodeData& The NodeData object.
		 */
		NodeData& getNode(Vec2ui pos)
		{
			return m_nodes[getIndex(pos)];
		}

	private:
		/**
		 * @brief Creates Region objects and assigns nodes to each region.
		 *
		 * Used for geometric preprocessing.
		 * It divides the navmesh into regions and determines which region each node should be assigned to.
		 *
		 * @param regionLengthOnX The amount of regions on the `x` axis.
		 * @param regionLengthOnY The amount of regions on the `y` axis.
		 */
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

		/**
		 * @brief Finds and stores the neighbors of each node within the navmesh.
		 *
		 * Done after generating or loading a navmesh.
		 * It determines the neighbors of each node and stores them at `m_neighbors[nodeIndex]`.
		 *
		 * @param nodeIndex The index of the node within the navmesh.
		 */
		void setNeighbors(ui32 nodeIndex)
		{
			NodeData& node = m_nodes[nodeIndex];
			f32 x = node.pos.x;
			f32 y = node.pos.y;

			for (i32 v = -1; v <= 1; ++v)
			{
				for (i32 u = -1; u <= 1; ++u)
				{
					// Skip current node
					if (u == 0 && v == 0)
						continue;

					if (x + u >= 0 && y + v >= 0 &&
						x + u < m_width && y + v < m_height)
					{
						m_neighbors[nodeIndex].push_back(getIndex(Vec2ui(x + u, y + v)));
					}
				}
			}
		}

		/**
		 * @brief Calculates the Manhattan distance between two Vec2ui objects.
		 *
		 * @param v1 First Vec2ui object.
		 * @param v2 Second Vec2ui object.
		 * @return f32 The distance between the Vec2ui objects.
		 */
		f32 manhattan(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 distX = std::abs(static_cast<f32>(v1.x) - static_cast<f32>(v2.x));
			f32 distY = std::abs(static_cast<f32>(v1.y) - static_cast<f32>(v2.y));

			return distX + distY;
		}

		/**
		 * @brief Calculates the Euclidean distance between two Vec2ui objects.
		 *
		 * @param v1 First Vec2ui object.
		 * @param v2 Second Vec2ui object.
		 * @return f32 The distance between the Vec2ui objects.
		 */
		f32 euclidean(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 distX = static_cast<f32>(v1.x) - static_cast<f32>(v2.x);
			f32 distY = static_cast<f32>(v1.y) - static_cast<f32>(v2.y);
			f32 dist = std::pow(distX, 2) + std::pow(distY, 2);

			return std::sqrt(dist);
		}

		/**
		 * @brief Method to be called by each thread during geometric preprocessing.
		 *
		 * This method does the same thing as the `preprocess()` method, only that it is adapted to work for individual threads.
		 *
		 * @param threadID The ID of the thread.
		 * @param numThreads The total number of threads used.
		 * @param nodes The navmesh nodes. Each thread receives a copy of the nodes such that data racing is avoided.
		 * @param regionList The list of regions.
		 */
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

		/**
		 * @brief The A* algorithm. Finds the shortest path between two nodes.
		 *
		 * @param start The position of the start node.
		 * @param target The position of the target node.
		 * @param nodes The nodes of the navmesh.
		 * @return std::vector<Vec2ui> The shortest path between the start and the target nodes.
		 */
		std::vector<Vec2ui> AStar(Vec2ui start, Vec2ui target, std::vector<NodeData>& nodes)
		{
			std::vector<i32> parents(nodes.size(), -1);

			for (NodeData& node : nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::priority_queue<NodeData, std::vector<NodeData>, NodeDataComparator> openSet;

			ui32 startNodeIndex = getIndex(start);
			ui32 targetNodeIndex = getIndex(target);

			nodes[startNodeIndex].g = 0.0f;

			openSet.push(nodes[startNodeIndex]);

			while (openSet.size() > 0)
			{
				ui32 currentIndex = getIndex(openSet.top().pos);

				while (openSet.size() > 0 && currentIndex == getIndex(openSet.top().pos))
				{
					openSet.pop();
				}

				if (currentIndex == targetNodeIndex)
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
						neighbor.h = euclidean(neighbor.pos, nodes[targetNodeIndex].pos);

						openSet.push(nodes[neighborIndex]);
					}
				}
			}

			return {};
		}

	private:
		bool m_isPreprocessed = false;													/*!< Indicates whether geometric preprocessing has been performed. */
		ui32 m_width, m_height;															/*!< Width and height of the navigation mesh. */
		RegionList* m_regions;															/*!< The list of regions. */
		std::vector<NodeData> m_nodes;													/*!< The nodes of the navigation mesh. */
		std::vector<std::vector<ui32>> m_neighbors;										/*!< The list of the nodes' neighbors. */
		std::vector<std::unordered_map<ui32, std::vector<Vec2ui>>> m_adjList;			/*!< The list of all preprocessed paths from each node to each region. */
	};

	class State;
	/**
	 * @brief Custom class that manages the behavior of transitions between states.
	 *
	 * Used in state machines. \n
	 * These transitions lead from one State to another (the `target` state) based on a set of given conditions.
	 * If the conditions are met, the `target` state becomes active.
	 */
	class Transition
	{
	public:
		/**
		 * @brief Constructs a new Transition object.
		 *
		 * @param target Target state to check conditions for.
		 * @param callback Condition that determines if the target state should be activated.
		 */
		Transition(State* target, std::function<bool()> callback)
		{
			targetState = target;
			onCheckCallback = callback;
		}

		/**
		 * @brief Destroys the Transition object.
		 *
		 */
		~Transition() = default;

		std::function<bool()> onCheckCallback;			/*!< Checks the given condition. */
		State* targetState = nullptr;					/*!< The state this `Transition` object is checking for. */
	};

	/**
	 * @brief Custom class that manages the behavior of states.
	 *
	 * Used in state machines. \n
	 * States represent actions or behaviors. They are triggered by transitions depending on certain conditions.
	 */
	class State
	{
	public:
		/**
		 * @brief Constructs a new State object.
		 *
		 */
		State() {}

		/**
		 * @brief Destroys the State object alongside its set of outgoing transitions.
		 *
		 */
		~State() {
			for (Transition* transition : m_transitions)
			{
				delete transition;
			}
		}

		/**
		 * @brief Method called when a State object is entered from a transition.
		 *
		 */
		std::function<void()> onEnterCallback;

		/**
		 * @brief Method called when a State object is exited.
		 */
		std::function<void()> onExitCallback;

		/**
		 * @brief Method called while a State object is active.
		 *
		 * While the State is active, this method is called to carry out a list of given actions.
		 */
		std::function<void(f32)> onUpdateCallback;

		/**
		 * @brief Creates an outgoing transition from this State object to another State object.
		 *
		 * This method adds a transition between this State object and another given one. It is given a condition which,
		 * if triggered, will activate the target state.
		 *
		 * @param targetState The target State.
		 * @param callback The condition on which the target State should activate.
		 */
		void addTransition(State* targetState, std::function<bool()> callback)
		{
			m_transitions.push_back(new Transition(targetState, callback));
		}

		/**
		 * @brief Gets a list of all transitions outgoing from this State object.
		 *
		 * @return std::vector<Transition*> The list of this state's outgoing transitions.
		 */
		std::vector<Transition*> getTransitions()
		{
			return m_transitions;
		}

	private:
		std::vector<Transition*> m_transitions;				/*!< The list of this state's outgoing transitions. */
	};

	/**
	 * @brief Custom class that manages the behavior of a state machine.
	 *
	 * It manages the states and transitions between them.
	 */
	class StateMachine
	{
	public:
		/**
		 * @brief Constructs a new StateMachine object.
		 *
		 */
		StateMachine() {}

		/**
		 * @brief Destroys the StateMachine object and its states.
		 *
		 */
		~StateMachine()
		{
			for (State* state : m_states)
			{
				delete state;
			}
		}

		/**
		 * @brief Creates a new State object, adds it to the list of states and returns it.
		 *
		 * @return State* The new State object.
		 */
		State* createState()
		{
			State* state = new State();
			m_states.push_back(state);
			return state;
		}

		/**
		 * @brief Updates the states and transitions between them.
		 *
		 * It should be called during the game loop such that it continuously checks if any transitions are triggered
		 * and if any states need to be activated.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 */
		void update(f32 deltaTime)
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
				m_currentState->onUpdateCallback(deltaTime);
			}
		}

		/**
		 * @brief Gets the current active State object.
		 *
		 * @return State* The current active State object.
		 */
		State* getCurrentState()
		{
			return m_currentState;
		}

	private:
		std::vector<State*> m_states;						/*!< The list of all states. */
		State* m_currentState = nullptr;					/*!< The current active state. */
	};

	/**
	 * @brief Custom class that manages the nodes of a decision tree.
	 *
	 */
	class DecisionNode
	{
	public:
		/**
		 * @brief Destroys the DecisionNode object and its child nodes.
		 *
		 */
		virtual ~DecisionNode()
		{
			for (DecisionNode* node : m_children)
			{
				delete node;
			}
		}

		/**
		 * @brief The implementation of this DecisionNode object.
		 *
		 * This method either carries out a list of given actions or it delegates work to its child nodes based on some criteria.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 */
		virtual void makeDecision(f32 deltaTime) = 0;

		/**
		 * @brief Get the child node of this DecisionNode object at a given index.
		 *
		 * @param index The index of the child node.
		 * @return DecisionNode& The child node of the DecisionNode object.
		 */
		DecisionNode& getChild(ui32 index)
		{
			VGAIL_ASSERT(index < m_children.size(), "Index out of bounds!");
			return *m_children[index];
		}

		/**
		 * @brief Creates a new DecisionNode object and adds it to the list of child nodes of this DecisionNode object.
		 *
		 * This is a templated method, meaning any custom class can be added as a child node as long as it inherits from DecisionNode. \n
		 * Any number of variables can be passed, but if there is a mismatch of variables with the `T` constructor then there will be a compile time error.
		 *
		 * @tparam T Generic class.
		 * @tparam Args Variadic variables.
		 * @param args Variadic variables.
		 * @return DecisionNode& The new DecisionNode object.
		 */
		template <class T, typename... Args>
		DecisionNode& addChild(Args&& ...args)
		{
			DecisionNode* child = new T(args...);
			m_children.push_back(child);
			return *child;
		}

		/**
		 * @brief Get the number of child nodes of this DecisionNode object.
		 *
		 * @return ui32 The number of child nodes.
		 */
		ui32 getChildrenSize()
		{
			return m_children.size();
		}

	private:
		std::vector<DecisionNode*> m_children;					/*!< The list of child nodes of this DecisionNode object. */
	};

	/**
	 * @brief Custom class that manages the nodes of a decision tree.
	 *
	 */
	class DecisionTree
	{
	public:
		/**
		 * @brief Constructs a new DecisionTree object.
		 *
		 */
		DecisionTree() {}

		/**
		 * @brief Destroys the DecisionTree object.
		 *
		 */
		~DecisionTree()
		{
			resetTree();
		}

		/**
		 * @brief Loops through the decision tree and performs the decision-making process.
		 *
		 * Starts from the root node and calls the `makeDecision()` method on its child nodes.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 */
		void update(f32 deltaTime)
		{
			if (m_root)
			{
				m_root->makeDecision(deltaTime);
			}
		}

		/**
		 * @brief Creates and returns the root of the decision tree.
		 *
		 * This is a templated method, meaning any custom class can be used as a root node as it inherits from DecisionNode. \n
		 * Any number of variables can be passed, but if there is a mismatch of variables with the `T` constructor then there will be a compile time error.
		 *
		 * @tparam T Generic class.
		 * @tparam Args Variadic variables.
		 * @param args Variadic variables.
		 * @return DecisionNode& The newly created root of the decision tree.
		 */
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
		/**
		 * @brief Resets the decision tree by deleting its root.
		 *
		 */
		void resetTree()
		{
			if (m_root)
			{
				delete m_root;
			}
		}

	private:
		DecisionNode* m_root = nullptr;			/*!< The root of this DecisionTree object. */
	};

	/**
	 * @brief Custom class responsible for managing agents that can use steering behaviors.
	 *
	 */
	class Boid
	{
	public:
		/**
		 * @brief Constructs a new Boid object.
		 *
		 * @param position The position of the Boid object.
		 * @param velocity The velocity of the Boid object.
		 * @param maxSpeed The maximum speed of the Boid object.
		 * @param id The ID of the Boid object. Only used in flocking; by default it is 0.
		 */
		Boid(Vec2f position, Vec2f velocity, f32 maxSpeed, ui32 id = 0)
			: m_position(position)
			, m_velocity(velocity)
			, m_maxSpeed(maxSpeed)
			, m_id(id)
		{}

		/**
		 * @brief Destroys this Boid object.
		 *
		 */
		~Boid() {}

		/**
		 * @brief Sets a new position for this Boid object.
		 *
		 * @param position The new position to be set.
		 */
		void setPosition(Vec2f position)
		{
			m_position = position;
		}

		/**
		 * @brief Gets the position of this Boid object.
		 *
		 * @return Vec2f The position of this Boid object.
		 */
		Vec2f getPosition() const
		{
			return m_position;
		}

		/**
		 * @brief Sets a new velocity for this Boid object.
		 *
		 * @param velocity The new velocity to be set.
		 */
		void setVelocity(Vec2f velocity)
		{
			m_velocity = velocity;
		}

		/**
		 * @brief Gets the velocity of this Boid object.
		 *
		 * @return Vec2f The velocity of this Boid object.
		 */
		Vec2f getVelocity() const
		{
			return m_velocity;
		}


		/**
		 * @brief Gets the ID of this Boid object.
		 *
		 * @return ui32 The ID of this Boid object.
		 */
		ui32 getID() const
		{
			return m_id;
		}

		/**
		 * @brief Gets the rotation of this Boid object in degrees.
		 *
		 * @return f32 The rotation of this Boid object in degrees.
		 */
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

		/**
		 * @brief Sets the minimum speed of this Boid object.
		 *
		 * @param minSpeed The value to which the minimum speed should be set to.
		 */
		void setMinSpeed(f32 minSpeed)
		{
			m_minSpeed = minSpeed;
		}

		/**
		 * @brief Sets the maximum speed of this Boid object.
		 *
		 * @param maxSpeed The value to which the maximum speed should be set to.
		 */
		void setMaxSpeed(f32 maxSpeed)
		{
			m_maxSpeed = maxSpeed;
		}

		/**
		 * @brief The flocking algorithm implemented per individual Boid.
		 *
		 * This method manages the three steering behaviors (separation, alignment and cohesion) in order to simulate a realistic simulation of flocking behavior.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 * @param separationRange The range to avoid colliding with other boids.
		 * @param perceptionRange The range within which a boid considers other boids as part of the flock.
		 * @param avoidFactor How strongly this boid reacts to possible collisions.
		 * @param matchingFactor How strongly this boid steers to match the average velocity of their neighbours.
		 * @param centeringFactor How strongly this boid steers to match the average position of their neighbours.
		 * @param flock The flock this Boid object is part of.
		 */
		void flocking(f32 deltaTime, f32 separationRange, f32 perceptionRange, f32 avoidFactor, f32 matchingFactor, f32 centeringFactor, const std::vector<Boid*>& flock)
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
				alignVector = alignVector / static_cast<f32>(neighbors);
				cohesionVector = cohesionVector / static_cast<f32>(neighbors);

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

			m_position = m_position + m_velocity * deltaTime;
		}

		/**
		 * @brief Performs the separation steering behavior.
		 *
		 * Separation ensures that the boids do not overlap, thus steers the boids away from one another to avoid crowding.
		 *
		 * @param separationRange The range to avoid colliding with other boids.
		 * @param avoidFactor How strongly this boid reacts to possible collisions.
		 * @param flock  The flock this Boid object is part of.
		 * @return Vec2f The steering force used to steer away from close boids.
		 */
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

		/**
		 * @brief Performs the alignment steering behavior.
		 *
		 * Alignment is responsible for calculating the average velocity of the boids and steer them accordingly.
		 *
		 * @param perceptionRange The range within which a boid considers other boids as part of the flock.
		 * @param matchingFactor How strongly this boid steers to match the average velocity of their neighbours.
		 * @param flock The flock this Boid object is part of.
		 * @return Vec2f The steering force used to steer towards the average velocity of the neighbouring boids.
		 */
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


		/**
		 * @brief Performs the cohesion steering behavior.
		 *
		 * @param perceptionRange The range within which a boid considers other boids as part of the flock.
		 * @param centeringFactor How strongly this boid steers to match the average position of their neighbours.
		 * @param flock The flock this Boid object is part of.
		 * @return Vec2f The steering force used to steer towards the average position of the neighbouring boids.
		 */
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

		/**
		 * @brief Performs the seek steering behavior.
		 *
		 * The seek steering behavior allows for a realistic movement towards a given target.
		 *
		 * @param targetPosition The position of the target.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @return Vec2f The steering force used to steer towards the target.
		 */
		Vec2f seek(Vec2f targetPosition, f32 maxAcceleration)
		{
			Vec2f steeringForce = targetPosition - m_position;
			steeringForce.normalize();
			steeringForce = steeringForce * maxAcceleration;

			return steeringForce;
		}

		/**
		 * @brief Performs the flee steering behavior.
		 *
		 * The flee steering behavior allows for a realistic movement of "running away" from a given target.
		 *
		 * @param targetPosition The position of the target.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @return Vec2f The steering force used to flee away from the target.
		 */
		Vec2f flee(Vec2f targetPosition, f32 maxAcceleration)
		{
			Vec2f steeringForce = m_position - targetPosition;
			steeringForce.normalize();
			steeringForce = steeringForce * maxAcceleration;

			return steeringForce;
		}

		/**
		 * @brief Performs the pursue steering behavior.
		 *
		 * The pursue steering behavior allows for a realistic movement of trying to "catch" a target by anticipating its movement.
		 * It will estimate where the target will be within the next few seconds and move towards that new position.
		 *
		 * @param target The target Boid object.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @param maxPrediction The maximum prediction time used to estimate where the target will be in the future.
		 * @return Vec2f The steering force used to pursue the target.
		 */
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

		/**
		 * @brief Performs the evade steering behavior.
		 *
		 * The evade steering behavior allows for a realistic movement of trying to "outrun" a target by anticipating its movement.
		 * It will estimate where the target will be within the next few seconds and move away from that new position.
		 *
		 * @param target The target Boid object.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @param maxPrediction The maximum prediction time used to estimate where the target will be in the future.
		 * @return Vec2f The steering force used to evade the target.
		 */
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

		/**
		 * @brief Performs the arrive steering behavior.
		 *
		 * 	The arrive steering behavior is responsible for slowing down the character before it reaches its destination such that it can stop smoothly.
		 *
		 * @param targetPosition The position of the target.
		 * @param slowRadius The radius of the slowing area.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @return Vec2f The steering force used to arrive at the target.
		 */
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

		/**
		 * @brief Performs the wander steering behavior.
		 *
		 * This implementation follows Craig Reynold's proposal, uses a circle defined in front of the character from which the steering force is calculated.
		 * Every frame, a random point is chosen from the outline of this circle which will the new direction the character will need to steer towards.
		 * In order to limit flickering, a `displacementRange` is needed such that it limits the interval from which this random point is chosen.
		 *
		 * @param circleDistance The distance from this Boid object to the circle.
		 * @param circleRadius The radius of the circle.
		 * @param displaceRange The range within which the random point is chosen.
		 * @param maxAcceleration The maximum rate at which the velocity can change per unit of time.
		 * @return Vec2f The steering force used to wander.
		 */
		Vec2f wander(f32 circleDistance, f32 circleRadius, f32 displacementRange, f32 maxAcceleration)
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

			m_theta = m_theta + randomFloat(-displacementRange, displacementRange);

			return steeringForce * maxAcceleration;
		}

		/**
		 * @brief Applies the newly calculated steering force to the Boid's velocity.
		 *
		 * @param steeringForce The steering force obtained from any of the methods performing steering behaviors.
		 */
		void applySteeringForce(Vec2f steeringForce)
		{
			m_velocity = m_velocity + steeringForce;
		}

		/**
		 * @brief Updates the position of the Boid object.
		 *
		 * Needed after the velocity of the Boid object changes.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 */
		void updatePosition(f32 deltaTime)
		{
			if (m_velocity.getMagnitude() > m_maxSpeed)
			{
				m_velocity = (m_velocity / m_velocity.getMagnitude()) * m_maxSpeed;
			}

			if (m_velocity.getMagnitude() <= 0.01f)
			{
				m_velocity = 0.0f;
			}

			m_position = m_position + m_velocity * deltaTime;
		}

	private:
		ui32 m_id;											/*!< The ID of this Boid object. */
		f32 m_lastRotation = 0.0f;							/*!< The last stored rotation of this Boid object. */
		f32 m_theta = PI / 2.0f;							/*!< The angle between the randomly chosen point and the horizontal axis used in the wander behavior. */
		f32 m_minSpeed = 1.0f, m_maxSpeed = 5.0f;			/*!< The minimum and maximum speed of this Boid object. */
		Vec2f m_position, m_velocity;						/*!< The position and velocity of this Boid object. */
	};

	/**
	 * @brief Custom class that is responsible for managing the flocking behavior for a group of Boid objects.
	 *
	 */
	class Flocking
	{
	public:
		/**
		 * @brief Constructs a new Flocking object.
		 *
		 */
		Flocking() {}

		/**
		 * @brief Destroys the Flocking object and its associated boids.
		 *
		 */
		~Flocking()
		{
			for (ui32 i = 0; i < boids.size(); i++)
			{
				delete boids[i];
			}
		}

		/**
		 * @brief Adds a Boid object to the flock (list of Boids).
		 *
		 * @param position The position of the new Boid object.
		 * @param velocity The velocity of the new Boid object.
		 * @param minSpeed The minimum speed of the new Boid object.
		 * @param maxSpeed The maximum speed of the new Boid object.
		 */
		void addBoid(Vec2f position, Vec2f velocity, f32 minSpeed, f32 maxSpeed)
		{
			Boid* boid = new Boid(position, velocity, maxSpeed, boids.size());
			boid->setMinSpeed(minSpeed);
			boids.push_back(boid);
		}

		/**
		 * @brief Sets the separation and perception ranges to be used by each Boid object.
		 *
		 * @param separationRange The range within which a boid avoids colliding with other boids.
		 * @param perceptionRange The range within which a boid considers other boids as part of the flock.
		 */
		void setRanges(f32 separationRange, f32 perceptionRange)
		{
			m_separationRange = separationRange;
			m_perceptionRange = perceptionRange;
		}

		/**
		 * @brief Performs the flocking steering behavior.
		 *
		 * It loops through the list of boids and calls their individual implementation of the algorithm.
		 *
		 * @param deltaTime Elapsed time between last frame and current frame.
		 * @param avoidFactor How strongly boids react to possible collisions.
		 * @param matchingFactor How strongly boids steer to match the average velocity of their neighbours.
		 * @param centeringFactor How strongly boids steer to match the average position of their neighbours.
		 */
		void update(f32 deltaTime, f32 avoidFactor, f32 matchingFactor, f32 centeringFactor)
		{
			for (Boid* boid : boids)
			{
				boid->flocking(deltaTime, m_separationRange, m_perceptionRange, avoidFactor, matchingFactor, centeringFactor, boids);
			}
		}

		std::vector<Boid*> boids;										/*!< The flock of boids. */

	private:
		f32 m_separationRange = 0.0f, m_perceptionRange = 0.0f;			/*!< The separation and perception ranges to be used on all boids of the flock. */
	};
}
