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

	InitWindow(screen_width, screen_height, "Demo");
	SetTargetFPS(60);

	bool collision = false;
	bool path_found = false;
	bool calculate_new_path = false;

	float grid_stride = 30.0f;
	float tile_size = 25.0f;
	uint32_t speed = 70;

	Rectangle box_collision = {0};

	VGAIL::GameAIManager* manager = new VGAIL::GameAIManager();

	//VGAIL::NavMesh* navmesh = manager->create_navmesh("Demo/res/navmesh.txt");
	// uint32_t navmesh_width = navmesh->get_width();
	// uint32_t navmesh_height = navmesh->get_height();

	uint32_t navmesh_width = static_cast<uint32_t>(screen_width / grid_stride);
	uint32_t navmesh_height = static_cast<uint32_t>(screen_height / grid_stride);
	VGAIL::NavMesh* navmesh = manager->create_navmesh(navmesh_width, navmesh_height);

    VGAIL::Vec2i agent_position = VGAIL::Vec2i(1, 1);
	Rectangle agent_rect = { static_cast<float>(agent_position.x * grid_stride), 
								static_cast<float>(agent_position.y * grid_stride), 
								tile_size, tile_size };

	VGAIL::Vec2i target_position = VGAIL::Vec2i(navmesh_width - 2, navmesh_height - 2);
	Rectangle target_rect = { static_cast<float>(target_position.x * grid_stride), 
								static_cast<float>(target_position.y * grid_stride), 
								tile_size, tile_size };

	if (navmesh->get_node(target_position)->state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->get_node(target_position)->state = VGAIL::NodeState::WALKABLE;
	}

	std::vector<VGAIL::Node*> path;
	int path_index = 0;

	{
		Timer timer("Get first path");
		path = navmesh->get_shortest_path(agent_position, target_position);
	}

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		uint32_t x = static_cast<uint32_t>(std::floor(GetMousePosition().x / grid_stride));
		uint32_t y = static_cast<uint32_t>(std::floor(GetMousePosition().y / grid_stride));

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			calculate_new_path = true;
			navmesh->set_obstacle(VGAIL::Vec2i(x, y));
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			calculate_new_path = true;

			if (navmesh->get_node(VGAIL::Vec2i(x, y))->state == VGAIL::NodeState::OBSTRUCTABLE)
				navmesh->get_node(VGAIL::Vec2i(x, y))->state = VGAIL::NodeState::WALKABLE;

			uint32_t index = x + y * navmesh_width;
			uint32_t new_x = navmesh->get_position(index).x * grid_stride;
			uint32_t new_y = navmesh->get_position(index).y * grid_stride;

			target_rect.x = new_x;
			target_rect.y = new_y;

			target_position = VGAIL::Vec2i(x, y);
		}

		if (IsKeyDown(KEY_S))
		{
			navmesh->save_to_file("Demo/res/navmesh.txt");
		}

		if (IsKeyDown(KEY_R))
		{
			uint32_t agent_x = static_cast<uint32_t>(std::floor(agent_rect.x / grid_stride));
			uint32_t agent_y = static_cast<uint32_t>(std::floor(agent_rect.y / grid_stride));

			uint32_t index = agent_x + agent_y * navmesh_width;
			uint32_t new_x = navmesh->get_position(index).x * grid_stride;
			uint32_t new_y = navmesh->get_position(index).y * grid_stride;

			agent_rect.x = new_x;
			agent_rect.y = new_y;

			path_index = 0;
			path = navmesh->get_shortest_path(VGAIL::Vec2i(agent_x, agent_y), target_position);
		}

		if (path.size() > 0)
			path_found = true;
		else
			path_found = false;

		if (path_found && path_index != path.size())
		{
			VGAIL::Node* target = path[path_index];
			VGAIL::Vec2f agent_velocity{ agent_rect.x, agent_rect.y };

			VGAIL::Vec2f target_pos{ target->pos.x * grid_stride, target->pos.y * grid_stride };
			VGAIL::Vec2f direction = target_pos - agent_velocity;

			direction.normalize();

			float epsilon = 1.1f;
			if (std::abs(agent_rect.x - target_pos.x) < epsilon &&
				std::abs(agent_rect.y - target_pos.y) < epsilon)
			{
				direction = { 0.0f, 0.0f };
				path_index++;
				if (calculate_new_path)
				{
					calculate_new_path = false;
					path_index = 0;
					VGAIL::Vec2i new_pos = VGAIL::Vec2i(static_cast<uint32_t>(target_pos.x / grid_stride), static_cast<uint32_t>(target_pos.y / grid_stride));

					{
						Timer timer("Recalculate path");
						path = navmesh->get_shortest_path(new_pos, target_position);
					}
				}
			}

			agent_rect.x += direction.x * speed * delta;
			agent_rect.y += direction.y * speed * delta;
		}

		collision = CheckCollisionRecs(target_rect, agent_rect);

		if (collision)
			box_collision = GetCollisionRec(target_rect, agent_rect);
		  
		BeginDrawing();
			ClearBackground(WHITE);

			for (uint32_t y = 0; y < navmesh_height; y++)
			{
				for (uint32_t x = 0; x < navmesh_width; x++)
				{
					VGAIL::Node* node = navmesh->get_node(VGAIL::Vec2i(x, y));

					Rectangle node_rect = { node->pos.x * grid_stride, node->pos.y * grid_stride, tile_size, tile_size };

					Color color;
					if (node->state == VGAIL::NodeState::OBSTRUCTABLE)
					{
						color = BLACK;
					}
					else {
						color = CLITERAL(Color) { 218, 220, 218, 255 };

					}

					DrawRectangleRec(node_rect, color);
				}
			}

			DrawRectangleRec(target_rect, BLUE);
			DrawRectangleRec(agent_rect, RED);

			if (collision)
			{
				DrawRectangleRec(box_collision, PURPLE);			
            }

			if (!path_found) 
			{
				DrawRectangleRec(box_collision, PURPLE);

				const char* text = "NO PATH FOUND!";
				float posX = GetScreenWidth() / 2 - MeasureText(text, 20) / 2;
				float posY = 10;				

				Rectangle rect = { posX - 2, posY, static_cast<float>(MeasureText(text, 20) + 5), 20.0f };
				DrawRectangleRec(rect, ORANGE);
				DrawText(text, posX, posY, 20, BLACK);
			}
			else {
				for (uint32_t i = 1; i < path.size(); i++)
				{
					DrawLineEx(
						Vector2{ path[i - 1]->pos.x * grid_stride + (tile_size / 2),
									path[i - 1]->pos.y * grid_stride + (tile_size / 2) },
						Vector2{ path[i]->pos.x * grid_stride + (tile_size / 2),
									path[i]->pos.y * grid_stride + (tile_size / 2) },
						3.0f, 
						MAGENTA);
				}

				DrawRectangleRec(box_collision, PURPLE);

				const char* text = "PATH FOUND!";
				float posX = GetScreenWidth() / 2 - MeasureText(text, 20) / 2;
				float posY = 10;

				Rectangle rect = { posX-2, posY, static_cast<float>(MeasureText(text, 20) + 5), 20.0f };
				DrawRectangleRec(rect, GREEN);
				DrawText(text, posX, posY, 20, BLACK);
			}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}