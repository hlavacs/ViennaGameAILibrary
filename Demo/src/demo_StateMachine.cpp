/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"
#include "Timer.h"

#include <sstream>
#include <string>

constexpr uint32_t screenWidth = 1200;
constexpr uint32_t screenHeight = 900;

float tileSize = 50.0f;

class Mine
{
public:
	Mine(uint32_t size, VGAIL::Vec2ui pos)
		: size(size)
		, position(pos)
	{}

	~Mine() {}

	Rectangle getRectangle()
	{
		return { static_cast<float>(position.x * tileSize),
				 static_cast<float>(position.y * tileSize),
				 tileSize, tileSize };
	}

	uint32_t size;
	VGAIL::Vec2ui position;
};

int main(int argc, char* argv[])
{
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh("Demo/res/navmesh.txt");
	uint32_t navmeshWidth = navmesh->getWidth();
	uint32_t navmeshHeight = navmesh->getHeight();

	VGAIL::Vec2ui workerPosition = VGAIL::Vec2ui(20, 2);
	Rectangle worker = { static_cast<float>(workerPosition.x * tileSize),
						static_cast<float>(workerPosition.y * tileSize),
						tileSize, tileSize };
	Color workerColor = BLUE;

	if (navmesh->getNode(workerPosition).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(workerPosition).state = VGAIL::NodeState::WALKABLE;
	}

	VGAIL::Vec2ui homePosition = VGAIL::Vec2ui(3, 2);
	Rectangle home = { static_cast<float>(homePosition.x * tileSize),
						static_cast<float>(homePosition.y * tileSize),
						tileSize, tileSize };

	if (navmesh->getNode(homePosition).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(homePosition).state = VGAIL::NodeState::WALKABLE;
	}

	Mine* mine1 = new Mine(3, VGAIL::Vec2ui{ 11, 15 });
	Mine* mine2 = new Mine(10, VGAIL::Vec2ui{ 13,  9 });
	Mine* mine3 = new Mine(6, VGAIL::Vec2ui{ 3, 11 });
	Mine* mine4 = new Mine(21, VGAIL::Vec2ui{ 16,  7 });
	Mine* mine5 = new Mine(18, VGAIL::Vec2ui{ 7,  2 });

	std::vector<Mine*> mines;
	mines.push_back(mine1);
	mines.push_back(mine2);
	mines.push_back(mine3);
	mines.push_back(mine4);
	mines.push_back(mine5);

	for (Mine* mine : mines)
	{
		if (navmesh->getNode(mine->position).state == VGAIL::NodeState::OBSTRUCTABLE)
		{
			navmesh->getNode(mine->position).state = VGAIL::NodeState::WALKABLE;
		}
	}

	Mine* currentMine = nullptr;

	uint32_t homeLoad = 0;
	uint32_t currentLoad = 0;
	uint32_t maxLoadCapacity = 5;
	float loadTime = 0.0f;
	float loadTimeCooldown = 1.0f;
	float workerSpeed = 100.0f;

	std::vector<VGAIL::Vec2ui> path;
	int32_t currentPathIndex = -1;

	InitWindow(screenWidth, screenHeight, "Demo for State Machine");
	SetTargetFPS(60);

	Font font = LoadFontEx("Demo/res/Signika-SemiBold.ttf", 40, 0, 0);

	Texture2D homeTexture = LoadTexture("Demo/res/demo_StateMachine/home.png");
	Rectangle homeTextureSrc = { 0.0f, 0.0f, static_cast<float>(homeTexture.width), static_cast<float>(homeTexture.height) };
	Rectangle homeTextureDest = { homePosition.x * tileSize, homePosition.y * tileSize, tileSize, tileSize };

	Texture2D mineTexture = LoadTexture("Demo/res/demo_StateMachine/mine.png");
	Rectangle mineTextureSrc = { 0.0f, 0.0f, static_cast<float>(mineTexture.width), static_cast<float>(mineTexture.height) };

	Texture2D workerTexture = LoadTexture("Demo/res/demo_StateMachine/worker.png");
	Rectangle workerTextureSrc = { 0.0f, 0.0f, static_cast<float>(workerTexture.width), static_cast<float>(workerTexture.height) };

	Texture2D groundTexture = LoadTexture("Demo/res/demo_StateMachine/ground.png");
	Texture2D obstacleTexture = LoadTexture("Demo/res/demo_StateMachine/obstacle.png");

	VGAIL::StateMachine stateMachine;
	uint32_t currentStateIndex = 0;

	VGAIL::State* dropOffState = stateMachine.createState();
	VGAIL::State* locateMineState = stateMachine.createState();
	VGAIL::State* locateHomeState = stateMachine.createState();
	VGAIL::State* idleState = stateMachine.createState();
	VGAIL::State* collectState = stateMachine.createState();

	dropOffState->addTransition(locateMineState, [&currentLoad]() {
		return currentLoad == 0;
		});

	locateMineState->addTransition(idleState, [&currentMine, &path]() {
		return currentMine->size == 0 || path.size() == 0;
		});

	locateMineState->addTransition(collectState, [&currentPathIndex, &path]() {
		return currentPathIndex == path.size() - 1;
		});

	collectState->addTransition(locateHomeState, [&currentLoad, &maxLoadCapacity, &currentMine]() {
		return currentLoad == maxLoadCapacity || (currentMine->size == 0 && currentLoad > 0);
		});

	locateHomeState->addTransition(dropOffState, [&currentPathIndex, &path]() {
		return currentPathIndex == path.size() - 1;
		});

	dropOffState->onEnterCallback = [&currentStateIndex]() {
		currentStateIndex = 0;
		};

	dropOffState->onUpdateCallback = [&currentStateIndex, &currentLoad, &loadTime, &loadTimeCooldown, &homeLoad](float delta) {
		currentStateIndex = 0;
		if (currentLoad > 0)
		{
			loadTime += delta;
			if (loadTime > loadTimeCooldown)
			{
				currentLoad--;
				homeLoad++;
				loadTime = 0.0f;
			}
		}
		};

	locateHomeState->onEnterCallback = [&currentStateIndex, &path, &navmesh, &homePosition, &worker, &currentPathIndex]() {
		currentStateIndex = 1;

		VGAIL::Vec2ui workerPos = VGAIL::Vec2ui{ static_cast<uint32_t>(worker.x / tileSize), static_cast<uint32_t>(worker.y / tileSize) };
		path = navmesh->findPath(workerPos, homePosition);

		if (path.size() > 0)
		{
			currentPathIndex = 0;
		}
		};

	locateHomeState->onUpdateCallback = [&currentPathIndex, &path, &worker, &workerSpeed](float delta) {
		if (currentPathIndex < path.size() - 1)
		{
			VGAIL::Vec2ui targetNode = path[currentPathIndex];
			VGAIL::Vec2f target = VGAIL::Vec2f{ static_cast<float>(targetNode.x) * tileSize, static_cast<float>(targetNode.y) * tileSize };

			if (std::abs(worker.x - target.x) < 1.0f && std::abs(worker.y - target.y) < 1.0f)
			{
				worker.x = target.x;
				worker.y = target.y;

				currentPathIndex++;
				return;
			}

			VGAIL::Vec2f direction = target - VGAIL::Vec2f{ worker.x, worker.y };
			direction.normalize();

			worker.x += direction.x * delta * workerSpeed;
			worker.y += direction.y * delta * workerSpeed;
		}
		};

	locateHomeState->onExitCallback = [&path, &currentPathIndex]() {
		path.clear();
		currentPathIndex = -1;
		};

	locateMineState->onEnterCallback = [&currentStateIndex, &worker, &mines, &path, &navmesh, &currentMine, & currentPathIndex]() {
		currentStateIndex = 2;

		VGAIL::Vec2ui workerPos = VGAIL::Vec2ui{ static_cast<uint32_t>(worker.x / tileSize), static_cast<uint32_t>(worker.y / tileSize) };

		uint32_t closestMineIndex = 0;
		uint32_t closestDistance = 10000;

		for (uint32_t i = 0; i < mines.size(); i++)
		{
			if (mines[i]->size == 0)
				continue;

			path = navmesh->findPath(workerPos, mines[i]->position);

			if (path.size() > 0)
			{
				if (path.size() < closestDistance)
				{
					closestMineIndex = i;
					closestDistance = path.size();
				}
			}
		}

		currentMine = mines[closestMineIndex];
		currentPathIndex = 0;
		path = navmesh->findPath(workerPos, currentMine->position);
		};

	locateMineState->onUpdateCallback = [&currentPathIndex, &path, &worker, &workerSpeed](float delta) {
		if (currentPathIndex < path.size() - 1)
		{
			VGAIL::Vec2ui targetNode = path[currentPathIndex];
			VGAIL::Vec2f target = VGAIL::Vec2f{ static_cast<float>(targetNode.x) * tileSize, static_cast<float>(targetNode.y) * tileSize };

			if (std::abs(worker.x - static_cast<float>(target.x)) < 1.0f && std::abs(worker.y - static_cast<float>(target.y)) < 1.0f)
			{
				worker.x = target.x;
				worker.y = target.y;

				currentPathIndex++;
				return;
			}

			VGAIL::Vec2f direction = target - VGAIL::Vec2f{ worker.x, worker.y };
			direction.normalize();

			worker.x += direction.x * delta * workerSpeed;
			worker.y += direction.y * delta * workerSpeed;
		}
		};

	locateMineState->onExitCallback = [&path, &currentPathIndex]() {
		path.clear();
		currentPathIndex = -1;
		};

	collectState->onEnterCallback = [&currentStateIndex]() {
		currentStateIndex = 3;
		};

	collectState->onUpdateCallback = [&currentLoad, &maxLoadCapacity, &currentMine, &loadTime, &loadTimeCooldown](float delta) {
		if (currentLoad <= maxLoadCapacity && currentMine->size > 0)
		{
			loadTime += delta;
			if (loadTime > loadTimeCooldown)
			{
				currentLoad++;
				currentMine->size--;
				loadTime = 0.0f;
			}
		}
		};

	idleState->onEnterCallback = [&currentStateIndex]() {
		currentStateIndex = 4;
		};

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();
		stateMachine.update(delta);

		if (IsKeyDown(KEY_S))
		{
			navmesh->saveToFile("Demo/res/navmesh.txt");
		}

		BeginDrawing();
		ClearBackground(WHITE);

		for (uint32_t y = 0; y < navmeshHeight; y++)
		{
			for (uint32_t x = 0; x < navmeshWidth; x++)
			{
				VGAIL::NodeData& node = navmesh->getNode(VGAIL::Vec2ui(x, y));

				if (node.state == VGAIL::NodeState::OBSTRUCTABLE)
				{
					Rectangle obstacleTextureSrc = { 0.0f, 0.0f, static_cast<float>(obstacleTexture.width), static_cast<float>(obstacleTexture.height) };
					Rectangle obstacleTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(obstacleTexture, obstacleTextureSrc, obstacleTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
				else
				{
					Rectangle groundTextureSrc = { 0.0f, 0.0f, static_cast<float>(groundTexture.width), static_cast<float>(groundTexture.height) };
					Rectangle groundTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(groundTexture, groundTextureSrc, groundTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
			}
		}

		DrawTexturePro(homeTexture, homeTextureSrc, homeTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
		for (Mine* mine : mines)
		{
			DrawTexturePro(mineTexture, mineTextureSrc, mine->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
		}
		DrawTexturePro(workerTexture, workerTextureSrc, worker, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

		std::stringstream ss;
		ss << currentLoad;
		DrawTextEx(font, ss.str().c_str(), Vector2{ worker.x - 5.0f, worker.y }, 25, 1.0f, BLACK);

		ss.str(std::string());
		ss << homeLoad;
		DrawTextEx(font, ss.str().c_str(), Vector2{ home.x - 10.0f, home.y }, 25, 1.0f, BLACK);

		for (Mine* mine : mines)
		{
			ss.str(std::string());
			ss << mine->size;
			DrawTextEx(font, ss.str().c_str(), Vector2{ mine->position.x * tileSize - 10.0f, mine->position.y * tileSize }, 25, 1.0f, BLACK);
		}

		std::string currentState = "";
		switch (currentStateIndex)
		{
		case 0:
			currentState = "Dropping off materials";
			break;
		case 1:
			currentState = "Locating home";
			break;
		case 2:
			currentState = "Locating nearest mine";
			break;
		case 3:
			currentState = "Collecting materials";
			break;
		case 4:
			currentState = "Idle";
			break;
		default:
			currentState = "";
		}

		ss.str(std::string());
		ss << "CURRENT STATE -> " << currentState;
		const char* fullText = ss.str().c_str();
		Vector2 textPosition = Vector2{ screenWidth / 2.0f - 300.0f, 50.0f };
		DrawRectangle(textPosition.x - 20.0f, textPosition.y, 750.0f, 40.0f, WHITE);
		DrawTextEx(font, fullText, textPosition, font.baseSize, 3.0f, BLACK);
		
		EndDrawing();
	}

	UnloadTexture(obstacleTexture);
	UnloadTexture(groundTexture);
	UnloadTexture(homeTexture);
	UnloadTexture(mineTexture);
	UnloadTexture(workerTexture);

	UnloadFont(font);

	CloseWindow();

	for (Mine* mine : mines)
	{
		delete mine;
	}

	return 0;
}