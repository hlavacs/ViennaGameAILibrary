// Resources
	// https://kenney.nl/assets/shape-characters
	// https://www.youtube.com/watch?v=ujsR2vcJlLk&t

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

int main(int argc, char* argv[])
{
	float screenWidth = 1200.0f;
	float screenHeight = 800.0f;

	VGAIL::Vec2f startPos = { 500.0f, 300.0f };
	VGAIL::Vec2f startVel = { VGAIL::randomFloat(0.5f, 1.0f) };
	float maxSpeed = 2.0f;

	float circleDistance = 100.0f;
	float circleRadius = 250.0f;
	float displaceRange = 0.02f;
	float maxAcceleration = 0.15f;

	float screenMargin = 30.0f;
	float turnFactor = 0.5f;

	VGAIL::Boid* agent = new VGAIL::Boid(startPos, startVel, maxSpeed);

	InitWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "Demo for Wander");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");

	while (!WindowShouldClose())
	{
		agent->applySteeringForce(agent->wander(circleDistance, circleRadius, displaceRange, maxAcceleration));
		agent->stayWithinBorders(screenWidth, screenHeight, screenMargin, turnFactor);
		agent->updatePosition();

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

		EndDrawing();
	}

	UnloadTexture(agentTexture);
	CloseWindow();

	return 0;
}