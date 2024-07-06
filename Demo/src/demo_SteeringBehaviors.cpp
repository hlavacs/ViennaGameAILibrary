#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

#include <sstream>
#include <string>
#include <iomanip>

uint32_t screenWidth = 1500;
uint32_t screenHeight = 960;
float tileSize = 30.0f;

uint32_t screenMargin = 30;
float maxSpeed_dog = 5.0f;
float maxSpeed_snake = 3.5f;
float maxSpeed_chicken = 2.0f;

VGAIL::Vec2f barnPosition = { 15.0f, 15.0f };
float barnSize = 100.0f;
VGAIL::Vec2f barnCenter = { barnPosition.x * tileSize + barnSize / 2.0f, barnPosition.y * tileSize + barnSize / 2.0f };

bool dayTime;
float countdown = 0.0f;

bool snakeSpawned = false;

bool isOutsideTheBorders(VGAIL::Vec2f pos)
{
	if (pos.x * tileSize > screenWidth || pos.x * tileSize < 0.0f || pos.y * tileSize > screenHeight || pos.y * tileSize < 0.0f)
		return true;
	else
		return false;
}

void stayWithinBorders(VGAIL::Boid* boid, float turnFactor)
{
	if (boid->getPosition().x * tileSize < screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x + turnFactor, boid->getVelocity().y });
	}
	if (boid->getPosition().x * tileSize > static_cast<float>(screenWidth) - screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x - turnFactor, boid->getVelocity().y });
	}

	if (boid->getPosition().y * tileSize < screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y + turnFactor });

	}
	if (boid->getPosition().y * tileSize > static_cast<float>(screenHeight) - screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y - turnFactor });
	}
}

void goAroundBarn(VGAIL::Boid* boid)
{
	if (VGAIL::distance(barnCenter, boid->getPosition() * tileSize) <= barnSize - 20.0f)
	{
		VGAIL::Vec2f direction = boid->getPosition() - barnCenter / tileSize;
		direction.normalize();
		boid->setVelocity({ boid->getVelocity().x + direction.x, boid->getVelocity().y + direction.y });
	}
}

class Chicken
{
public:
	Chicken(VGAIL::Vec2f pos, VGAIL::Vec2f vel, float maxSpeed, int id)
	{
		boid = new VGAIL::Boid(pos, vel, maxSpeed, id);
	}

	~Chicken()
	{
		delete boid;
	}

	void update(float dt)
	{
		if (dayTime)
		{
			visible = true;
			if (boid->getVelocity() == 0.0f)
			{
				boid->setVelocity(VGAIL::Vec2f{ VGAIL::randomFloat(0.0f, 0.3f), VGAIL::randomFloat(0.0f, 0.3f) });
			}

			boid->applySteeringForce(boid->wander(500.0f, 100.0f, 0.2f, 5.0f));
			stayWithinBorders(boid, 3.0f);
			goAroundBarn(boid);
			boid->updatePosition(dt);
		}
		else
		{
			float distance = VGAIL::distance(boid->getPosition(), VGAIL::Vec2f{ barnPosition.x + (barnSize / tileSize / 2.0f), barnPosition.y + (barnSize / tileSize) });
			if (distance <= 1.0f)
			{
				visible = false;
				boid->setVelocity(VGAIL::Vec2f{ 0.0f });
				boid->updatePosition(dt);
			}
			else
			{
				VGAIL::Vec2f steeringForce = boid->arrive(VGAIL::Vec2f{ barnPosition.x + (barnSize / tileSize / 2.0f), barnPosition.y + (barnSize / tileSize) }, 3.0f, 0.3f);
				boid->applySteeringForce(steeringForce);
				boid->updatePosition(dt);
			}
		}
	}

	bool visible = true;
	VGAIL::Boid* boid = nullptr;
};

