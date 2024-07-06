#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

#include <sstream>
#include <string>

int screenWidth = 1200;
int screenHeight = 900;

float tileSize = 50.0f;
float playerSpeed = 100.0f;

float castleSizeX = 400.0f;
float castleSizeY = 300.0f;

bool entered = false;
float enteringTime = 0.0f;

enum BurglarMode
{
	WAITING,
	WALKING,
	ATTACKING,
	FLEEING,
	ENTERING
};

enum GuardMode
{
	PATROLLING,
	FIGHTING,
	ALERTING,
	RUNNING
};

class Burglar
{
public:
	Burglar(VGAIL::Vec2ui index, uint32_t health, BurglarMode defaultMode)
		: index(index)
		, health(health)
		, mode(defaultMode)
	{
		position.x = static_cast<float>(index.x) * tileSize;
		position.y = static_cast<float>(index.y) * tileSize;
	}

	~Burglar() {}

	VGAIL::Vec2ui index;
	VGAIL::Vec2f position{ 0.0f, 0.0f };

	uint32_t health;
	BurglarMode mode;
};

class Guard
{
public:
	Guard(VGAIL::Vec2ui index, uint32_t health, GuardMode defaultMode)
		: index(index)
		, health(health)
		, mode(defaultMode)
	{
		position.x = static_cast<float>(index.x) * tileSize;
		position.y = static_cast<float>(index.y) * tileSize;
	}

	~Guard() {}

	VGAIL::Vec2ui index;
	VGAIL::Vec2f position{ 0.0f, 0.0f };

	uint32_t health;
	GuardMode mode;
};

class isGuardClose : public VGAIL::DecisionNode
{
public:
	isGuardClose(Burglar* burglar, Guard* guard)
		: burglar(burglar)
		, guard(guard)
	{}

	~isGuardClose() {}

	void makeDecision(float dt) override
	{
		float dist = VGAIL::distance(burglar->position, guard->position);
		if (dist <= tileSize * 3.0f)
		{
			getChild(0).makeDecision(dt);
		}
		else
		{
			getChild(1).makeDecision(dt);
		}
	}

	Burglar* burglar = nullptr;
	Guard* guard = nullptr;
};

class isInDanger : public VGAIL::DecisionNode
{
public:
	isInDanger(Burglar* burglar, Guard* guard)
		: burglar(burglar)
		, guard(guard)
	{}

	~isInDanger() {}

	void makeDecision(float dt) override
	{
		float dist = VGAIL::distance(burglar->position, guard->position);

		if (dist <= tileSize)
		{
			if (burglar->health > 1)
			{
				burglar->mode = BurglarMode::ATTACKING;
				takeDamageTime += dt;
				if (takeDamageTime > takeDamageTimeCooldown)
				{
					burglar->health--;
					takeDamageTime = 0.0f;
				}
			}
			else
			{
				burglar->mode = BurglarMode::FLEEING;
				burglar->position.x = burglar->position.x + dt * playerSpeed;
			}
		}
		else
		{
			burglar->mode = BurglarMode::WAITING;
		}
	}

	Burglar* burglar = nullptr;
	Guard* guard = nullptr;

	float takeDamageTime = 0.0f;
	float takeDamageTimeCooldown = 2.0f;
};

class isCloseToCastle : public VGAIL::DecisionNode
{
public:
	isCloseToCastle(Burglar* burglar, VGAIL::Vec2f castlePosition)
		: burglar(burglar)
	{
		castlePos.x = castlePosition.x * tileSize + castleSizeX / 2.0f;
		castlePos.y = burglar->position.y;
	}

	~isCloseToCastle() {}

	void makeDecision(float dt) override
	{
		float dist = VGAIL::distance(burglar->position, castlePos);

		if (dist >= tileSize / 4.0f)
		{
			if (burglar->position.x > castlePos.x)
			{
				burglar->position.x = burglar->position.x - dt * playerSpeed;
			}
			else
			{
				burglar->position.x = burglar->position.x + dt * playerSpeed;
			}

			burglar->mode = BurglarMode::WALKING;
		}
		else
		{
			burglar->mode = BurglarMode::ENTERING;

			enteringTime += dt;
			if (enteringTime >= 4.0f)
			{
				entered = true;
			}
		}
	}

	Burglar* burglar = nullptr;
	VGAIL::Vec2f castlePos;
};

