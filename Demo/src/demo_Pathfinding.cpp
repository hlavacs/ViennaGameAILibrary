/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"
#include <cmath>
#include <sstream>
#include <string>

int main(int argc, char* argv[])
{
	constexpr uint32_t screenWidth = 1200;
	constexpr uint32_t screenHeight = 800;
	// Strip below the map for the instructions and the result of the search.
	constexpr uint32_t panelHeight = 70;

	bool pathFound = false;

	float gridStride = 50.0f;
	float tileSize = 45.0f;

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

	// A cell can be start or target only if it lies inside the map and is not an obstacle.
	auto isFree = [&](uint32_t cellX, uint32_t cellY)
		{
			return cellX < navmeshWidth && cellY < navmeshHeight &&
				navmesh->getNode(VGAIL::Vec2ui(cellX, cellY)).state != VGAIL::NodeState::OBSTRUCTABLE;
		};

	// Runs both searches for the current start and target.
	auto search = [&]()
		{
			path_A = navmesh->findPath(startPosition, targetPosition);
			path_GP = navmesh->findPreprocessedPath(startPosition, targetPosition);
			pathFound = path_A.size() > 0 || path_GP.size() > 0;
		};

	// Length of a path: 1 for a straight step and the square root of 2 for a diagonal step.
	auto pathLength = [](const std::vector<VGAIL::Vec2ui>& path)
		{
			float length = 0.0f;
			for (size_t i = 1; i < path.size(); i++)
			{
				float dx = static_cast<float>(path[i].x) - static_cast<float>(path[i - 1].x);
				float dy = static_cast<float>(path[i].y) - static_cast<float>(path[i - 1].y);
				length += std::sqrt(dx * dx + dy * dy);
			}
			return length;
		};

	// Shown for a short time after a click on an obstacle.
	float hintTime = 0.0f;

	// Search once at the start, so that a path is visible immediately.
	search();

	InitWindow(screenWidth, screenHeight + panelHeight, "Demo for Pathfinding");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		uint32_t x = static_cast<uint32_t>(std::floor(GetMousePosition().x / gridStride));
		uint32_t y = static_cast<uint32_t>(std::floor(GetMousePosition().y / gridStride));

		hintTime -= delta;
		const bool mouseOnMap = GetMousePosition().x >= 0.0f && GetMousePosition().y >= 0.0f && x < navmeshWidth && y < navmeshHeight;

		// A click on a free cell moves the start or the target, and the path is searched again at once.
		// A click on an obstacle changes nothing, because no path can start or end there.
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouseOnMap)
		{
			if (isFree(x, y))
			{
				startPosition = VGAIL::Vec2ui(x, y);
				startRect.x = startPosition.x * gridStride;
				startRect.y = startPosition.y * gridStride;
				search();
			}
			else
			{
				hintTime = 2.5f;
			}
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && mouseOnMap)
		{
			if (isFree(x, y))
			{
				targetPosition = VGAIL::Vec2ui(x, y);
				targetRect.x = targetPosition.x * gridStride;
				targetRect.y = targetPosition.y * gridStride;
				search();
			}
			else
			{
				hintTime = 2.5f;
			}
		}

		if (IsKeyPressed(KEY_R))
		{
			search();
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

		// The A* path is drawn as a wide green line. The path found with preprocessing is drawn on top
		// of it as a thin red line, so that both stay visible where they are the same.
		if (path_A.size() > 0)
		{
			for (uint32_t i = 1; i < path_A.size(); i++)
			{
				DrawLineEx(
					Vector2{ startPosition.x * gridStride + (tileSize / 2),
								startPosition.y * gridStride + (tileSize / 2) },
					Vector2{ path_A[0].x * gridStride + (tileSize / 2),
								path_A[0].y * gridStride + (tileSize / 2) },
					9.0f,
					CLITERAL(Color) { 89, 166, 140, 255 });
				DrawLineEx(
					Vector2{ path_A[i - 1].x * gridStride + (tileSize / 2),
								path_A[i - 1].y * gridStride + (tileSize / 2) },
					Vector2{ path_A[i].x * gridStride + (tileSize / 2),
								path_A[i].y * gridStride + (tileSize / 2) },
					9.0f,
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

		// Panel below the map: what the mouse and the keyboard do, and what the last search found.
		const float panelY = static_cast<float>(screenHeight);
		DrawRectangle(0, static_cast<int>(screenHeight), static_cast<int>(screenWidth), static_cast<int>(panelHeight), CLITERAL(Color) { 40, 44, 52, 255 });
		DrawText("Left click: set start (red)     Right click: set target (blue)     R: search again", 20, static_cast<int>(panelY) + 10, 20, RAYWHITE);

		std::stringstream status;
		Color statusColor = RAYWHITE;
		if (hintTime > 0.0f)
		{
			status << "This cell is an obstacle. Start and target must be on a free (light) cell.";
			statusColor = ORANGE;
		}
		else if (!pathFound)
		{
			status << "No path: obstacles separate the start from the target.";
			statusColor = ORANGE;
		}
		else
		{
			status.setf(std::ios::fixed);
			status.precision(2);
			status << "A* (wide green line): " << (path_A.size() > 0 ? path_A.size() - 1 : 0) << " steps, length " << pathLength(path_A)
				<< "     With preprocessing (thin red line): " << (path_GP.size() > 0 ? path_GP.size() - 1 : 0) << " steps, length " << pathLength(path_GP);
		}
		DrawText(status.str().c_str(), 20, static_cast<int>(panelY) + 38, 20, statusColor);

		EndDrawing();
	}
	CloseWindow();

	return 0;
}
