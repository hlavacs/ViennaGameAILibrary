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

	float minSpeed = 50.0f;
	float maxSpeed = 80.0f;

	float turnFactor = 2.5f;
	float avoidFactor = 0.05f;
	float matchingFactor = 0.1f;
	float centeringFactor = 0.005f;

	float separationRange = 25.0f;
	float perceptionRange = 50.0f;

	VGAIL::Flock* flock = new VGAIL::Flock();
	flock->setRanges(separationRange, perceptionRange);
	int boids = 1000;
	for (uint32_t i = 0; i < boids; i++)
	{
		VGAIL::Vec2f position{VGAIL::randomFloat(0.0f, screenWidth), VGAIL::randomFloat(0.0f, screenHeight)};
		VGAIL::Vec2f velocity{VGAIL::randomFloat(0.5f, 1.0f)};
		flock->addBoid(position, velocity, minSpeed, maxSpeed);
	}

 	InitWindow(screenWidth, screenHeight, "Test for Flocking");
	SetTargetFPS(60);

    Texture2D texture = LoadTexture("Demo/res/bird.png");

	Timer timer("Flocking");
	double total = 0.0;
	double frameCount = 0.0;

	while (!WindowShouldClose())
	{
		if(frameCount < 1000.0)
		{
			timer.start();
			flock->update(GetFrameTime(), avoidFactor, matchingFactor, centeringFactor);
			timer.end();
			total += timer.getDuration();
			frameCount++;
		}
		
		if(frameCount == 1000.0)
		{
			std::cout << "Average time for " << boids << " boids: " << total / 1000.0 << " microseconds" << std::endl;
			frameCount++;
		}

		BeginDrawing();
		ClearBackground(WHITE);

		for (VGAIL::Boid* boid : flock->boids)
		{
			DrawTexturePro(
				texture,
				{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) },
				{ boid->getPosition().x, boid->getPosition().y, 50.0f, 50.0f },
				Vector2{ 25.0f, 25.0f },
				boid->getRotationInDegrees() - 90.f,
				WHITE
			);
		}

		DrawText(TextFormat("CURRENT FPS: %i", GetFPS()), GetScreenWidth() / 2 - 100, 50, 20, BLACK);

		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}