class isBurglarClose : public VGAIL::DecisionNode
{
public:
	isBurglarClose(Guard* guard, Burglar* burglar)
		: guard(guard)
		, burglar(burglar)
	{}

	~isBurglarClose() {}

	void makeDecision(float dt) override
	{
		float dist = VGAIL::distance(guard->position, burglar->position);

		if (entered)
		{
			guard->mode = GuardMode::ALERTING;
		}
		else
		{
			if (dist >= tileSize)
			{
				guard->mode = GuardMode::PATROLLING;

				timeLeft += dt;
				if (timeLeft > timeLeftCooldown / 2)
				{
					guard->position.x = guard->position.x - dt * playerSpeed;
				}
				else
				{
					guard->position.x = guard->position.x + dt * playerSpeed;
				}

				if (timeLeft > timeLeftCooldown)
				{
					timeLeft = 0.0f;
				}
			}
			else
			{
				if (guard->health > 1)
				{
					guard->mode = GuardMode::FIGHTING;
					takeDamageTime += dt;
					if (takeDamageTime > takeDamageTimeCooldown)
					{
						guard->health--;
						takeDamageTime = 0.0f;
					}
				}
				else
				{
					guard->mode = GuardMode::RUNNING;
					guard->position.x = guard->position.x - dt * playerSpeed;
				}
			}
		}
	}

	Guard* guard = nullptr;
	Burglar* burglar = nullptr;

	float takeDamageTime = 0.0f;
	float takeDamageTimeCooldown = 2.0f;

	float timeLeft = 0.0f;
	float timeLeftCooldown = 8.0f;

};

uint32_t navmeshWidth = static_cast<uint32_t>(screenWidth / tileSize);
uint32_t navmeshHeight = static_cast<uint32_t>((screenHeight - 100) / tileSize);
VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 0.0f);

