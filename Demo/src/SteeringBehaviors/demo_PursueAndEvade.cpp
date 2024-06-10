// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	InitWindow(screenWidth, screenHeight, "Demo for Pursue and Evade");
	SetTargetFPS(60);
	
	VGAIL::Boid* agentPursuing = new VGAIL::Boid(VGAIL::Vec2f{100.0f, 100.0f}, VGAIL::Vec2f{1.5f, 1.5f}, 0);
	VGAIL::Boid* agentEvading = new VGAIL::Boid(VGAIL::Vec2f{900.0f, 600.0f}, VGAIL::Vec2f{1.5f, 1.5f}, 0);
	VGAIL::Boid* player = new VGAIL::Boid(VGAIL::Vec2f{500.0f, 500.0f}, VGAIL::Vec2f{0.2f, 0.2f}, 0);

	agentPursuing->setMinSpeed(2.0f);
	agentPursuing->setMaxSpeed(3.0f);
	agentEvading->setMinSpeed(2.0f);
	agentEvading->setMaxSpeed(3.0f);

	Texture2D agentPursuingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");
	Texture2D agentEvadingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/green.png");
	Texture2D playerTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/yellow.png");

	float speed = 200.0f;
	while (!WindowShouldClose())
	{
		float dt = GetFrameTime();

		if (IsKeyDown(KEY_LEFT))
		{
			float x = player->getPosition().x - dt * speed;
			player->setPosition({x, player->getPosition().y});
		}

		if (IsKeyDown(KEY_RIGHT))
		{
			float x = player->getPosition().x + dt * speed;
			player->setPosition({x, player->getPosition().y});
		}

		if (IsKeyDown(KEY_UP))
		{
			float y = player->getPosition().y - dt * speed;
			player->setPosition({player->getPosition().x, y});
		}

		if (IsKeyDown(KEY_DOWN))
		{
			float y = player->getPosition().y + dt * speed;
			player->setPosition({player->getPosition().x, y});
		}

		agentPursuing->applySteeringForce(agentPursuing->pursue(player, 0.1f));
		agentPursuing->updatePosition();

		agentEvading->applySteeringForce(agentEvading->evade(player, 0.1f));
		agentEvading->updatePosition();

		agentPursuing->stayWithinBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), 50.0f, 2.0f);
		agentEvading->stayWithinBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), 50.0f, 2.0f);

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