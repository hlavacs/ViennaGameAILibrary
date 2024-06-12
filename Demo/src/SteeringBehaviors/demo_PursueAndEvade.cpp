// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

int main(int argc, char* argv[])
{
	float screenWidth = 1200.0f;
	float screenHeight = 800.0f;

	float maxAcceleration = 0.1f;

	float screenMargin = 50.0f;
	float turnFactor = 2.0f;

	float maxSpeed = 3.0f;
	float playerSpeed = 150.0f;

	VGAIL::Vec2f startVel = VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.5f) };

	VGAIL::Vec2f agentPursuingStartPos = VGAIL::Vec2f{ 100.0f, 100.0f };
	VGAIL::Vec2f agentEvadingStartPos = VGAIL::Vec2f{ 900.0f, 600.0f };
	VGAIL::Vec2f playerStartPos = VGAIL::Vec2f{ 500.0f, 500.0f };

	VGAIL::Boid* agentPursuing = new VGAIL::Boid(agentPursuingStartPos, startVel, maxSpeed);
	VGAIL::Boid* agentEvading = new VGAIL::Boid(agentEvadingStartPos, startVel, maxSpeed);
	VGAIL::Boid* player = new VGAIL::Boid(playerStartPos, startVel, playerSpeed);

	InitWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "Demo for Pursue and Evade");
	SetTargetFPS(60);

	Texture2D agentPursuingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");
	Texture2D agentEvadingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/green.png");
	Texture2D playerTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/yellow.png");

	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		if (IsKeyDown(KEY_LEFT))
		{
			float x = player->getPosition().x - dt * playerSpeed;
			player->setPosition({ x, player->getPosition().y });
		}

		if (IsKeyDown(KEY_RIGHT))
		{
			float x = player->getPosition().x + dt * playerSpeed;
			player->setPosition({ x, player->getPosition().y });
		}

		if (IsKeyDown(KEY_UP))
		{
			float y = player->getPosition().y - dt * playerSpeed;
			player->setPosition({ player->getPosition().x, y });
		}

		if (IsKeyDown(KEY_DOWN))
		{
			float y = player->getPosition().y + dt * playerSpeed;
			player->setPosition({ player->getPosition().x, y });
		}

		agentPursuing->applySteeringForce(agentPursuing->pursue(player, maxAcceleration));
		agentPursuing->stayWithinBorders(screenWidth, screenHeight, screenMargin, turnFactor);
		agentPursuing->updatePosition();

		agentEvading->applySteeringForce(agentEvading->evade(player, maxAcceleration));
		agentEvading->stayWithinBorders(screenWidth, screenHeight, screenMargin, turnFactor);
		agentEvading->updatePosition();

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentPursuingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentPursuingTexture.width), static_cast<float>(agentPursuingTexture.height) },
			{ agentPursuing->getPosition().x, agentPursuing->getPosition().y, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentPursuing->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			agentEvadingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentEvadingTexture.width), static_cast<float>(agentEvadingTexture.height) },
			{ agentEvading->getPosition().x, agentEvading->getPosition().y, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentEvading->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			playerTexture,
			{ 0.0f, 0.0f, static_cast<float>(playerTexture.width), static_cast<float>(playerTexture.height) },
			{ player->getPosition().x, player->getPosition().y, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			player->getRotationInDegrees(),
			WHITE
		);

		DrawText("P", agentPursuing->getPosition().x, agentPursuing->getPosition().y, 10, BLACK);
		DrawText("E", agentEvading->getPosition().x, agentEvading->getPosition().y, 10, BLACK);

		EndDrawing();
	}

	UnloadTexture(agentPursuingTexture);
	UnloadTexture(agentEvadingTexture);
	UnloadTexture(playerTexture);
	CloseWindow();

	return 0;
}