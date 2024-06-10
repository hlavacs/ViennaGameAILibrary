// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

float randomFloat(float min, float max)
{
	return (rand() / (float)RAND_MAX * max) + min;
}

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	float targetX = 0.0f;
	float targetY = 0.0f;

	InitWindow(screenWidth, screenHeight, "Demo for Arrive");
	SetTargetFPS(60);
	
	VGAIL::Boid* agent = new VGAIL::Boid(VGAIL::Vec2f{100.0f, 100.0f}, VGAIL::Vec2f{ randomFloat(1.5f, 2.0f), randomFloat(1.5f, 2.0f)}, 0);
	agent->setMaxSpeed(1.0f);
	agent->setMaxSpeed(3.0f);
	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");

	while (!WindowShouldClose())
	{
		float mouseX = GetMousePosition().x;
		float mouseY = GetMousePosition().y;

		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			targetX = mouseX;
			targetY = mouseY;
		}

		if(targetX > 0.0f && targetY > 0.0f)
		{
			agent->applySteeringForce(agent->arrive(VGAIL::Vec2f{targetX, targetY}, 150.0f, 1.5f));
			agent->updatePosition();
		}

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentTexture, 
			{ 0.0f, 0.0f, static_cast<float>(agentTexture.width), static_cast<float>(agentTexture.height) }, 
			{ agent->getPosition().x, agent->getPosition().y, 40.0f, 40.0f }, 
			Vector2{ 20.0f, 20.0f }, 
			agent->getRotationInDegrees(), 
			WHITE
		);

		DrawCircle(targetX, targetY, 3.0f, GREEN);
		
		EndDrawing();
	}
	UnloadTexture(agentTexture);

	CloseWindow();

	return 0;
}