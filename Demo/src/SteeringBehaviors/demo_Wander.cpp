/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

constexpr uint32_t screenWidth = 1200;
constexpr uint32_t screenHeight = 900;

int screenMargin = 50;
float tileSize = 50.0f;

void stayWithinBorders(VGAIL::Boid* boid, float turnFactor)
{
	if (boid->getPosition().x * tileSize < screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x + turnFactor, boid->getVelocity().y });
	}
	if (boid->getPosition().x * tileSize > static_cast<float>(screenWidth) - screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x - turnFactor, boid->getVelocity().y });
	}

	if (boid->getPosition().y * tileSize < screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y + turnFactor });

	}
	if (boid->getPosition().y * tileSize > static_cast<float>(screenHeight) - screenMargin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y - turnFactor });
	}
}

int main(int argc, char* argv[])
{
	VGAIL::Vec2f startPos = { 5.0f, 5.0f };
	VGAIL::Vec2f startVel = { VGAIL::randomFloat(0.5f, 1.0f) };

	float maxSpeed = 2.0f;
	float maxAcceleration = 5.0f;

	float circleDistance = 400.0f;
	float circleRadius = 80.0f;
	float displaceRange = 0.1f;

	float turnFactor = 0.5f;

	VGAIL::Boid* agent = new VGAIL::Boid(startPos, startVel, maxSpeed);

	InitWindow(screenWidth, screenHeight, "Demo for Wander");
	SetTargetFPS(60);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");

	while (!WindowShouldClose())
	{
		agent->applySteeringForce(agent->wander(circleDistance, circleRadius, displaceRange, maxAcceleration));
		stayWithinBorders(agent, turnFactor);
		agent->updatePosition(GetFrameTime());

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

		DrawLine(
			agent->getPosition().x * tileSize,
			agent->getPosition().y * tileSize,
			agent->getPosition().x * tileSize + agent->getVelocity().x * tileSize,
			agent->getPosition().y * tileSize + agent->getVelocity().y * tileSize,
			RED
		);

		EndDrawing();
	}

	UnloadTexture(agentTexture);
	CloseWindow();

	return 0;
}