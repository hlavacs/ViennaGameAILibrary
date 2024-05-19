#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

float randomFloat(float min, float max)
{
	return (rand() / (float)RAND_MAX * max) + min;
}

float speed = 2.0f;
float maxForce = 0.05f;

float separationRadius = 10.0f;
float perceptionRadius = 50.0f;

float margin = 50.0f;

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	VGAIL::Flocking* flock = new VGAIL::Flocking(speed, maxForce, separationRadius, perceptionRadius);
	flock->setBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), margin);

	for (uint32_t i = 0; i < 200; i++)
	{
		flock->addBoid(VGAIL::Vec2f{ randomFloat(0.0f, static_cast<float>(screenWidth)), randomFloat(0.0f, static_cast<float>(screenHeight)) },
			VGAIL::Vec2f{ randomFloat(0.0f, 1.0f), randomFloat(0.0f, 1.0f) });
	}

	InitWindow(screenWidth, screenHeight, "Demo for Flocking");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();
		flock->update();

		BeginDrawing();
		ClearBackground(WHITE);

		for (VGAIL::Boid* boid : flock->boids)
		{
			DrawCircle(boid->getPosition().x, boid->getPosition().y, separationRadius, Color{ 112, 31, 126, 50 });
			DrawCircle(boid->getPosition().x, boid->getPosition().y, perceptionRadius, Color{ 102, 191, 255, 20 });
			DrawCircle(boid->getPosition().x, boid->getPosition().y, 3.5f, BLACK);
		}

		EndDrawing();
	}
	CloseWindow();

	return 0;
}