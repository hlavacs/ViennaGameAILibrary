// Resources
	// https://kenney.nl/assets/shape-characters
	// https://www.youtube.com/watch?v=ujsR2vcJlLk&t

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	InitWindow(screenWidth, screenHeight, "Demo for Wander");
	SetTargetFPS(60);
	
	VGAIL::Boid* agent = new VGAIL::Boid(VGAIL::Vec2f{500.0f, 300.0f}, VGAIL::Vec2f{ VGAIL::random(0.5f, 1.0f) }, 0);
	agent->setMinSpeed(1.0f);
	agent->setMaxSpeed(2.0f);

	Texture2D agentTexture = LoadTexture("Demo/res/demo_SteeringBehaviors/blue.png");

	while (!WindowShouldClose())
	{
		agent->applySteeringForce(agent->wander(300.0f, 150.0f, 0.2f, 1.0f));
		agent->stayWithinBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), 30.0f, 0.5f);
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