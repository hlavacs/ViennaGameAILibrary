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
		ui32 region_id;
		std::vector<ui32> nodes;
	};

	struct RegionList
	{
		std::vector<Region*> regions;
		ui32 size_x, size_y;

		RegionList(ui32 size_x, ui32 size_y)
			: size_x(size_x)
			, size_y(size_y)
		{
			for (ui32 y = 0; y < size_y; y++)
			{
				for (ui32 x = 0; x < size_x; x++)
				{
					Region* region = new Region();
					region->region_id = x + y * size_x;
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

		ui32 get_region_id(Vec2ui pos)
		{
			return pos.x + pos.y * size_x;
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
		ui32 region_id;
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
			return pos == other.pos && g == other.g && h == other.h && region_id == other.region_id && state == other.state;
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
		NavMesh(ui32 width, ui32 height, ui32 obstacles_percentage)
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
					if (val <= obstacles_percentage)
					{
						node.state = NodeState::OBSTRUCTABLE;
					}

					m_nodes.emplace_back(node);
				}
			}

			m_adj_list.resize(m_nodes.size(), {});
			m_neighbors.resize(m_nodes.size(), {});

			for (ui32 i = 0; i < m_nodes.size(); i++)
			{
				set_neighbors(i);
			}

			create_regions();
		}

		void create_regions()
		{
			f32 region_length_on_x = 9.0f;
			f32 region_length_on_y = 9.0f;

			ui32 num_regions_x = std::ceil(static_cast<f32>(m_width) / region_length_on_x);
			ui32 num_regions_y = std::ceil(static_cast<f32>(m_height) / region_length_on_y);

			m_regions = new RegionList(num_regions_x, num_regions_y);

			for (ui32 y = 0; y < m_height; y++)
			{
				ui32 y_index = std::floor(static_cast<f32>(y) / region_length_on_y);

				for (ui32 x = 0; x < m_width; x++)
				{
					ui32 x_index = std::floor(static_cast<f32>(x) / region_length_on_x);

					ui32 region_id = m_regions->get_region_id(Vec2ui(x_index, y_index));

					NodeData& node = get_node(Vec2ui(x, y));
					node.region_id = region_id;

					m_regions->regions[region_id]->nodes.push_back(get_index(node.pos));
				}
			}
		}

		~NavMesh() {}

		std::vector<Vec2ui> A_star(Vec2ui start_pos, Vec2ui end_pos)
		{
			std::vector<i32> parents(m_nodes.size(), -1);

			for (NodeData& node : m_nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::priority_queue<NodeData, std::vector<NodeData>, NodeDataComparator> open_set;

			ui32 start_node_index = get_index(start_pos);
			ui32 end_node_index = get_index(end_pos);

			open_set.push(m_nodes[start_node_index]);

			m_nodes[start_node_index].g = 0.0f;

			while (open_set.size() > 0)
			{
				ui32 current_index = get_index(open_set.top().pos);

				while (open_set.size() > 0 && current_index == get_index(open_set.top().pos))
				{
					open_set.pop();
				}

				if (current_index == end_node_index)
				{
					std::vector<Vec2ui> shortest_path;

					while (current_index != -1)
					{
						shortest_path.push_back(m_nodes[current_index].pos);
						current_index = parents[current_index];
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				std::vector<ui32> neighbors = m_neighbors[current_index];
				for (ui32 neighbor_index : neighbors)
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

						open_set.push(m_nodes[neighbor_index]);
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
					if (m_nodes[i].region_id == R->region_id)
						continue;

					std::vector<std::vector<Vec2ui>> paths;

					// Calculate distances from N to all nodes in region R
					for (ui32 O_tentative : R->nodes)
					{
						std::vector<Vec2ui> path = A_star(m_nodes[i].pos, m_nodes[O_tentative].pos);

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
					ui32 best_path_index = 0;
					for (ui32 j = 1; j < paths.size(); j++)
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

		std::vector<Vec2ui> get_path(Vec2ui start, Vec2ui target)
		{
			NodeData& start_node = get_node(start);
			NodeData& target_node = get_node(target);

			ui32 start_region_id = start_node.region_id;
			ui32 target_region_id = target_node.region_id;

			if (start_region_id == target_region_id)
			{
				return A_star(start, target);
			}

			// Get precomputed path from start node to target region
			if (m_adj_list[get_index(start_node.pos)].size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}

			ui32 start_node_index = get_index(start_node.pos);

			std::vector<Vec2ui> path_to_region = m_adj_list[start_node_index][target_region_id];

			if (path_to_region.size() == 0)
			{
				std::cout << "No path found to region!" << std::endl;
				return {};
			}

			// Calculate shortest path between node O and target node
			Vec2ui region_start_node = path_to_region[path_to_region.size() - 1];
			if (region_start_node == target)
			{
				// Reached destination
				return path_to_region;
			}

			std::vector<Vec2ui> path_within_region = A_star(region_start_node, target);
			if (path_within_region.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			path_to_region.insert(path_to_region.end(), path_within_region.begin(), path_within_region.end());

			return path_to_region;
		}

		std::vector<Vec2ui> A_star_multithreading(Vec2ui start_pos, Vec2ui end_pos)
		{
			std::vector<NodeData> nodes = m_nodes;

			std::vector<i32> parents(nodes.size(), -1);

			for (NodeData& node : nodes)
			{
				node.g = INFINITY;
				node.h = INFINITY;
			}

			std::priority_queue<NodeData, std::vector<NodeData>, NodeDataComparator> open_set;

			ui32 start_node_index = get_index(start_pos);
			ui32 end_node_index = get_index(end_pos);

			nodes[start_node_index].g = 0.0f;

			open_set.push(nodes[start_node_index]);

			while (open_set.size() > 0)
			{
				ui32 current_index = get_index(open_set.top().pos);

				while (open_set.size() > 0 && current_index == get_index(open_set.top().pos))
				{
					open_set.pop();
				}

				if (current_index == end_node_index)
				{
					std::vector<Vec2ui> shortest_path;

					while (current_index != -1)
					{
						shortest_path.push_back(nodes[current_index].pos);
						current_index = parents[current_index];
					}

					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}

				std::vector<ui32> neighbors = m_neighbors[current_index];
				for (ui32 neighbor_index : neighbors)
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

						open_set.push(nodes[neighbor_index]);
					}
				}
			}

			return {};
		}

		void preprocess_multithreading(ui32 thread_id, ui32 num_threads, std::vector<NodeData> nodes, std::vector<Region*> region_list)
		{
			ui32 start_index = thread_id;

			for (ui32 region_index = start_index; region_index < region_list.size(); region_index += num_threads)
			{
				std::cout << "Preprocessing region " << region_index << "/" << region_list.size() << std::endl;
				for (ui32 i = 0; i < region_list[region_index]->nodes.size(); i++)
				{
					ui32 node_index = region_list[region_index]->nodes[i];

					if (nodes[node_index].state == NodeState::OBSTRUCTABLE)
						continue;

					for (Region* R : region_list)
					{
						if (nodes[node_index].region_id == R->region_id)
							continue;

						std::vector<std::vector<Vec2ui>> paths;

						// Calculate distances from N to all nodes in region R
						for (ui32 O_tentative : R->nodes)
						{
							std::vector<Vec2ui> path = A_star_multithreading(nodes[node_index].pos, nodes[O_tentative].pos);

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
						ui32 best_path_index = 0;
						for (ui32 j = 1; j < paths.size(); j++)
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
			ui32 num_threads = 4;
			std::vector<std::thread> threads;

			ui32 regions_per_thread = m_regions->regions.size() / num_threads;

			for (ui32 i = 0; i < num_threads; i++)
			{
				threads.push_back(std::thread(&NavMesh::preprocess_multithreading, this, i, num_threads, m_nodes, m_regions->regions));
			}

			for (ui32 i = 0; i < threads.size(); i++)
			{
				threads[i].join();
			}
		}

		std::vector<Vec2ui> get_path_multithreading(Vec2ui start, Vec2ui target)
		{
			NodeData& start_node = get_node(start);
			NodeData& target_node = get_node(target);

			ui32 start_region_id = start_node.region_id;
			ui32 target_region_id = target_node.region_id;

			if (start_region_id == target_region_id)
			{
				return A_star_multithreading(start, target);
			}

			// Get precomputed path from start node to target region
			if (m_adj_list[get_index(start_node.pos)].size() == 0)
			{
				std::cout << "Cannot find path!" << std::endl;
				return {};
			}

			ui32 start_node_index = get_index(start_node.pos);
			std::vector<Vec2ui> path_to_region = m_adj_list[start_node_index][target_region_id];

			if (path_to_region.size() == 0)
			{
				std::cout << "No path found to region!" << std::endl;
				return {};
			}

			// Calculate shortest path between node O and target node
			Vec2ui region_start_node = path_to_region[path_to_region.size() - 1];
			if (region_start_node == target)
			{
				// Reached destination
				return path_to_region;
			}

			std::vector<Vec2ui> path_within_region = A_star_multithreading(region_start_node, target);
			if (path_within_region.size() == 0)
			{
				std::cout << "No path found inside region!" << std::endl;
				return {};
			}

			path_to_region.insert(path_to_region.end(), path_within_region.begin(), path_within_region.end());

			return path_to_region;
		}

		void set_obstacle(Vec2ui obstacle_pos)
		{
			ui32 index = get_index(obstacle_pos);
			if (m_nodes[index].state == NodeState::OBSTRUCTABLE)
			{
				m_nodes[index].state = NodeState::WALKABLE;
			}
			else
			{
				m_nodes[index].state = NodeState::OBSTRUCTABLE;
			}
		}

		ui32 get_width()
		{
			return m_width;
		}

		ui32 get_height()
		{
			return m_height;
		}

		Vec2ui get_2D_coordinates(ui32 index)
		{
			ui32 x = index % m_width;
			ui32 y = (index - x) / m_width;
			return Vec2ui(x, y);
		}

		ui32 get_index(Vec2ui pos)
		{
			return pos.x + pos.y * m_width;
		}

		NodeData& get_node(Vec2ui pos)
		{
			return m_nodes[get_index(pos)];
		}

	private:
		void set_neighbors(ui32 node_index)
		{
			NodeData& node = m_nodes[node_index];
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
						m_neighbors[node_index].push_back(get_index(Vec2ui(x + u, y + v)));
					}
				}
			}
		}

		f32 manhattan(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 dist_X = std::abs(static_cast<f32>(v1.x) - static_cast<f32>(v2.x));
			f32 dist_Y = std::abs(static_cast<f32>(v1.y) - static_cast<f32>(v2.y));

			return dist_X + dist_Y;
		}

		f32 euclidean(const Vec2ui& v1, const Vec2ui& v2)
		{
			f32 dist_x = static_cast<f32>(v1.x) - static_cast<f32>(v2.x);
			f32 dist_Y = static_cast<f32>(v1.y) - static_cast<f32>(v2.y);
			f32 dist = std::pow(dist_x, 2) + std::pow(dist_Y, 2);

			return std::sqrt(dist);
		}

	private:
		ui32 m_width, m_height;

		RegionList* m_regions;

		std::vector<NodeData> m_nodes;
		std::vector<std::vector<ui32>> m_neighbors;
		std::vector<std::unordered_map<ui32, std::vector<Vec2ui>>> m_adj_list;
	};

	class State;

	class Transition
	{
	public:
		Transition(State* target, std::function<bool()> callback)
		{
			target_state = target;
			onCheckCallback = callback;
		}

		~Transition() = default;

		std::function<bool()> onCheckCallback;
		State* target_state = nullptr;
	};

	class State
	{
	public:
		std::function<void()> onEnterCallback;
		std::function<void()> onExitCallback;
		std::function<void(float)> onUpdateCallback;

		State() {}

		~State() {
			for (Transition* transition : m_transitions)
			{
				delete transition;
			}
		}

		void add_transition(State* target_state, std::function<bool()> callback)
		{
			m_transitions.push_back(new Transition(target_state, callback));
		}

		std::vector<Transition*> get_transitions()
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

		State* create_state()
		{
			State* state = new State();
			m_states.push_back(state);
			return state;
		}

		void update(float delta)
		{
			if (m_states.size() <= 0)
				return;

			if (!m_current_state)
			{
				m_current_state = m_states[0];

				if (m_current_state->onEnterCallback)
				{
					m_current_state->onEnterCallback();
				}
			}

			Transition* active_transition = nullptr;
			for (Transition* transition : m_current_state->get_transitions())
			{
				if (transition->onCheckCallback && transition->onCheckCallback())
				{
					active_transition = transition;
					break;
				}
			}

			if (active_transition)
			{
				if (m_current_state->onExitCallback)
				{
					m_current_state->onExitCallback();
				}
				m_current_state = active_transition->target_state;

				if (m_current_state->onEnterCallback)
				{
					m_current_state->onEnterCallback();
				}
			}

			if(m_current_state->onUpdateCallback)
			{
				m_current_state->onUpdateCallback(delta);
			}
		}

		State* get_current_state()
		{
			return m_current_state;
		}

	private:
		std::vector<State*> m_states;
		State* m_current_state = nullptr;
	};
}