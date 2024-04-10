// Resources
	// https://kenney.nl/assets/roguelike-rpg-pack
	// https://kenney.nl/assets/platformer-characters

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"
#include "Timer.h"

#include <sstream>
#include <string>
#include <algorithm>

constexpr uint32_t screenWidth = 1200;
constexpr uint32_t screenHeight = 900;

float tileSize = 50.0f;
float speed = 100.0f;

uint32_t navmeshWidth = static_cast<uint32_t>(screenWidth / tileSize);
uint32_t navmeshHeight = static_cast<uint32_t>(screenHeight / tileSize);
VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 0);

enum NPCMode
{
	Resting,
	Walking,
	Harvesting,
	Eating,
	Stretching,
	Fighting,
	Running
};

std::string currentDecision = "";

class NPC
{
public:
	NPC(VGAIL::Vec2ui index, uint32_t health, uint32_t food)
		: index(index)
		, health(health)
		, food(food)
	{
		position.x = static_cast<float>(index.x) * tileSize;
		position.y = static_cast<float>(index.y) * tileSize;
	}

	~NPC() {}

	Rectangle getRectangle()
	{
		return { position.x, position.y, tileSize, tileSize };
	}

	VGAIL::Vec2ui index;
	VGAIL::Vec2f position{ 0.0f, 0.0f };

	uint32_t health;
	uint32_t food;
	NPCMode mode = NPCMode::Resting;
};

float distance(VGAIL::Vec2f& v1, VGAIL::Vec2f& v2)
{
	float distX = std::abs(v1.x - v2.x);
	float distY = std::abs(v1.y - v2.y);

	return std::sqrt(std::pow(distX, 2) + std::pow(distY, 2));
}

class isEnemyClose : public VGAIL::DecisionNode
{
public:
	isEnemyClose(NPC* npc, VGAIL::Vec2f& enemyPos)
		: npc(npc)
		, enemyPos(enemyPos)
	{}

	~isEnemyClose() {}

	void makeDecision(float dt) override
	{
		float dist = distance(npc->position, enemyPos);
		if (dist <= 120.0f)
		{
			getChild(0).makeDecision(dt);
		}
		else if (dist > 120.0f && dist < 150.0f)
		{
			getChild(1).makeDecision(dt);
		}
		else
		{
			getChild(2).makeDecision(dt);
		}
	}

	NPC* npc = nullptr;
	VGAIL::Vec2f& enemyPos;
};

class HealthCheck : public VGAIL::DecisionNode
{
public:
	HealthCheck(NPC* npc, VGAIL::Vec2f& enemyPos)
		: npc(npc)
		, enemyPos(enemyPos)
	{}

	~HealthCheck() {}

	void makeDecision(float dt) override
	{
		if (npc->health == 1)
		{
			npc->mode = NPCMode::Running;

			float dX = npc->position.x - enemyPos.x;
			float dY = npc->position.y - enemyPos.y;

			float length = std::sqrt(dX * dX + dY * dY);
			dX /= length;
			dY /= length;

			npc->position.x += dX * dt * speed;
			npc->position.y += dY * dt * speed;

			npc->position.x = std::max(0.0f, std::min(npc->position.x, static_cast<float>(screenWidth) - 50.0f));
			npc->position.y = std::max(0.0f, std::min(npc->position.y, static_cast<float>(screenHeight) - 50.0f));
		}
		else
		{
			float dist = distance(npc->position, enemyPos);
			if (dist <= 40.0f)
			{
				npc->mode = NPCMode::Fighting;

				takeDamageTime += dt;
				if (takeDamageTime > takeDamageTimeCooldown)
				{
					npc->health--;
					takeDamageTime = 0.0f;
				}
			}
			else
			{
				npc->mode = NPCMode::Resting;
			}
		}
	}

	NPC* npc = nullptr;
	VGAIL::Vec2f& enemyPos;

	float takeDamageTime = 0.0f;
	float takeDamageTimeCooldown = 3.0f;
};

class FoodCheck : public VGAIL::DecisionNode
{
public:
	FoodCheck(NPC* npc)
		: npc(npc)
	{}

	~FoodCheck() {}

	void makeDecision(float dt) override
	{
		if (npc->food > 0)
		{
			npc->mode = NPCMode::Eating;

			eatingTime += dt;
			if (eatingTime > eatingTimeCooldown)
			{
				npc->food--;
				npc->health++;
				eatingTime = 0.0f;
			}
		}
		else
		{
			npc->mode = NPCMode::Stretching;
		}
	}

	NPC* npc = nullptr;

	float eatingTime = 0.0f;
	float eatingTimeCooldown = 5.0f;
};

class CropsCheck : public VGAIL::DecisionNode
{
public:
	CropsCheck(bool& cropsReady, uint32_t& cropsAmount, NPC* npc)
		: cropsReady(cropsReady)
		, cropsAmount(cropsAmount)
		, npc(npc)
	{}

	~CropsCheck() {}

