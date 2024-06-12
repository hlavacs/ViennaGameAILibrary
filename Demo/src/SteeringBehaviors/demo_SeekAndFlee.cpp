// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

int main(int argc, char* argv[])
{
	float screenWidth = 1200.0f;
	float screenHeight = 800.0f;

	float maxAcceleration = 0.1f;

	float screenMargin = 50.0f;
	float turnFactor = 2.0f;

	float maxSpeed = 3.0f;
	float playerSpeed = 150.0f;

	VGAIL::Vec2f startVel = VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.0f) };

	VGAIL::Vec2f agentSeekingStartPos = VGAIL::Vec2f{ 100.0f, 100.0f };
	VGAIL::Vec2f agentFleeingStartPos = VGAIL::Vec2f{ 900.0f, 600.0f };
	VGAIL::Vec2f playerStartPos = VGAIL::Vec2f{ 500.0f, 500.0f };

	VGAIL::Boid* agentSeeking = new VGAIL::Boid(agentSeekingStartPos, startVel, maxSpeed);
	VGAIL::Boid* agentFleeing = new VGAIL::Boid(agentFleeingStartPos, startVel, maxSpeed);
	VGAIL::Boid* player = new VGAIL::Boid(playerStartPos, startVel, playerSpeed);

	InitWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "Demo for Seek and Flee");
	SetTargetFPS(60);

	Texture2D agentSeekingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");
	Texture2D agentFleeingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/green.png");
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

		agentSeeking->applySteeringForce(agentSeeking->seek(player->getPosition(), maxAcceleration));
		agentSeeking->stayWithinBorders(screenWidth, screenHeight, screenMargin, turnFactor);
		agentSeeking->updatePosition();

		agentFleeing->applySteeringForce(agentFleeing->flee(player->getPosition(), maxAcceleration));
		agentFleeing->stayWithinBorders(screenWidth, screenHeight, screenMargin, turnFactor);
		agentFleeing->updatePosition();

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentSeekingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentSeekingTexture.width), static_cast<float>(agentSeekingTexture.height) },
			{ agentSeeking->getPosition().x, agentSeeking->getPosition().y, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentSeeking->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			agentFleeingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentFleeingTexture.width), static_cast<float>(agentFleeingTexture.height) },
			{ agentFleeing->getPosition().x, agentFleeing->getPosition().y, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentFleeing->getRotationInDegrees(),
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

		DrawText("S", agentSeeking->getPosition().x, agentSeeking->getPosition().y, 10, BLACK);
		DrawText("F", agentFleeing->getPosition().x, agentFleeing->getPosition().y, 10, BLACK);

		EndDrawing();
	}

	UnloadTexture(agentSeekingTexture);
	UnloadTexture(agentFleeingTexture);
	UnloadTexture(playerTexture);
	CloseWindow();

	return 0;
}