int main(int argc, char* argv[])
{
	uint32_t navmeshWidth = screenWidth / tileSize;
	uint32_t navmeshHeight = screenHeight / tileSize;
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 5.0f);

	VGAIL::Boid* dog = new VGAIL::Boid(VGAIL::Vec2f{ 10.0f, 10.0f },
		VGAIL::Vec2f{ 0.2f, 0.4f },
		maxSpeed_dog, 500);
	VGAIL::Boid* snake = new VGAIL::Boid(VGAIL::Vec2f{ 25.0f },
		VGAIL::Vec2f{ 0.0f },
		maxSpeed_snake);

	std::vector<Chicken*> chickens;
	for (int i = 0; i < 10; i++)
	{
		VGAIL::Vec2f pos = VGAIL::Vec2f{ static_cast<float>(VGAIL::randomInt(0, 50)), static_cast<float>(VGAIL::randomInt(0, 30)) };
		VGAIL::Vec2f vel = VGAIL::Vec2f{ VGAIL::randomFloat(0.0f, 0.3f), VGAIL::randomFloat(0.0f, 0.3f) };
		chickens.push_back(new Chicken(pos, vel, maxSpeed_chicken, i));
	}

	InitWindow(screenWidth, screenHeight, "Demo for Steering Behaviors");
	SetTargetFPS(60);

	Texture2D barnTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/barn.png");
	Texture2D snakeTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/snake.png");
	Texture2D dogTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/dog.png");
	Texture2D chickenTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/chicken.png");
	Texture2D groundTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/ground.png");
	Texture2D grassTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/grass.png");
	Texture2D groundTextureNight = LoadTexture("Demo/res/demo_SteeringBehaviors/ground_night.png");
	Texture2D grassTextureNight = LoadTexture("Demo/res/demo_SteeringBehaviors/grass_night.png");

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		countdown += dt;

		if (countdown >= 24.0f)
		{
			countdown = 0.0f;
		}

		if (countdown < 12.0f)
		{
			dayTime = true;
		}
		else
		{
			dayTime = false;
		}

		if (!snakeSpawned && !dayTime)
		{
			snakeSpawned = true;
			snake->setPosition({ VGAIL::randomInt(0, 50), VGAIL::randomInt(0, 30) });
			snake->setVelocity({ 0.1f });
		}

		if (snakeSpawned)
		{
			dog->applySteeringForce(dog->pursue(snake, 5.0f, 0.1f));

			if (VGAIL::distance(dog->getPosition(), snake->getPosition()) <= 3.0f)
			{
				snake->applySteeringForce(snake->evade(dog, 10.0f, 0.1f));
			}
			else
			{
				Chicken* target = nullptr;
				int32_t index = -1;

				for (uint32_t i = 0; i < chickens.size(); i++)
				{
					if (VGAIL::distance(snake->getPosition(), chickens[i]->boid->getPosition()) <= 6.0f)
					{
						snake->applySteeringForce(snake->seek(chickens[i]->boid->getPosition(), 2.0f));

						if (VGAIL::distance(snake->getPosition(), chickens[i]->boid->getPosition()) <= 0.5f)
						{
							target = chickens[i];
							index = i;
							break;
						}
					}
				}

				if (target && index >= 0)
				{
					chickens.erase(chickens.begin() + index);
					delete target;
				}

				snake->applySteeringForce(snake->wander(500.0f, 100.0f, 0.2f, 2.0f));
			}

			if (VGAIL::distance(dog->getPosition(), snake->getPosition()) <= 1.0f)
			{
				snakeSpawned = false;
			}
		}
		else
		{
			dog->applySteeringForce(dog->wander(500.0f, 100.0f, 0.2f, 5.0f));
		}

		stayWithinBorders(dog, 3.0f);

		dog->updatePosition(dt);
		snake->updatePosition(dt);

		if (isOutsideTheBorders(snake->getPosition()))
		{
			snakeSpawned = false;
		}

		for (Chicken* chicken : chickens)
		{
			chicken->update(dt);
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
					Rectangle grassTextureSrc = { 0.0f, 0.0f, static_cast<float>(grassTexture.width), static_cast<float>(grassTexture.height) };
					Rectangle grassTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };

					if (dayTime)
						DrawTexturePro(grassTexture, grassTextureSrc, grassTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
					else
						DrawTexturePro(grassTextureNight, grassTextureSrc, grassTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
				else
				{
					Rectangle groundTextureSrc = { 0.0f, 0.0f, static_cast<float>(groundTexture.width), static_cast<float>(groundTexture.height) };
					Rectangle groundTextureDest = { node.pos.x * tileSize, node.pos.y * tileSize, tileSize, tileSize };

					if (dayTime)
						DrawTexturePro(groundTexture, groundTextureSrc, groundTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
					else
						DrawTexturePro(groundTextureNight, groundTextureSrc, groundTextureDest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
				}
			}
		}

		DrawTexturePro(
			barnTexture,
			{ 0.0f, 0.0f, static_cast<float>(barnTexture.width), static_cast<float>(barnTexture.height) },
			{ barnPosition.x * tileSize, barnPosition.y * tileSize, barnSize, barnSize },
			{ 0.0f },
			0.0f,
			WHITE
		);

		DrawTexturePro(
			dogTexture,
			{ 0.0f, 0.0f, static_cast<float>(dogTexture.width), static_cast<float>(dogTexture.height) },
			{ dog->getPosition().x * tileSize, dog->getPosition().y * tileSize, 30.0f, 35.0f },
			Vector2{ 15.0f, 17.5f },
			dog->getRotationInDegrees() - 90.0f,
			WHITE);

		for (Chicken* chicken : chickens)
		{
			if (chicken->visible)
			{
				DrawTexturePro(
					chickenTexture,
					{ 0.0f, 0.0f, static_cast<float>(chickenTexture.width), static_cast<float>(chickenTexture.height) },
					{ chicken->boid->getPosition().x * tileSize, chicken->boid->getPosition().y * tileSize, 30.0f, 35.0f },
					Vector2{ 15.0f, 17.5f },
					chicken->boid->getRotationInDegrees() - 90.0f,
					WHITE
				);
			}
		}

		if (snakeSpawned)
		{
			DrawTexturePro(
				snakeTexture,
				{ 0.0f, 0.0f, static_cast<float>(snakeTexture.width), static_cast<float>(snakeTexture.height) },
				{ snake->getPosition().x * tileSize, snake->getPosition().y * tileSize, 30.0f, 35.0f },
				Vector2{ 15.0f, 17.5f },
				snake->getRotationInDegrees() - 90.0f,
				WHITE
			);
		}

		std::stringstream ss;
		if (dayTime)
			ss << "Day. Countdown: " << std::fixed << std::setprecision(1) << countdown << " / 12.0";
		else
			ss << "Night. Countdown: " << std::fixed << std::setprecision(1) << countdown << " / 24.0";
		DrawText(ss.str().c_str(), 10, 10, 20, BLACK);

		ss.str(std::string());
		ss << "Chickens: " << chickens.size();
		DrawText(ss.str().c_str(), 1300, 10, 20, BLACK);

		EndDrawing();
	}

	UnloadTexture(barnTexture);
	UnloadTexture(snakeTexture);
	UnloadTexture(dogTexture);
	UnloadTexture(chickenTexture);
	UnloadTexture(groundTexture);
	UnloadTexture(groundTextureNight);
	UnloadTexture(grassTexture);
	UnloadTexture(grassTextureNight);

	CloseWindow();

	return 0;
}