#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"
#include "Timer.h"

#include <chrono>
#include <string>

int main(int argc, char* argv[])
{
	uint32_t screen_width = 1200;
	uint32_t screen_height = 800;

	bool collision = false;
	bool path_found = false;

	float grid_stride = 30.0f;
	float tile_size = 15.0f;

	Rectangle box_collision = { 0 };

	VGAIL::GameAIManager* manager = new VGAIL::GameAIManager();

	uint32_t navmesh_width = static_cast<uint32_t>(screen_width / grid_stride);
	uint32_t navmesh_height = static_cast<uint32_t>(screen_height / grid_stride);
	VGAIL::NavMesh* navmesh = manager->create_navmesh(navmesh_width, navmesh_height);

	VGAIL::Vec2i start_position = VGAIL::Vec2i(1, 1);
	Rectangle start_rect = { static_cast<float>(start_position.x * grid_stride),
								static_cast<float>(start_position.y * grid_stride),
								tile_size, tile_size };

	VGAIL::Vec2i target_position = VGAIL::Vec2i(navmesh_width - 2, navmesh_height - 2);
	Rectangle target_rect = { static_cast<float>(target_position.x * grid_stride),
								static_cast<float>(target_position.y * grid_stride),
								tile_size, tile_size };

	if (navmesh->get_node(start_position).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->get_node(start_position).state = VGAIL::NodeState::WALKABLE;
	}

	if (navmesh->get_node(target_position).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->get_node(target_position).state = VGAIL::NodeState::WALKABLE;
	}

	std::vector<VGAIL::Vec2i> path;
	int path_index = 0;

	std::vector<VGAIL::Vec2i> path2;

	{
		Timer timer("Preprocess");
		navmesh->start_preprocess();
	}

	InitWindow(screen_width, screen_height, "Demo");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		uint32_t x = static_cast<uint32_t>(std::floor(GetMousePosition().x / grid_stride));
		uint32_t y = static_cast<uint32_t>(std::floor(GetMousePosition().y / grid_stride));

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			path.clear();
			path2.clear();

			start_position = VGAIL::Vec2i(x, y);
			start_rect.x = start_position.x * grid_stride;
			start_rect.y = start_position.y * grid_stride;
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			path.clear();
			path2.clear();

			target_position = VGAIL::Vec2i(x, y);
			target_rect.y = target_position.y * grid_stride;
			target_rect.x = target_position.x * grid_stride;
		}

		if (IsKeyPressed(KEY_R))
		{
			path_index = 0;
			std::cout << "------------------" << std::endl;
			{
				Timer timer("A* time");
				path = navmesh->A_star(start_position, target_position);
			}
			{
				Timer timer("GP time");
				path2 = navmesh->get_path_multithreading(start_position, target_position);
			}
		}

		BeginDrawing();
		ClearBackground(WHITE);

		for (uint32_t y = 0; y < navmesh_height; y++)
		{
			for (uint32_t x = 0; x < navmesh_width; x++)
			{
				VGAIL::NodeData& node = navmesh->get_node(VGAIL::Vec2i(x, y));

				Rectangle node_rect = { node.pos.x * grid_stride, node.pos.y * grid_stride, tile_size, tile_size };

				Color color;
				if (node.state == VGAIL::NodeState::OBSTRUCTABLE)
				{
					color = CLITERAL(Color) { 139, 173, 133, 255 };
				}
				else {
					color = CLITERAL(Color) { 244, 247, 243, 255 };

				}

				DrawRectangleRec(node_rect, color);
			}
		}

		DrawRectangleRec(start_rect, CLITERAL(Color) { 191, 64, 64, 255 });
		DrawRectangleRec(target_rect, CLITERAL(Color) { 102, 119, 204, 255 });

		if (path.size() > 0)
		{
			for (uint32_t i = 1; i < path.size(); i++)
			{
				DrawLineEx(
					Vector2{ start_position.x * grid_stride + (tile_size / 2),
								start_position.y * grid_stride + (tile_size / 2) },
					Vector2{ path[0].x * grid_stride + (tile_size / 2),
								path[0].y * grid_stride + (tile_size / 2) },
					3.0f,
					CLITERAL(Color) { 89, 166, 140, 255 });
				DrawLineEx(
					Vector2{ path[i - 1].x * grid_stride + (tile_size / 2),
								path[i - 1].y * grid_stride + (tile_size / 2) },
					Vector2{ path[i].x * grid_stride + (tile_size / 2),
								path[i].y * grid_stride + (tile_size / 2) },
					3.0f,
					CLITERAL(Color) { 89, 166, 140, 255 });
			}
		}

		if (path2.size() > 0)
		{
			DrawLineEx(
				Vector2{ start_position.x * grid_stride + (tile_size / 2),
							start_position.y * grid_stride + (tile_size / 2) },
				Vector2{ path2[0].x * grid_stride + (tile_size / 2),
							path2[0].y * grid_stride + (tile_size / 2) },
				3.0f,
				RED);
			for (uint32_t i = 1; i < path2.size(); i++)
			{
				DrawLineEx(
					Vector2{ path2[i - 1].x * grid_stride + (tile_size / 2),
								path2[i - 1].y * grid_stride + (tile_size / 2) },
					Vector2{ path2[i].x * grid_stride + (tile_size / 2),
								path2[i].y * grid_stride + (tile_size / 2) },
					3.0f,
					RED);
			}
		}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}