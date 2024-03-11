#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>
#include <unordered_map>

namespace VGAIL
{
	typedef uint32_t u32;
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

	struct Node
	{
		Vec2i pos;
		f32 g, h;
		Node* parent;
		NodeState state;
		std::vector<Node*> neighbors;
		std::unordered_map<u32, std::vector<Vec2i>> adj_list;

		u32 region_id;

		Node(Vec2i pos)
			: pos(pos)
			, g(INFINITY)
			, h(INFINITY)
			, parent(nullptr)
			, state(NodeState::WALKABLE)
		{}

		f32 f() const
		{
			return g + h;
		}
	};

	struct Region
	{
		u32 region_id;
		std::vector<Node*> nodes;
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
					Node* node = new Node(Vec2i{ x, y });
					m_nodes.push_back(node);

					auto val = distribution(rng);
					if (val <= 45)
						node->state = NodeState::OBSTRUCTABLE;
				}
			}

			for (u32 i = 0; i < m_nodes.size(); i++)
			{
				set_neighbors(m_nodes[i]);
			}
		}

		// NavMesh(const std::string& filepath)
		// {
		// 	std::ifstream stream(filepath);
		// 	std::string navmesh_data;

		// 	if (stream.is_open())
		// 	{
		// 		std::string line;
		// 		getline(stream, line);
		// 		m_width = std::stoi(line);

		// 		getline(stream, line);
		// 		m_height = std::stoi(line);

		// 		stream >> navmesh_data;
		// 	}
		// 	else
		// 	{
		// 		std::cout << "Could not read file " << filepath << std::endl;
		// 		return;
		// 	}

		// 	for (u32 y = 0; y < m_height; y++)
		// 	{

		// 		for (u32 x = 0; x < m_width; x++)
		// 		{
		// 			Node* node = new Node(Vec2i{x, y});
		// 			m_nodes.push_back(node);

		// 			char state = navmesh_data[get_index(Vec2i{x, y})];
		// 			if (state == 'w')
		// 			{
		// 				node->state = NodeState::WALKABLE;
		// 			}
		// 			else
		// 			{
		// 				node->state = NodeState::OBSTRUCTABLE;
		// 			}
		// 		}
		// 	}

		// 	for (u32 i = 0; i < m_nodes.size(); i++)
		// 	{
		// 		set_neighbors(m_nodes[i]);
		// 	}
		// }

		~NavMesh()
		{
			for (u32 i = 0; i < m_width * m_height; i++)
			{
				delete m_nodes[i];
			}
		}

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
			if (m_nodes[index]->state == NodeState::OBSTRUCTABLE)
			{
				m_nodes[index]->state = NodeState::WALKABLE;
			}
			else
			{
				m_nodes[index]->state = NodeState::OBSTRUCTABLE;
			}
		}

		std::vector<Vec2i> A_star(Vec2i start_pos, Vec2i end_pos)
		{
			for (Node* node : m_nodes)
			{
				node->g = INFINITY;
				node->h = INFINITY;
				node->parent = nullptr;
			}

			std::vector<Node*> open_set;

			Node* start_node = get_node(start_pos);
			Node* end_node = get_node(end_pos);

			open_set.push_back(start_node);

			start_node->g = 0.0f;

			while (open_set.size() > 0)
			{
				Node* current = open_set[0];

				for (Node* node : open_set)
				{
					if (node->f() < current->f())
					{
						current = node;
					}
				}

				open_set.erase(std::remove(open_set.begin(), open_set.end(), current));

				if (current == end_node)
				{
					std::vector<Vec2i> shortest_path;

					while (current->parent)
					{
						shortest_path.push_back(current->pos);
						current = current->parent;
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				for (Node* neighbor : current->neighbors)
				{
					if (neighbor->state == NodeState::OBSTRUCTABLE)
						continue;

					f32 tentative_g = current->g + euclidean(neighbor->pos, current->pos);

					if (tentative_g < neighbor->g)
					{
						neighbor->parent = current;
						neighbor->g = tentative_g;
						neighbor->h = euclidean(neighbor->pos, end_node->pos);

						if (std::find(open_set.begin(), open_set.end(), neighbor) == std::end(open_set))
						{
							open_set.push_back(neighbor);
						}
					}
				}
			}

			return {};
		}

		void preprocess()
		{
			f32 region_length_on_x = 3.0f;
			f32 region_length_on_y = 3.0f;

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
					Node* node = get_node(Vec2i(x, y));
					node->region_id = region_id;

					m_regions->regions[region_id]->nodes.push_back(node);
					//std::cout << "Region " << region_id << "-- Node: (" << x << ", " << y << ")"  << std::endl;
				}
			}

			uint32_t count = 0;
			for (Node* N : m_nodes)
			{
				std::cout << "Processing node " << ++count << "/" << m_nodes.size() << std::endl;

				if (N->state == NodeState::OBSTRUCTABLE)
					continue;

				for (Region* R : m_regions->regions)
				{
					if (N->region_id == R->region_id)
						continue;

					std::vector<std::vector<Vec2i>> paths;

					// Calculate distances from N to all nodes in region R
					for (Node* O_tentative : R->nodes)
					{
						std::vector<Vec2i> path = A_star(N->pos, O_tentative->pos);

						if (path.size() > 0)
						{
							paths.push_back(path);
						}
					}

					// No paths found
					if (paths.size() == 0)
					{
						N->adj_list[R->region_id] = {};
						continue;
					}

					// Find most optimal path from N to R
					u32 best_path_index = 0;
					for (u32 i = 1; i < paths.size(); i++)
					{
						if (paths[i].size() < paths[best_path_index].size())
						{
							best_path_index = i;
						}
					}

					// Add best path to R to the node N
					N->adj_list[R->region_id] = paths[best_path_index];
				}
			}
		}

		std::vector<Vec2i> get_path(Vec2i start, Vec2i target)
		{
			Node* start_node = get_node(start);
			Node* target_node = get_node(target);

			u32 start_region_id = start_node->region_id;
			u32 target_region_id = target_node->region_id;

			// Get precomputed path from start node to target region
			if (start_node->adj_list.size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}
			std::vector<Vec2i> path_to_region = start_node->adj_list[target_region_id];

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

		Vec2i get_2D_coordinates(u32 index)
		{
			u32 x = index % m_width;
			u32 y = (index - x) / m_width;
			return Vec2i(x, y);
		}

		void save_to_file(const std::string& filepath)
		{
			std::ofstream stream(filepath);
			stream << m_width << '\n';
			stream << m_height << '\n';
			for (Node* node : m_nodes)
			{
				if (node->state == NodeState::WALKABLE)
					stream << "w";
				else
					stream << "o";
			}
			stream.close();
		}

		u32 get_index(Vec2i pos)
		{
			return pos.x + pos.y * m_width;
		}
		Node* get_node(Vec2i pos)
		{
			return m_nodes[get_index(pos)];
		}
	private:

		void set_neighbors(Node* node)
		{
			f32 x = node->pos.x;
			f32 y = node->pos.y;
			Node* neighbor;

			for (int v = -1; v <= 1; ++v)
			{
				for (int u = -1; u <= 1; ++u)
				{
					if (u == 0 && v == 0) // Current node
						continue;

					if (x + u >= 0 && y + v >= 0 &&
						x + u < m_width && y + v < m_height)
					{
						neighbor = get_node(Vec2i(x + u, y + v));
						node->neighbors.push_back(neighbor);
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
		std::vector<Node*> m_nodes;
		u32 m_width, m_height;

		RegionList* m_regions;
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

		// NavMesh* create_navmesh(const std::string& filepath)
		// {
		// 	m_navmeshes.push_back(new NavMesh(filepath));
		// 	return m_navmeshes[m_navmeshes.size() - 1];
		// }

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