// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

int main(int argc, char* argv[])
{
	int screenWidth = 1200;
	int screenHeight = 800;

	float targetX = 0.0f;
	float targetY = 0.0f;

	float tileSize = 50.0f;

	VGAIL::Vec2f startPos = { 2.0f, 2.0f };
	VGAIL::Vec2f startVel = { 0.0f };

	float maxSpeed = 5.0f;
	float slowRadius = 5.0f;
	float maxAcceleration = 0.3f;

	VGAIL::Boid* agent = new VGAIL::Boid(startPos, startVel, maxSpeed);

	InitWindow(screenWidth, screenHeight, "Demo for Arrive");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");

	while (!WindowShouldClose())
	{
		float mouseX = GetMousePosition().x;
		float mouseY = GetMousePosition().y;

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			targetX = mouseX;
			targetY = mouseY;
		}

		if (targetX > 0.0f && targetY > 0.0f)
		{
			agent->applySteeringForce(agent->arrive(VGAIL::Vec2f{ targetX / tileSize, targetY / tileSize }, slowRadius, maxAcceleration));
			agent->updatePosition(GetFrameTime());
		}

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentTexture.width), static_cast<float>(agentTexture.height) },
			{ agent->getPosition().x * tileSize, agent->getPosition().y * tileSize, 40.0f, 40.0f },
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