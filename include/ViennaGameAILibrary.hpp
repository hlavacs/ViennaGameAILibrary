#pragma once

#include <iostream>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>

namespace VGAIL
{
	typedef uint32_t u32;
	typedef uint64_t u64;
	typedef int32_t i32;
	typedef float f32;

	struct Vec2i
	{
		u32 x, y;

		Vec2i(u32 x, u32 y)
			:x(x), y(y) {}

		Vec2i(u32 val)
			: x(val), y(val) {}

		bool operator==(const Vec2i& other)
		{
			return x == other.x && y == other.y;
		}
	};

	std::ostream& operator<<(std::ostream& os, const Vec2i& vec)
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
			x = x / mag;
			y = y / mag;
		}
	};

	enum NodeState
	{
		OBSTRUCTABLE,
		WALKABLE
	};
	
	struct Region
	{
		u32 region_id;
		std::vector<u32> nodes;
	};

	struct RegionList
	{
		std::vector<Region*> regions;
		u32 size_x, size_y;

		RegionList(u32 size_x, u32 size_y)
			: size_x(size_x)
			, size_y(size_y)
		{
			for (u32 y = 0; y < size_y; y++)
			{
				for (u32 x = 0; x < size_x; x++)
				{
					Region* region = new Region();
					region->region_id = x + y * size_x;
					regions.push_back(region);
				}
			}
		}

		~RegionList()
		{
			for (u32 i = 0; i < regions.size(); i++)
			{
				delete regions[i];
			}
		}

		u32 get_region_id(Vec2i pos)
		{
			return pos.x + pos.y * size_x;
		}
	};

	struct NodeData
	{
		Vec2i pos;
		f32 g, h;
		u32 region_id;
		NodeState state;

		f32 f() const
		{
			return g + h;
		}

		NodeData(Vec2i pos)
			: pos(pos)
			, g(INFINITY)
			, h(INFINITY)
			, state(NodeState::WALKABLE)
		{}

		bool operator==(const NodeData& other)
		{
			return pos == other.pos && g == other.g && h == other.h && region_id == other.region_id && state == other.state;
		}
	};

	class NavMesh
	{
	public:
		NavMesh(u32 width, u32 height)
			: m_width(width)
			, m_height(height)
		{
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_int_distribution<std::mt19937::result_type> distribution(0, 100); // distribution in range [0, 1]

			for (u32 y = 0; y < m_height; y++)
			{
				for (u32 x = 0; x < m_width; x++)
				{
					NodeData node(Vec2i{x, y});

					auto val = distribution(rng);
					if (val <= 30)
					{
						node.state = NodeState::OBSTRUCTABLE;
					}

					m_nodes.emplace_back(node);
				}
			}

			m_adj_list.resize(m_nodes.size(), {});
			m_neighbors.resize(m_nodes.size(), {});

			for (u32 i = 0; i < m_nodes.size(); i++)
			{
				set_neighbors(i);
			}

			create_regions();
		}

		void create_regions()
		{
			f32 region_length_on_x = 9.0f;
			f32 region_length_on_y = 9.0f;

			u32 num_regions_x = std::ceil(static_cast<f32>(m_width) / region_length_on_x);
			u32 num_regions_y = std::ceil(static_cast<f32>(m_height) / region_length_on_y);

			m_regions = new RegionList(num_regions_x, num_regions_y);

			for (u32 y = 0; y < m_height; y++)
			{
				u32 y_index = std::floor(static_cast<f32>(y) / region_length_on_y);

				for (u32 x = 0; x < m_width; x++)
				{
					u32 x_index = std::floor(static_cast<f32>(x) / region_length_on_x);

					u32 region_id = m_regions->get_region_id(Vec2i(x_index, y_index));
					
					NodeData& node = get_node(Vec2i(x, y));
					node.region_id = region_id;

					m_regions->regions[region_id]->nodes.push_back(get_index(node.pos));
				}
			}
		}

		~NavMesh() {}

		u32 get_width()
		{
			return m_width;
		}

		u32 get_height()
		{
			return m_height;
		}

		void set_obstacle(Vec2i obstacle_pos)
		{
			u32 index = get_index(obstacle_pos);
			if (m_nodes[index].state == NodeState::OBSTRUCTABLE)
			{
				m_nodes[index].state = NodeState::WALKABLE;
			}
			else
			{
				m_nodes[index].state = NodeState::OBSTRUCTABLE;
			}
		}

		std::vector<Vec2i> A_star(Vec2i start_pos, Vec2i end_pos)
		{
			std::vector<i32> parents(m_nodes.size(), -1);

			for (NodeData& node : m_nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::vector<u32> open_set;

			u32 start_node_index = get_index(start_pos);
			u32 end_node_index = get_index(end_pos);

			open_set.push_back(start_node_index);

			m_nodes[start_node_index].g = 0.0f;

			while (open_set.size() > 0)
			{
				u32 current_index = open_set[0];

				for (u32 node_index : open_set)
				{
					if (m_nodes[node_index].f() < m_nodes[current_index].f())
					{
						current_index = node_index;
					}
				}

				open_set.erase(std::remove(open_set.begin(), open_set.end(), current_index));

				if (current_index == end_node_index)
				{
					std::vector<Vec2i> shortest_path;

					while(current_index != -1)
					{
						shortest_path.push_back(m_nodes[current_index].pos);
						current_index = parents[current_index];
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				std::vector<u32> neighbors = m_neighbors[current_index];
				for (u32 neighbor_index : neighbors)
				{
					NodeData& neighbor = m_nodes[neighbor_index];
					if (neighbor.state == NodeState::OBSTRUCTABLE)
						continue;

					f32 tentative_g = m_nodes[current_index].g + euclidean(neighbor.pos, m_nodes[current_index].pos);

					if (tentative_g < neighbor.g)
					{
						parents[neighbor_index] = get_index(m_nodes[current_index].pos);
						neighbor.g = tentative_g;
						neighbor.h = euclidean(neighbor.pos, m_nodes[end_node_index].pos);

						if (std::find(open_set.begin(), open_set.end(), neighbor_index) == std::end(open_set))
						{
							open_set.push_back(neighbor_index);
						}
					}
				}
			}

			return {};
		}

		void preprocess()
		{
			for (u32 i = 0; i < m_nodes.size(); i++)
			{
				std::cout << "Processing node " << i << "/" << m_nodes.size() - 1 << std::endl;

				if (m_nodes[i].state == NodeState::OBSTRUCTABLE)
					continue;

				for (Region* R : m_regions->regions)
				{
					if (m_nodes[i].region_id == R->region_id)
						continue;

					std::vector<std::vector<Vec2i>> paths;

					// Calculate distances from N to all nodes in region R
					for (u32 O_tentative : R->nodes)
					{
						std::vector<Vec2i> path = A_star(m_nodes[i].pos, m_nodes[O_tentative].pos);

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
					u32 best_path_index = 0;
					for (u32 j = 1; j < paths.size(); j++)
					{
						if (paths[j].size() < paths[best_path_index].size())
						{
							best_path_index = j;
						}
					}

					// Add best path to R to the node N
					m_adj_list[i][R->region_id] = paths[best_path_index];
				}
			}
		}

		std::vector<Vec2i> get_path(Vec2i start, Vec2i target)
		{
			NodeData& start_node = get_node(start);
			NodeData& target_node = get_node(target);

			u32 start_region_id = start_node.region_id;
			u32 target_region_id = target_node.region_id;

			// Get precomputed path from start node to target region
			if (m_adj_list[get_index(start_node.pos)].size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}
			std::vector<Vec2i> path_to_region = m_adj_list[get_index(start_node.pos)][target_region_id];

			if (path_to_region.size() == 0)
			{
				std::cout << "No path found to region!" << std::endl;
				return {};
			}

			// Calculate shortest path between node O and target node
			Vec2i region_start_node = path_to_region[path_to_region.size() - 1];
			if (region_start_node == target)
			{
				// Reached destination
				return path_to_region;
			}

			std::vector<Vec2i> path_within_region = A_star(region_start_node, target);
			if (path_within_region.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			path_to_region.insert(path_to_region.end(), path_within_region.begin(), path_within_region.end());

			return path_to_region;
		}

		std::vector<Vec2i> A_star_multithreading(Vec2i start_pos, Vec2i end_pos)
		{
			std::vector<NodeData> nodes = m_nodes;

			std::vector<i32> parents(nodes.size(), -1);

			for (NodeData& node : nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::vector<u32> open_set;

			u32 start_node_index = get_index(start_pos);
			u32 end_node_index = get_index(end_pos);

			open_set.push_back(start_node_index);

			nodes[start_node_index].g = 0.0f;

			while (open_set.size() > 0)
			{
				u32 current_index = open_set[0];

				for (u32 node_index : open_set)
				{
					if (nodes[node_index].f() < nodes[current_index].f())
					{
						current_index = node_index;
					}
				}

				open_set.erase(std::remove(open_set.begin(), open_set.end(), current_index));

				if (current_index == end_node_index)
				{
					std::vector<Vec2i> shortest_path;

					while(current_index != -1)
					{
						shortest_path.push_back(nodes[current_index].pos);
						current_index = parents[current_index];
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				std::vector<u32> neighbors = m_neighbors[current_index];
				for (u32 neighbor_index : neighbors)
				{
					NodeData& neighbor = nodes[neighbor_index];
					if (neighbor.state == NodeState::OBSTRUCTABLE)
						continue;

					f32 tentative_g = nodes[current_index].g + euclidean(neighbor.pos, nodes[current_index].pos);

					if (tentative_g < neighbor.g)
					{
						parents[neighbor_index] = get_index(nodes[current_index].pos);
						neighbor.g = tentative_g;
						neighbor.h = euclidean(neighbor.pos, nodes[end_node_index].pos);

						if (std::find(open_set.begin(), open_set.end(), neighbor_index) == std::end(open_set))
						{
							open_set.push_back(neighbor_index);
						}
					}
				}
			}

			return {};
		}

		void preprocess_multithreading(u32 thread_id, u32 num_threads, std::vector<NodeData> nodes, std::vector<Region*> region_list)
		{
			u32 start_index = thread_id;

			for(u32 region_index = start_index; region_index < region_list.size(); region_index += num_threads)
			{
				std::cout << "Preprocessing region " << region_index << "/" << region_list.size() << std::endl;
				for(u32 i = 0; i < region_list[region_index]->nodes.size(); i++)
				{
					u32 node_index = region_list[region_index]->nodes[i];
					
					if (nodes[node_index].state == NodeState::OBSTRUCTABLE)
						continue;

					for (Region* R :region_list)
					{
						if (nodes[node_index].region_id == R->region_id)
							continue;

						std::vector<std::vector<Vec2i>> paths;

						// Calculate distances from N to all nodes in region R
						for (u32 O_tentative : R->nodes)
						{
							std::vector<Vec2i> path = A_star_multithreading(nodes[node_index].pos, nodes[O_tentative].pos);

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
						u32 best_path_index = 0;
						for (u32 j = 1; j < paths.size(); j++)
						{
							if (paths[j].size() < paths[best_path_index].size())
							{
								best_path_index = j;
							}
						}

						// Add best path to R to the node N
						m_adj_list[node_index][R->region_id] = paths[best_path_index];
					}
				}
			}
		}
			
		void start_preprocess()
		{
			u32 num_threads = 6;
			std::vector<std::thread> threads;

			u32 regions_per_thread = m_regions->regions.size() / num_threads;

			for(u32 i = 0; i < num_threads; i++)
			{
				threads.push_back(std::thread(&NavMesh::preprocess_multithreading, this, i, num_threads, m_nodes, m_regions->regions));
			}

			for(u32 i = 0; i < threads.size(); i++)
			{
				threads[i].join();
			}
		}

		Vec2i get_2D_coordinates(u32 index)
		{
			u32 x = index % m_width;
			u32 y = (index - x) / m_width;
			return Vec2i(x, y);
		}

		u32 get_index(Vec2i pos)
		{
			return pos.x + pos.y * m_width;
		}

		NodeData& get_node(Vec2i pos)
		{
			return m_nodes[get_index(pos)];
		}

	private:
		void set_neighbors(u32 node_index)
		{
			NodeData& node = m_nodes[node_index];
			f32 x = node.pos.x;
			f32 y = node.pos.y;

			for (int v = -1; v <= 1; ++v)
			{
				for (int u = -1; u <= 1; ++u)
				{
					if (u == 0 && v == 0) // Current node
						continue;

					if (x + u >= 0 && y + v >= 0 &&
						x + u < m_width && y + v < m_height)
					{
						m_neighbors[node_index].push_back(get_index(Vec2i(x + u, y + v)));
					}
				}
			}
		}

		f32 manhattan(const Vec2i& v1, const Vec2i& v2)
		{
			f32 dist_X = std::abs(static_cast<f32>(v1.x) - static_cast<f32>(v2.x));
			f32 dist_Y = std::abs(static_cast<f32>(v1.y) - static_cast<f32>(v2.y));

			return dist_X + dist_Y;
		}

		f32 euclidean(const Vec2i& v1, const Vec2i& v2)
		{
			f32 dist_x = static_cast<f32>(v1.x) - static_cast<f32>(v2.x);
			f32 dist_Y = static_cast<f32>(v1.y) - static_cast<f32>(v2.y);
			f32 dist = std::pow(dist_x, 2) + std::pow(dist_Y, 2);

			return std::sqrt(dist);
		}

	private:
		u32 m_width, m_height;

		RegionList* m_regions;

		std::vector<NodeData> m_nodes;
		std::vector<std::vector<u32>> m_neighbors;
		std::vector<std::unordered_map<u32, std::vector<Vec2i>>> m_adj_list;
	};

	class GameAIManager
	{
	public:
		GameAIManager() {}
		~GameAIManager()
		{
			for (u32 i = 0; i < m_navmeshes.size(); i++)
			{
				delete m_navmeshes[i];
			}
		}

		NavMesh* create_navmesh(u32 width, u32 height)
		{
			m_navmeshes.push_back(new NavMesh(width, height));
			return m_navmeshes[m_navmeshes.size() - 1];
		}

		// To be used when having multiple navmeshes
		void free_navmesh(NavMesh* navmesh)
		{
			std::remove(m_navmeshes.begin(), m_navmeshes.end(), navmesh);
			delete navmesh;
		}

	private:
		std::vector<NavMesh*> m_navmeshes;
	};
}