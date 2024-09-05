/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

int main(int argc, char* argv[])
{
	constexpr uint32_t screenWidth = 1200;
	constexpr uint32_t screenHeight = 900;

	float targetX = 0.0f;
	float targetY = 0.0f;

	float tileSize = 50.0f;

	VGAIL::Vec2f startPos = { 2.0f, 2.0f };
	VGAIL::Vec2f startVel = { 0.0f };

	float maxSpeed = 5.0f;
	float slowRadius = 2.5f;
	float maxAcceleration = 0.5f;

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