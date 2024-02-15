#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>

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
	};

	struct Vec2f
	{
		float x, y;

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
		float g, h;
		Node* parent;
		NodeState state;
		std::vector<Node*> neighbors;

		Node(Vec2i pos)
			: pos(pos)
			, g(INFINITY)
			, h(INFINITY)
			, parent(nullptr)
			, state(NodeState::WALKABLE)
		{}

		float f() const
		{
			return g + h;
		}
	};

	struct MinHeapComparer
	{
		bool operator()(Node* lhs, Node* rhs) const {
			return lhs->f() > rhs->f();
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
					Node* node = new Node(Vec2i{x, y});
					m_nodes.push_back(node);

					auto val = distribution(rng);
					if (val <= 30)
						node->state = NodeState::OBSTRUCTABLE;
				}
			}
			
			for (u32 i = 0; i < m_nodes.size(); i++)
			{
				set_neighbors(m_nodes[i]);
			}
		}

		NavMesh(const std::string& filepath)
		{
			std::ifstream stream(filepath);
			std::string navmesh_data;

			if (stream.is_open())
			{
				std::string line;
				getline(stream, line);
				m_width = std::stoi(line);

				getline(stream, line);
				m_height = std::stoi(line);

				stream >> navmesh_data;
			}
			else
			{
				std::cout << "Could not read file " << filepath << std::endl;
				return;
			}

			for (u32 y = 0; y < m_height; y++)
			{
				for (u32 x = 0; x < m_width; x++)
				{
					Node* node = new Node(Vec2i{x, y});
					m_nodes.push_back(node);

					char state = navmesh_data[get_index(Vec2i{x, y})];
					if (state == 'w')
					{
						node->state = NodeState::WALKABLE;
					}
					else
					{
						node->state = NodeState::OBSTRUCTABLE;
					}
				}
			}

			for (u32 i = 0; i < m_nodes.size(); i++)
			{
				set_neighbors(m_nodes[i]);
			}
		}

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

		Node* get_node(Vec2i pos)
		{
			return m_nodes[get_index(pos)];
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

		std::vector<Node*> get_shortest_path(Vec2i start_pos, Vec2i end_pos)
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
					std::cout << "Path found!" << std::endl;

					std::vector<Node*> shortest_path;

					while (current)
					{
						shortest_path.push_back(current);
						current = current->parent;
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				for (Node* neighbor : current->neighbors)
				{
					if (neighbor->state == NodeState::OBSTRUCTABLE)
						continue;

					float tentative_g = current->g + euclidean(neighbor->pos, current->pos);

					if (tentative_g < neighbor->g)
					{
						neighbor->parent = current;
						neighbor->g = tentative_g;
						neighbor->h = manhattan(neighbor->pos, end_node->pos);

						if (std::find(open_set.begin(), open_set.end(), neighbor) == std::end(open_set))
						{
							open_set.push_back(neighbor);
						}
					}
				}
			}

			std::cout << "Path not found!" << std::endl;
			return {};
		}

		std::vector<Node*> get_shortest_path_min_heap(Vec2i start_pos, Vec2i end_pos)
		{
			for (Node* node : m_nodes)
			{
				node->g = INFINITY;
				node->h = INFINITY;
				node->parent = nullptr;
			}

			std::vector<Node*> open_set;
			std::make_heap(open_set.begin(), open_set.end(), MinHeapComparer());
			
			Node* start_node = get_node(start_pos);
			Node* end_node = get_node(end_pos);
			
			open_set.push_back(start_node);
			std::push_heap(open_set.begin(), open_set.end(), MinHeapComparer());

			start_node->g = 0.0f;

			while (open_set.size() > 0)
			{
				Node* current = open_set.front();

				std::pop_heap(open_set.begin(), open_set.end(), MinHeapComparer());
				open_set.pop_back();

				if (current == end_node)
				{
					std::cout << "Path found!" << std::endl;

					std::vector<Node*> shortest_path;

					while (current)
					{
						shortest_path.push_back(current);
						current = current->parent;
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				for (Node* neighbor : current->neighbors)
				{
					if (neighbor->state == NodeState::OBSTRUCTABLE)
						continue;

					float tentative_g = current->g + euclidean(neighbor->pos, current->pos);

					if (tentative_g < neighbor->g)
					{
						neighbor->parent = current;
						neighbor->g = tentative_g;
						neighbor->h = manhattan(neighbor->pos, end_node->pos);

						if (std::find(open_set.begin(), open_set.end(), neighbor) == std::end(open_set))
						{
							open_set.push_back(neighbor);
							std::push_heap(open_set.begin(), open_set.end(), MinHeapComparer());
						}
					}
				}
			}

			std::cout << "Path not found!" << std::endl;
			return {};
		}

		Vec2i get_position(u32 index)
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

	private:
		u32 get_index(Vec2i pos)
		{
			return pos.x + pos.y * m_width;
		}

		void set_neighbors(Node* node)
		{
			float x = node->pos.x;
			float y = node->pos.y;
			Node* neighbor;

			// top left
			if (x - 1 >= 0 && y - 1 >= 0 &&
				x - 1 < m_width && y - 1 < m_height)
			{
				neighbor = get_node(Vec2i(x - 1, y - 1));
				node->neighbors.push_back(neighbor);
			}

			// top mid
			if (x >= 0 && y - 1 >= 0 &&
				x < m_width && y - 1 < m_height)
			{
				neighbor = get_node(Vec2i(x, y - 1));
				node->neighbors.push_back(neighbor);
			}

			// top right
			if (x + 1 >= 0 && y - 1 >= 0 &&
				x + 1 < m_width && y - 1 < m_height)
			{
				neighbor = get_node(Vec2i(x + 1, y - 1));
				node->neighbors.push_back(neighbor);
			}

			// mid left
			if (x - 1 >= 0 && y >= 0 &&
				x - 1 < m_width && y < m_height)
			{
				neighbor = get_node(Vec2i(x - 1, y));
				node->neighbors.push_back(neighbor);
			}

			// mid right
			if (x + 1 >= 0 && y >= 0 &&
				x + 1 < m_width && y < m_height)
			{
				neighbor = get_node(Vec2i(x + 1, y));
				node->neighbors.push_back(neighbor);
			}

			// bottom left
			if (x - 1 >= 0 && y + 1 >= 0 &&
				x - 1 < m_width && y + 1 < m_height)
			{
				neighbor = get_node(Vec2i(x - 1, y + 1));
				node->neighbors.push_back(neighbor);
			}

			// bottom mid
			if (x >= 0 && y + 1 >= 0 &&
				x < m_width && y + 1 < m_height)
			{
				neighbor = get_node(Vec2i(x, y + 1));
				node->neighbors.push_back(neighbor);
			}

			// bottom right
			if (x + 1 >= 0 && y + 1 >= 0 &&
				x + 1 < m_width && y + 1 < m_height)
			{
				neighbor = get_node(Vec2i(x + 1, y + 1));
				node->neighbors.push_back(neighbor);
			}
		}

		float manhattan(const Vec2i& v1, const Vec2i& v2)
		{
			float dist_X = std::abs(static_cast<f32>(v1.x) - static_cast<f32>(v2.x));
			float dist_Y = std::abs(static_cast<f32>(v1.y) - static_cast<f32>(v2.y));

			return dist_X + dist_Y;
		}

		float euclidean(const Vec2i& v1, const Vec2i& v2)
		{
			float dist_x = static_cast<f32>(v1.x) - static_cast<f32>(v2.x);
			float dist_Y = static_cast<f32>(v1.y) - static_cast<f32>(v2.y);
			float dist = std::pow(dist_x, 2) + std::pow(dist_Y, 2);

			return std::sqrt(dist);
		}

	private:
		std::vector<Node*> m_nodes;
		u32 m_width, m_height;
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
			return m_navmeshes[m_navmeshes.size()-1];
		}

		NavMesh* create_navmesh(const std::string& filepath)
		{
			m_navmeshes.push_back(new NavMesh(filepath));
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