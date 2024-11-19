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

	float targetX = 0.0f;
	float targetY = 0.0f;

	float maxSpeed = 5.0f;
	float slowRadius = 2.5f;
	float maxAcceleration = 0.5f;
	VGAIL::Vec2f startVel = { 0.0f };

    int entities = 10000;
    std::vector<VGAIL::Boid*> boids;
    for(int i = 0; i < entities; i++)
    {
	    VGAIL::Boid* boid = new VGAIL::Boid(
            VGAIL::Vec2f{ VGAIL::randomFloat(0.0f, screenWidth / tileSize), VGAIL::randomFloat(0.0f, screenHeight / tileSize) },
            startVel, 
			maxSpeed, 
			i);
        boids.push_back(boid);        
    }

 	InitWindow(screenWidth, screenHeight, "Test for Arrive");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/pink.png");

	Timer timer("Arrive");
	double total = 0.0;
	double frameCount = 0.0;

	while (!WindowShouldClose())
	{
		float mouseX = GetMousePosition().x;
		float mouseY = GetMousePosition().y;

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			targetX = mouseX;
			targetY = mouseY;
		}

		if(frameCount < 1000.0)
		{
			timer.start();
            if (targetX > 0.0f && targetY > 0.0f)
            {
                for(int i = 0; i < entities; i++)
			    {
                    boids[i]->applySteeringForce(boids[i]->arrive(VGAIL::Vec2f{ targetX / tileSize, targetY / tileSize }, slowRadius, maxAcceleration));
                    boids[i]->updatePosition(GetFrameTime());
                }
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
        }

		DrawText(TextFormat("CURRENT FPS: %i", GetFPS()), GetScreenWidth() / 2 - 100, 50, 20, BLACK);

        if (targetX > 0.0f && targetY > 0.0f)
		{
			DrawCircle(targetX, targetY, 3.0f, GREEN);
			DrawCircleLines(targetX, targetY, slowRadius * tileSize, RED);
		}

		EndDrawing();
	}

	UnloadTexture(agentTexture);
	CloseWindow();

	return 0;
}