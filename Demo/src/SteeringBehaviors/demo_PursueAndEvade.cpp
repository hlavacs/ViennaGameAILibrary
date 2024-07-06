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
	float playerSpeed = 3.0f;
	float maxSpeed = 3.0f;
	float maxAcceleration = 0.1f;
	float maxPrediction = 0.3f;

	VGAIL::Vec2f startVel = VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.5f) };

	VGAIL::Vec2f agentPursuingStartPos = VGAIL::Vec2f{ 4.0f, 4.0f };
	VGAIL::Vec2f agentEvadingStartPos = VGAIL::Vec2f{ 7.0f, 3.0f };
	VGAIL::Vec2f playerStartPos = VGAIL::Vec2f{ 8.0f, 5.0f };

	VGAIL::Boid* agentPursuing = new VGAIL::Boid(agentPursuingStartPos, startVel, maxSpeed);
	VGAIL::Boid* agentEvading = new VGAIL::Boid(agentEvadingStartPos, startVel, maxSpeed);
	VGAIL::Boid* player = new VGAIL::Boid(playerStartPos, startVel, playerSpeed);

	InitWindow(screenWidth, screenHeight, "Demo for Pursue and Evade");
	SetTargetFPS(60);

	Texture2D agentPursuingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/purple.png");
	Texture2D agentEvadingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/pink.png");
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

		agentPursuing->applySteeringForce(agentPursuing->pursue(player, maxAcceleration, maxPrediction));
		stayWithinBorders(agentPursuing, turnFactor);
		agentPursuing->updatePosition(dt);

		agentEvading->applySteeringForce(agentEvading->evade(player, maxAcceleration, maxPrediction));
		stayWithinBorders(agentEvading, turnFactor);
		agentEvading->updatePosition(dt);

		BeginDrawing();
		ClearBackground(WHITE);

		DrawTexturePro(
			agentPursuingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentPursuingTexture.width), static_cast<float>(agentPursuingTexture.height) },
			{ agentPursuing->getPosition().x * tileSize, agentPursuing->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentPursuing->getRotationInDegrees(),
			WHITE
		);

		DrawTexturePro(
			agentEvadingTexture,
			{ 0.0f, 0.0f, static_cast<float>(agentEvadingTexture.width), static_cast<float>(agentEvadingTexture.height) },
			{ agentEvading->getPosition().x * tileSize, agentEvading->getPosition().y * tileSize, 50.0f, 50.0f },
			Vector2{ 25.0f, 25.0f },
			agentEvading->getRotationInDegrees(),
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

		DrawText("P", agentPursuing->getPosition().x * tileSize, agentPursuing->getPosition().y * tileSize, 10, BLACK);
		DrawText("E", agentEvading->getPosition().x * tileSize, agentEvading->getPosition().y * tileSize, 10, BLACK);

		EndDrawing();
	}

	UnloadTexture(agentPursuingTexture);
	UnloadTexture(agentEvadingTexture);
	UnloadTexture(playerTexture);
	CloseWindow();

	return 0;
}