int main(int argc, char* argv[])
{
	Burglar* burglar = new Burglar(VGAIL::Vec2ui{ 22, 8 }, 3, BurglarMode::WAITING);
	Guard* guard = new Guard(VGAIL::Vec2ui{ 7, 8 }, 3, GuardMode::PATROLLING);

	std::string burglar_currentDecision = "";
	std::string guard_currentDecision = "";

	VGAIL::Vec2f castlePos = { 7, 3 };

	VGAIL::DecisionTree burglar_tree;
	VGAIL::DecisionNode& burglar_root = burglar_tree.createRoot<isGuardClose>(burglar, guard);
	burglar_root.addChild<isInDanger>(burglar, guard);
	burglar_root.addChild<isCloseToCastle>(burglar, castlePos);

	VGAIL::DecisionTree guard_tree;
	VGAIL::DecisionNode& guard_root = guard_tree.createRoot<isBurglarClose>(guard, burglar);

	InitWindow(screenWidth, screenHeight, "Demo for Decision Tree");
	SetTargetFPS(60);

	Font sunnyFont = LoadFont("Demo/res/demo_DecisionTree/SunnySpellsBasic.ttf");
	Font pixelFont = LoadFont("Demo/res/demo_DecisionTree/pixelplay.ttf");

	Texture2D backgroundTexture = LoadTexture("Demo/res/demo_DecisionTree/background.png");
	Texture2D groundTexture = LoadTexture("Demo/res/demo_DecisionTree/ground.png");
	Texture2D gateClosedTexture = LoadTexture("Demo/res/demo_DecisionTree/gate_closed.png");
	Texture2D gateOpenTexture = LoadTexture("Demo/res/demo_DecisionTree/gate_open.png");

	Texture2D burglarWalkingTexture = LoadTexture("Demo/res/demo_DecisionTree/char_walk.png");
	Texture2D burglarWaitingTexture = LoadTexture("Demo/res/demo_DecisionTree/char_idle.png");
	Texture2D burglarFightingTexture = LoadTexture("Demo/res/demo_DecisionTree/char_attack.png");
	Texture2D burglarRunningTexture = LoadTexture("Demo/res/demo_DecisionTree/char_run.png");
	Texture2D burglarEnteringTexture = LoadTexture("Demo/res/demo_DecisionTree/char_enter.png");

	Texture2D guardFightingTexture = LoadTexture("Demo/res/demo_DecisionTree/guard_fight.png");
	Texture2D guardPatrollingTexture = LoadTexture("Demo/res/demo_DecisionTree/guard_walk.png");
	Texture2D guardAlertingTexture = LoadTexture("Demo/res/demo_DecisionTree/guard_alert.png");
	Texture2D guardRunningTexture = LoadTexture("Demo/res/demo_DecisionTree/guard_run.png");

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();
		burglar_tree.update(dt);
		guard_tree.update(dt);

		if (IsKeyDown(KEY_LEFT))
		{
			float x = guard->position.x - dt * playerSpeed;
			guard->position.x = x;
		}

		if (IsKeyDown(KEY_RIGHT))
		{
			float x = guard->position.x + dt * playerSpeed;
			guard->position.x = x;
		}

		BeginDrawing();
		ClearBackground(BLACK);

		for (uint32_t y = 0; y < navmeshHeight; y++)
		{
			for (uint32_t x = 0; x < navmeshWidth; x++)
			{
				VGAIL::NodeData& node = navmesh->getNode(VGAIL::Vec2ui(x, y));

				if (y >= 9)
				{
					Rectangle groundTextureSrc = { 0.0f, 0.0f, static_cast<float>(groundTexture.width), static_cast<float>(groundTexture.height) };
					Rectangle groundTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(groundTexture, groundTextureSrc, groundTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
				else
				{
					Rectangle backgroundTextureSrc = { 0.0f, 0.0f, static_cast<float>(backgroundTexture.width), static_cast<float>(backgroundTexture.height) };
					Rectangle backgroundTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };
					DrawTexturePro(backgroundTexture, backgroundTextureSrc, backgroundTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
			}
		}

		if (entered)
		{
			DrawTexturePro(
				gateOpenTexture,
				{ 0.0f, 0.0f, static_cast<float>(gateOpenTexture.width), static_cast<float>(gateOpenTexture.height) },
				{ castlePos.x * tileSize, castlePos.y * tileSize, castleSizeX, castleSizeY },
				{ 0.0f },
				0.0f,
				WHITE
			);
		}
		else
		{
			DrawTexturePro(
				gateClosedTexture,
				{ 0.0f, 0.0f, static_cast<float>(gateClosedTexture.width), static_cast<float>(gateClosedTexture.height) },
				{ castlePos.x * tileSize, castlePos.y * tileSize, castleSizeX, castleSizeY },
				{ 0.0f },
				0.0f,
				WHITE
			);
		}

		if (!entered)
		{
			switch (burglar->mode)
			{
			case BurglarMode::WALKING:
				DrawTexturePro(
					burglarWalkingTexture,
					{ 0.0f, 0.0f, static_cast<float>(burglarWalkingTexture.width), static_cast<float>(burglarWalkingTexture.height) },
					{ burglar->position.x, burglar->position.y, 40.0f, 50.0f },
					{ 0.0f },
					0.0f,
					WHITE
				);
				burglar_currentDecision = "Walking to the castle";
				break;

			case BurglarMode::WAITING:
				DrawTexturePro(
					burglarWaitingTexture,
					{ 0.0f, 0.0f, static_cast<float>(burglarWaitingTexture.width), static_cast<float>(burglarWaitingTexture.height) },
					{ burglar->position.x, burglar->position.y, 40.0f, 50.0f },
					{ 0.0f },
					0.0f,
					WHITE
				);
				burglar_currentDecision = "Waiting";
				break;

			case BurglarMode::ATTACKING:
				DrawTexturePro(
					burglarFightingTexture,
					{ 0.0f, 0.0f, static_cast<float>(burglarFightingTexture.width), static_cast<float>(burglarFightingTexture.height) },
					{ burglar->position.x, burglar->position.y, 40.0f, 50.0f },
					{ 0.0f },
					0.0f,
					WHITE
				);
				burglar_currentDecision = "Fighting";
				break;

			case BurglarMode::ENTERING:
				DrawTexturePro(
					burglarEnteringTexture,
					{ 0.0f, 0.0f, static_cast<float>(burglarEnteringTexture.width), static_cast<float>(burglarEnteringTexture.height) },
					{ burglar->position.x, burglar->position.y, 40.0f, 50.0f },
					{ 0.0f },
					0.0f,
					WHITE
				);
				burglar_currentDecision = "Entering the castle";
				break;

			case BurglarMode::FLEEING:
				DrawTexturePro(
					burglarRunningTexture,
					{ 0.0f, 0.0f, static_cast<float>(burglarRunningTexture.width), static_cast<float>(burglarRunningTexture.height) },
					{ burglar->position.x, burglar->position.y, 40.0f, 50.0f },
					{ 0.0f },
					0.0f,
					WHITE
				);
				burglar_currentDecision = "Running away from the guard";
				break;
			}
		}

		switch (guard->mode)
		{
		case GuardMode::ALERTING:
			DrawTexturePro(
				guardAlertingTexture,
				{ 0.0f, 0.0f, static_cast<float>(guardAlertingTexture.width), static_cast<float>(guardAlertingTexture.height) },
				{ guard->position.x, guard->position.y, 40.0f, 50.0f },
				Vector2{ 0.0f },
				0.0f,
				WHITE
			);
			guard_currentDecision = "Alerting the castle!";
			break;

		case GuardMode::PATROLLING:
			DrawTexturePro(
				guardPatrollingTexture,
				{ 0.0f, 0.0f, static_cast<float>(guardPatrollingTexture.width), static_cast<float>(guardPatrollingTexture.height) },
				{ guard->position.x, guard->position.y, 40.0f, 50.0f },
				Vector2{ 0.0f },
				0.0f,
				WHITE
			);
			guard_currentDecision = "Patroling";
			break;

		case GuardMode::FIGHTING:
			DrawTexturePro(
				guardFightingTexture,
				{ 0.0f, 0.0f, static_cast<float>(guardFightingTexture.width), static_cast<float>(guardFightingTexture.height) },
				{ guard->position.x, guard->position.y, 40.0f, 50.0f },
				Vector2{ 0.0f },
				0.0f,
				WHITE
			);
			guard_currentDecision = "Fighting";
			break;

		case GuardMode::RUNNING:
			DrawTexturePro(
				guardRunningTexture,
				{ 0.0f, 0.0f, static_cast<float>(guardRunningTexture.width), static_cast<float>(guardRunningTexture.height) },
				{ guard->position.x, guard->position.y, 40.0f, 50.0f },
				Vector2{ 0.0f },
				0.0f,
				WHITE
			);
			guard_currentDecision = "Running away";
			break;
		}

		std::stringstream ss;
		ss << "Guard health: " << guard->health;
		DrawTextEx(sunnyFont, ss.str().c_str(), Vector2{ 10.0f, 20.0f }, sunnyFont.baseSize, 1.0f, BLACK);

		ss.str(std::string());
		ss << "Burglar health: " << burglar->health;
		DrawTextEx(sunnyFont, ss.str().c_str(), Vector2{ 950.0f, 20.0f }, sunnyFont.baseSize, 1.0f, BLACK);

		if (!entered)
		{
			ss.str(std::string());
			ss << "Burglar decision: " << burglar_currentDecision;
			DrawTextEx(pixelFont, ss.str().c_str(), Vector2{ 10.0f, screenHeight - 85.0f }, sunnyFont.baseSize, 1.0f, WHITE);
		}
		else
		{
			ss.str(std::string());
			ss << "The burglar entered the castle!";
			DrawTextEx(pixelFont, ss.str().c_str(), Vector2{ 10.0f, screenHeight - 85.0f }, sunnyFont.baseSize, 1.0f, WHITE);
		}

		ss.str(std::string());
		ss << "Guard decision: " << guard_currentDecision;
		DrawTextEx(pixelFont, ss.str().c_str(), Vector2{ 10.0f, screenHeight - 50.0f }, sunnyFont.baseSize, 1.0f, WHITE);

		EndDrawing();
	}

	UnloadTexture(backgroundTexture);
	UnloadTexture(groundTexture);
	UnloadTexture(gateClosedTexture);
	UnloadTexture(gateOpenTexture);

	UnloadTexture(burglarWalkingTexture);
	UnloadTexture(burglarWaitingTexture);
	UnloadTexture(burglarRunningTexture);
	UnloadTexture(burglarFightingTexture);
	UnloadTexture(burglarEnteringTexture);

	UnloadTexture(guardAlertingTexture);
	UnloadTexture(guardFightingTexture);
	UnloadTexture(guardPatrollingTexture);
	UnloadTexture(guardRunningTexture);

	UnloadFont(sunnyFont);
	UnloadFont(pixelFont);

	return 0;
}