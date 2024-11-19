/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"
#include "../src/Timer.h"

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 900;
	float tileSize = 50.0f;

	float playerSpeed = 3.0f;
	float maxSpeed = 3.0f;
	float maxAcceleration = 0.1f;
	float maxPrediction = 0.3f;

	VGAIL::Vec2f startVel = VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.0f) };
	VGAIL::Vec2f playerStartPos = VGAIL::Vec2f{ 5.0f, 5.0f };
	VGAIL::Boid* player = new VGAIL::Boid(playerStartPos, startVel, playerSpeed);

	int entities = 10000;
	std::vector<VGAIL::Boid*> boids;
	for(int i = 0; i < entities; i++)
	{
		VGAIL::Vec2f position{VGAIL::randomFloat(0.0f, screenWidth / tileSize), VGAIL::randomFloat(0.0f, screenHeight / tileSize)};
		VGAIL::Boid* boid = new VGAIL::Boid(position, startVel, maxSpeed, i);
		boids.push_back(boid);        
	}

	InitWindow(screenWidth, screenHeight, "Test for Pursue and Evade");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/green.png");
	Texture2D playerTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/yellow.png");

	Timer timer("Pursue and Evade");
	double total = 0.0;
	double frameCount = 0.0;

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		if (IsKeyDown(KEY_LEFT))
		{
			float x = player->getPosition().x - dt * playerSpeed;
			player->setPosition({ x, player->getPosition().y });
		}

		if (IsKeyDown(KEY_RIGHT))
		{
			float x = player->getPosition().x + dt * playerSpeed;
			player->setPosition({ x, player->getPosition().y });
		}

		if (IsKeyDown(KEY_UP))
		{
			float y = player->getPosition().y - dt * playerSpeed;
			player->setPosition({ player->getPosition().x, y });
		}

		if (IsKeyDown(KEY_DOWN))
		{
			float y = player->getPosition().y + dt * playerSpeed;
			player->setPosition({ player->getPosition().x, y });
		}

		if(frameCount < 1000)
		{
			timer.start();
			for(int i = 0; i < entities; i++)
			{
				/*!<--- Pursue --*/
				boids[i]->applySteeringForce(boids[i]->pursue(player, maxAcceleration, maxPrediction));
				/*!<--- Evade --*/
				// boids[i]->applySteeringForce(boids[i]->evade(player, maxAcceleration, maxPrediction));

				boids[i]->updatePosition(dt); 
			}
			timer.end();
			total += timer.getDuration();
			frameCount++;
		}
		
		if(frameCount == 1000)
		{
			std::cout << "Average time for " << entities << " boids: " << total / 1000 << " microseconds" << std::endl;
			frameCount++;
		}

		BeginDrawing();
		ClearBackground(WHITE);

		for(int i = 0; i < entities; i++)
		{
			DrawTexturePro(
				agentTexture,
				{ 0.0f, 0.0f, static_cast<float>(agentTexture.width), static_cast<float>(agentTexture.height) },
				{ boids[i]->getPosition().x * tileSize, boids[i]->getPosition().y * tileSize, 50.0f, 50.0f },
				Vector2{ 25.0f, 25.0f },
				boids[i]->getRotationInDegrees(),
				WHITE
			);
		}

		DrawTexturePro(
			playerTexture,
			{ 0.0f, 0.0f, static_cast<float>(playerTexture.width), static_cast<float>(playerTexture.height) },
			{ player->getPosition().x * tileSize, player->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			player->getRotationInDegrees(),
			WHITE
		);

		DrawText(TextFormat("CURRENT FPS: %i", GetFPS()), GetScreenWidth() / 2 - 100, 50, 20, BLACK);
		EndDrawing();
	}

	UnloadTexture(agentTexture);
	UnloadTexture(playerTexture);
	CloseWindow();

	return 0;
}