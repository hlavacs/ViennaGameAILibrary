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

	float circleDistance = 400.0f;
	float circleRadius = 80.0f;
	float displaceRange = 0.1f;

	float maxSpeed = 2.0f;
	float maxAcceleration = 5.0f;
	VGAIL::Vec2f startVel = { VGAIL::randomFloat(0.5f, 1.0f) };

	int entities = 10000;
	std::vector<VGAIL::Boid*> boids;
	for(int i = 0; i < entities; i++)
	{
		VGAIL::Vec2f position{VGAIL::randomFloat(0.0f, screenWidth / tileSize), VGAIL::randomFloat(0.0f, screenHeight / tileSize)};
		VGAIL::Boid* boid = new VGAIL::Boid(position, startVel, maxSpeed, i);
		boids.push_back(boid);        
	}

	InitWindow(screenWidth, screenHeight, "Test for Wander and Face");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/purple.png");

	Timer timer("Wander and Face");
	double total = 0.0;
	double frameCount = 0.0;

	while (!WindowShouldClose())
	{
		if(frameCount < 1000.0)
		{
			timer.start();
			for(int i = 0; i < entities; i++)
			{
				/*!<--- Wander --*/
				boids[i]->applySteeringForce(boids[i]->wander(circleDistance, circleRadius, displaceRange, maxAcceleration));
				boids[i]->updatePosition(GetFrameTime());

				/*!<--- Face --*/
				// boids[i]->getRotationInDegrees();
			}
			timer.end();
			total += timer.getDuration();
			frameCount++;
		}
		
		if(frameCount == 1000.0)
		{
			std::cout << "Average time for " << entities << " boids: " << total / 1000.0 << " microseconds" << std::endl;
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

			DrawLine(
				boids[i]->getPosition().x * tileSize,
				boids[i]->getPosition().y * tileSize,
				boids[i]->getPosition().x * tileSize + boids[i]->getVelocity().x * tileSize,
				boids[i]->getPosition().y * tileSize + boids[i]->getVelocity().y * tileSize,
				RED
			);
		}

		DrawText(TextFormat("CURRENT FPS: %i", GetFPS()), GetScreenWidth() / 2 - 100, 50, 20, BLACK);

		EndDrawing();
	}

	UnloadTexture(agentTexture);
	CloseWindow();

	return 0;
}