	void makeDecision(float dt) override
	{
		if (cropsReady)
		{
			VGAIL::Vec2ui npcCurrentIndex = VGAIL::Vec2ui(static_cast<uint32_t>(npc->position.x / tileSize), static_cast<uint32_t>(npc->position.y / tileSize));

			uint32_t minX = 17;
			uint32_t maxX = 23;
			uint32_t newX = rand() % (maxX - minX + 1) + minX;

			uint32_t minY = 0;
			uint32_t maxY = 6;
			uint32_t newY = rand() % (maxY - minY + 1) + minY;

			VGAIL::Vec2ui newPos = VGAIL::Vec2ui(newX, newY);

			harvestTime += dt;
			if (harvestTime > harvestTimeCooldown
				&& npcCurrentIndex.x >= 17 && npcCurrentIndex.x <= 23 && npcCurrentIndex.y <= 6)
			{
				npc->mode = NPCMode::Harvesting;
				npc->food++;
				cropsAmount--;
				harvestTime = 0.0f;
			};

			if (path.size() == 0 || currentPathIndex == path.size())
			{
				path = navmesh->A_Star(npcCurrentIndex, newPos);
				currentPathIndex = 0;
			}

			if (path.size() > 0)
			{
				VGAIL::Vec2ui targetNode = path[currentPathIndex];
				VGAIL::Vec2f target = VGAIL::Vec2f{ static_cast<float>(targetNode.x) * tileSize, static_cast<float>(targetNode.y) * tileSize };

				if (std::abs(npc->position.x - target.x) < 1.0f && std::abs(npc->position.y - target.y) < 1.0f)
				{
					npc->position.x = target.x;
					npc->position.y = target.y;

					if (npcCurrentIndex.x >= 17 && npcCurrentIndex.x <= 23 && npcCurrentIndex.y <= 6)
						npc->mode = NPCMode::Harvesting;
					else
						npc->mode = NPCMode::Walking;

					currentPathIndex++;
					return;
				}

				VGAIL::Vec2f direction = target - npc->position;
				direction.normalize();

				npc->position.x += direction.x * speed * dt;
				npc->position.y += direction.y * speed * dt;
			}
		}
		else
		{
			npc->mode = NPCMode::Resting;
		}
	}

	NPC* npc = nullptr;
	std::vector<VGAIL::Vec2ui> path;
	int32_t currentPathIndex = 0;

	bool& cropsReady;
	uint32_t& cropsAmount;
	float harvestTime = 0.0f;
	float harvestTimeCooldown = 5.0f;
};

