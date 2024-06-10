// Resources
	// https://kenney.nl/assets/shape-characters

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	InitWindow(screenWidth, screenHeight, "Demo for Seek and Flee");
	SetTargetFPS(60);
	
	VGAIL::Boid* agentSeeking = new VGAIL::Boid(VGAIL::Vec2f{100.0f, 100.0f}, VGAIL::Vec2f{1.5f, 1.5f}, 0);
	VGAIL::Boid* agentFleeing = new VGAIL::Boid(VGAIL::Vec2f{900.0f, 600.0f}, VGAIL::Vec2f{1.5f, 1.5f}, 0);
	VGAIL::Boid* player = new VGAIL::Boid(VGAIL::Vec2f{500.0f, 500.0f}, VGAIL::Vec2f{0.2f, 0.2}, 0);

	agentSeeking->setMinSpeed(2.0f);
	agentSeeking->setMaxSpeed(3.0f);
	agentFleeing->setMinSpeed(2.0f);
	agentFleeing->setMaxSpeed(3.0f);

	Texture2D agentSeekingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");
	Texture2D agentFleeingTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/green.png");
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

		agentSeeking->applySteeringForce(agentSeeking->seek(player->getPosition(), 0.1f));
		agentSeeking->updatePosition();

		agentFleeing->applySteeringForce(agentFleeing->flee(player->getPosition(), 0.1f));
		agentFleeing->updatePosition();

		agentSeeking->stayWithinBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), 50.0f, 2.0f);
		agentFleeing->stayWithinBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), 50.0f, 2.0f);

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