/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "raylib.h"

int screenWidth = 1200;
int screenHeight = 800;
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
	float turnFactor = 2.0f;
	float maxSpeed = 3.0f;
	float playerSpeed = 5.0f;
	float maxAcceleration = 0.1f;

	VGAIL::Vec2f startVel = VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.0f) };

	VGAIL::Vec2f agentSeekingStartPos = VGAIL::Vec2f{ 2.0f, 2.0f };
	VGAIL::Vec2f agentFleeingStartPos = VGAIL::Vec2f{ 4.0f, 8.0f };
	VGAIL::Vec2f playerStartPos = VGAIL::Vec2f{ 5.0f, 5.0f };

	VGAIL::Boid* agentSeeking = new VGAIL::Boid(agentSeekingStartPos, startVel, maxSpeed);
	VGAIL::Boid* agentFleeing = new VGAIL::Boid(agentFleeingStartPos, startVel, maxSpeed);
	VGAIL::Boid* player = new VGAIL::Boid(playerStartPos, startVel, playerSpeed);

	InitWindow(screenWidth, screenHeight, "Demo for Seek and Flee");
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
		stayWithinBorders(agentSeeking, turnFactor);
		agentSeeking->updatePosition(dt);

		agentFleeing->applySteeringForce(agentFleeing->flee(player->getPosition(), maxAcceleration));
		stayWithinBorders(agentFleeing, turnFactor);
		agentFleeing->updatePosition(dt);

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentSeekingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentSeekingTexture.width), static_cast<float>(agentSeekingTexture.height) },
			{ agentSeeking->getPosition().x * tileSize, agentSeeking->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentSeeking->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			agentFleeingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentFleeingTexture.width), static_cast<float>(agentFleeingTexture.height) },
			{ agentFleeing->getPosition().x * tileSize, agentFleeing->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentFleeing->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			playerTexture,
			{ 0.0f, 0.0f, static_cast<float>(playerTexture.width), static_cast<float>(playerTexture.height) },
			{ player->getPosition().x * tileSize, player->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			player->getRotationInDegrees(),
			WHITE
		);

		DrawText("S", agentSeeking->getPosition().x * tileSize, agentSeeking->getPosition().y * tileSize, 10, BLACK);
		DrawText("F", agentFleeing->getPosition().x * tileSize, agentFleeing->getPosition().y * tileSize, 10, BLACK);

		EndDrawing();
	}

	UnloadTexture(agentSeekingTexture);
	UnloadTexture(agentFleeingTexture);
	UnloadTexture(playerTexture);
	CloseWindow();

	return 0;
}