int main(int argc, char* argv[])
{
	bool cropsReady = false;
	float cropsGrowingTime = 0.0f;
	float cropsGrowingCooldown = 2.0f;
	uint32_t cropsAmount = 0;

	VGAIL::Vec2ui enemyIndex(9, 9);
	VGAIL::Vec2f enemyPosition(static_cast<float>(enemyIndex.x) * tileSize, static_cast<float>(enemyIndex.y) * tileSize);
	Rectangle enemyRectangle = { enemyPosition.x, enemyPosition.y, tileSize, tileSize };

	if (navmesh->getNode(enemyIndex).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(enemyIndex).state = VGAIL::NodeState::WALKABLE;
	}

	NPC* npc = new NPC(VGAIL::Vec2ui(5, 5), 3, 1);

	if (navmesh->getNode(npc->index).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->getNode(npc->index).state = VGAIL::NodeState::WALKABLE;
	}

	VGAIL::DecisionTree tree;
	VGAIL::DecisionNode& root = tree.createRoot<isEnemyClose>(npc, enemyPosition);
	root.addChild<HealthCheck>(npc, enemyPosition);
	root.addChild<FoodCheck>(npc);
	root.addChild<CropsCheck>(cropsReady, cropsAmount, npc);

	InitWindow(screenWidth, screenHeight, "Demo for Decision Tree");
	SetTargetFPS(60);

	Texture2D grassTexture = LoadTexture("Demo/res/demo_DecisionTree/grass.png");
	Texture2D cropsTexture = LoadTexture("Demo/res/demo_DecisionTree/crops.png");

	Texture2D enemyTexture = LoadTexture("Demo/res/demo_DecisionTree/enemy.png");
	Rectangle enemyTextureSrc = { 0.0f, 0.0f, static_cast<float>(enemyTexture.width), static_cast<float>(enemyTexture.height) };

	Texture2D restingNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_resting.png");
	Rectangle restingNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(restingNPCTexture.width), static_cast<float>(restingNPCTexture.height) };

	Texture2D walkingNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_walking.png");
	Rectangle walkingNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(walkingNPCTexture.width), static_cast<float>(walkingNPCTexture.height) };

	Texture2D harvestNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_harvesting.png");
	Rectangle harvestNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(harvestNPCTexture.width), static_cast<float>(harvestNPCTexture.height) };

	Texture2D eatingNPCtexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_eating.png");
	Rectangle eatingNPCtextureSrc = { 0.0f, 0.0f, static_cast<float>(eatingNPCtexture.width), static_cast<float>(eatingNPCtexture.height) };

	Texture2D stretchingNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_stretching.png");
	Rectangle stretchingNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(stretchingNPCTexture.width), static_cast<float>(stretchingNPCTexture.height) };

	Texture2D fightingNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_fighting.png");
	Rectangle fightingNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(fightingNPCTexture.width), static_cast<float>(fightingNPCTexture.height) };

	Texture2D runningNPCTexture = LoadTexture("Demo/res/demo_DecisionTree/NPC_running.png");
	Rectangle runningNPCTextureSrc = { 0.0f, 0.0f, static_cast<float>(runningNPCTexture.width), static_cast<float>(runningNPCTexture.height) };

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();
		tree.update(delta);

		cropsGrowingTime += delta;
		if (cropsGrowingTime > cropsGrowingCooldown)
		{
			cropsAmount++;
			cropsGrowingTime = 0.0f;
		}

		if (cropsAmount > 0)
		{
			cropsReady = true;
		}
		else
		{
			cropsReady = false;
		}

		if (IsKeyDown(KEY_LEFT) && enemyPosition.x > 5.0f)
		{
			enemyPosition.x -= delta * speed;
		}

		if (IsKeyDown(KEY_RIGHT) && enemyPosition.x < 1150.0f)
		{
			enemyPosition.x += delta * speed;
		}

		enemyRectangle.x = enemyPosition.x;
		enemyIndex.x = static_cast<uint32_t>(enemyPosition.x / tileSize);

		if (IsKeyDown(KEY_UP) && enemyPosition.y > 5.0f)
		{
			enemyPosition.y -= delta * speed;
		}

		if (IsKeyDown(KEY_DOWN) && enemyPosition.y < 850.0f)
		{
			enemyPosition.y += delta * speed;
		}

		enemyRectangle.y = enemyPosition.y;
		enemyIndex.y = static_cast<uint32_t>(enemyPosition.y / tileSize);

		BeginDrawing();
		ClearBackground(WHITE);

		for (uint32_t y = 0; y < navmeshHeight; y++)
		{
			for (uint32_t x = 0; x < navmeshWidth; x++)
			{
				VGAIL::NodeData& node = navmesh->getNode(VGAIL::Vec2ui(x, y));

				if (x > 16 && y < 7)
				{
					Rectangle cropsTextureSrc = { 0.0f, 0.0f, static_cast<float>(cropsTexture.width), static_cast<float>(cropsTexture.height) };
					Rectangle cropsTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(cropsTexture, cropsTextureSrc, cropsTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
				else
				{
					Rectangle grassTextureSrc = { 0.0f, 0.0f, static_cast<float>(grassTexture.width), static_cast<float>(grassTexture.height) };
					Rectangle grassTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(grassTexture, grassTextureSrc, grassTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
			}
		}

		switch (npc->mode)
		{
		case NPCMode::Resting:
			DrawTexturePro(restingNPCTexture, restingNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Resting";
			break;

		case NPCMode::Walking:
			DrawTexturePro(walkingNPCTexture, walkingNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Walking to crops";
			break;

		case NPCMode::Harvesting:
			DrawTexturePro(harvestNPCTexture, harvestNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Harvesting";
			break;

		case NPCMode::Eating:
			DrawTexturePro(eatingNPCtexture, eatingNPCtextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Eating";
			break;

		case NPCMode::Stretching:
			DrawTexturePro(stretchingNPCTexture, stretchingNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Stretching for fight";
			break;

		case NPCMode::Fighting:
			DrawTexturePro(fightingNPCTexture, fightingNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Fighting";
			break;

		case NPCMode::Running:
			DrawTexturePro(runningNPCTexture, runningNPCTextureSrc, npc->getRectangle(), Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
			currentDecision = "Running away";
			break;
		}

		DrawTexturePro(enemyTexture, enemyTextureSrc, enemyRectangle, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

		std::stringstream ss;
		ss << "Health: " << npc->health;
		DrawText(ss.str().c_str(), 10.0f, 10.0f, 20, BLACK);

		ss.str(std::string());
		ss << "Food: " << npc->food;
		DrawText(ss.str().c_str(), 10.0f, 30.0f, 20, BLACK);

		ss.str(std::string());
		ss << "Crops: " << cropsAmount;
		DrawText(ss.str().c_str(), 10.0f, 50.0f, 20, BLACK);

		ss.str(std::string());
		ss << "Decision: " << currentDecision;
		DrawText(ss.str().c_str(), 10.0f, 70.0f, 20, BLACK);

		EndDrawing();
	}

	UnloadTexture(grassTexture);
	UnloadTexture(cropsTexture);
	UnloadTexture(enemyTexture);

	UnloadTexture(restingNPCTexture);
	UnloadTexture(walkingNPCTexture);
	UnloadTexture(harvestNPCTexture);
	UnloadTexture(eatingNPCtexture);
	UnloadTexture(stretchingNPCTexture);
	UnloadTexture(runningNPCTexture);
	UnloadTexture(fightingNPCTexture);

	return 0;
}