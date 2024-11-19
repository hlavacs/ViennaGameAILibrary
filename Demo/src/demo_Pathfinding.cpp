/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"
#include <string>

int main(int argc, char* argv[])
{
	constexpr uint32_t screenWidth = 1200;
	constexpr uint32_t screenHeight = 800;

	bool pathFound = false;

	float gridStride = 30.0f;
	float tileSize = 25.0f;

	uint32_t navmeshWidth = static_cast<uint32_t>(screenWidth / gridStride);
	uint32_t navmeshHeight = static_cast<uint32_t>(screenHeight / gridStride);
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 30.0f);

	VGAIL::Vec2ui startPosition = VGAIL::Vec2ui(1, 1);
	Rectangle startRect = { static_cast<float>(startPosition.x * gridStride),
								static_cast<float>(startPosition.y * gridStride),
								tileSize, tileSize };

	VGAIL::Vec2ui targetPosition = VGAIL::Vec2ui(navmeshWidth - 2, navmeshHeight - 2);
	Rectangle targetRect = { static_cast<float>(targetPosition.x * gridStride),
								static_cast<float>(targetPosition.y * gridStride),
								tileSize, tileSize };

	if (navmesh->getNode(startPosition).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(startPosition).state = VGAIL::NodeState::WALKABLE;
	}

	if (navmesh->getNode(targetPosition).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(targetPosition).state = VGAIL::NodeState::WALKABLE;
	}

	std::vector<VGAIL::Vec2ui> path_A;
	std::vector<VGAIL::Vec2ui> path_GP;

	navmesh->preprocess(true, 4);

	InitWindow(screenWidth, screenHeight, "Demo for Pathfinding");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		uint32_t x = static_cast<uint32_t>(std::floor(GetMousePosition().x / gridStride));
		uint32_t y = static_cast<uint32_t>(std::floor(GetMousePosition().y / gridStride));

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			path_A.clear();
			path_GP.clear();

			startPosition = VGAIL::Vec2ui(x, y);
			startRect.x = startPosition.x * gridStride;
			startRect.y = startPosition.y * gridStride;
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			path_A.clear();
			path_GP.clear();

			targetPosition = VGAIL::Vec2ui(x, y);
			targetRect.x = targetPosition.x * gridStride;
			targetRect.y = targetPosition.y * gridStride;
		}

		if (IsKeyPressed(KEY_R))
		{
			path_A = navmesh->findPath(startPosition, targetPosition);
			path_GP = navmesh->findPreprocessedPath(startPosition, targetPosition);
		}

		BeginDrawing();
		ClearBackground(WHITE);

		for (uint32_t y = 0; y < navmeshHeight; y++)
		{
			for (uint32_t x = 0; x < navmeshWidth; x++)
			{
				VGAIL::NodeData& node = navmesh->getNode(VGAIL::Vec2ui(x, y));

				Rectangle nodeRect = { node.pos.x * gridStride, node.pos.y * gridStride, tileSize, tileSize };

				Color color;
				if (node.state == VGAIL::NodeState::OBSTRUCTABLE)
				{
					color = CLITERAL(Color) { 139, 173, 133, 255 };
				}
				else {
					color = CLITERAL(Color) { 244, 247, 243, 255 };

				}

				DrawRectangleRec(nodeRect, color);
			}
		}

		DrawRectangleRec(startRect, CLITERAL(Color) { 191, 64, 64, 255 });
		DrawRectangleRec(targetRect, CLITERAL(Color) { 102, 119, 204, 255 });

		if (path_A.size() > 0)
		{
			for (uint32_t i = 1; i < path_A.size(); i++)
			{
				DrawLineEx(
					Vector2{ startPosition.x * gridStride + (tileSize / 2),
								startPosition.y * gridStride + (tileSize / 2) },
					Vector2{ path_A[0].x * gridStride + (tileSize / 2),
								path_A[0].y * gridStride + (tileSize / 2) },
					3.0f,
					CLITERAL(Color) { 89, 166, 140, 255 });
				DrawLineEx(
					Vector2{ path_A[i - 1].x * gridStride + (tileSize / 2),
								path_A[i - 1].y * gridStride + (tileSize / 2) },
					Vector2{ path_A[i].x * gridStride + (tileSize / 2),
								path_A[i].y * gridStride + (tileSize / 2) },
					3.0f,
					CLITERAL(Color) { 89, 166, 140, 255 });
			}
		}

		if (path_GP.size() > 0)
		{
			DrawLineEx(
				Vector2{ startPosition.x * gridStride + (tileSize / 2),
							startPosition.y * gridStride + (tileSize / 2) },
				Vector2{ path_GP[0].x * gridStride + (tileSize / 2),
							path_GP[0].y * gridStride + (tileSize / 2) },
				3.0f,
				RED);
			for (uint32_t i = 1; i < path_GP.size(); i++)
			{
				DrawLineEx(
					Vector2{ path_GP[i - 1].x * gridStride + (tileSize / 2),
								path_GP[i - 1].y * gridStride + (tileSize / 2) },
					Vector2{ path_GP[i].x * gridStride + (tileSize / 2),
								path_GP[i].y * gridStride + (tileSize / 2) },
					3.0f,
					RED);
			}
		}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}
