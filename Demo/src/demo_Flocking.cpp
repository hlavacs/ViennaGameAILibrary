#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"
#include <string>

float randomFloat(float min, float max)
{
	return (rand() / (float)RAND_MAX * max) + min;
}

float minSpeed = 0.5f;
float maxSpeed = 1.5f;

float turnFactor = 0.05f;
float avoidFactor = 0.05f;
float matchingFactor = 0.05f;
float centeringFactor = 0.0005f;

float separationRange = 8.0f;
float perceptionRange = 40.0f;

float margin = 50.0f;

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	VGAIL::Flocking* flock = new VGAIL::Flocking();
	flock->setBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), margin);
	flock->setRanges(separationRange, perceptionRange);

	InitWindow(screenWidth, screenHeight, "Demo for Flocking");
	SetTargetFPS(60);

	for (uint32_t i = 0; i < 200; i++)
	{
		VGAIL::Vec2f pos = VGAIL::Vec2f{ randomFloat(0.0f, static_cast<float>(screenWidth)), randomFloat(0.0f, static_cast<float>(screenHeight)) };
		flock->addBoid(pos, VGAIL::Vec2f{ randomFloat(0.2f, 0.5f), randomFloat(0.2f, 0.5f) });
	}

	while (!WindowShouldClose())
	{
		flock->update(minSpeed, maxSpeed, avoidFactor, matchingFactor, centeringFactor, turnFactor);

		BeginDrawing();
		ClearBackground(WHITE);

		for (VGAIL::Boid* boid : flock->boids)
		{
			DrawCircle(boid->getPosition().x, boid->getPosition().y, 3.0f, BLACK);
		}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}