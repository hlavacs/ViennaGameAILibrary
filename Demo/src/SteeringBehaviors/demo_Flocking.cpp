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

float margin = 50.0f;

void stayWithinBorders(VGAIL::Boid* boid, float turnFactor)
{
	if (boid->getPosition().x < margin)
	{
		boid->setVelocity({ boid->getVelocity().x + turnFactor, boid->getVelocity().y });
	}
	if (boid->getPosition().x > static_cast<float>(screenWidth) - margin)
	{
		boid->setVelocity({ boid->getVelocity().x - turnFactor, boid->getVelocity().y });
	}

	if (boid->getPosition().y < margin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y + turnFactor });

	}
	if (boid->getPosition().y > static_cast<float>(screenHeight) - margin)
	{
		boid->setVelocity({ boid->getVelocity().x, boid->getVelocity().y - turnFactor });
	}
}

int main(int argc, char* argv[])
{
	float minSpeed = 50.0f;
	float maxSpeed = 80.0f;

	float turnFactor = 2.5f;
	float avoidFactor = 0.05f;
	float matchingFactor = 0.1f;
	float centeringFactor = 0.005f;

	float separationRange = 25.0f;
	float perceptionRange = 50.0f;

	// Avoid factor UI variables
	char avoid_char[11] = "0.05\0";
	int avoid_letterCount = 4;
	Rectangle avoid_textBox = { 150, 70, 225, 40 };
	bool avoid_mouseOnText = false;

	// Matching factor UI variables
	char matching_char[11] = "0.1\0";
	int matching_letterCount = 4;
	Rectangle matching_textBox = { 450, 70, 225, 40 };
	bool matching_mouseOnText = false;

	// Centering factor UI variables
	char centering_char[11] = "0.005\0";
	int centering_letterCount = 6;
	Rectangle centering_textBox = { 750, 70, 225, 40 };
	bool centering_mouseOnText = false;

	VGAIL::Flock* flock = new VGAIL::Flock();
	flock->setRanges(separationRange, perceptionRange);

	InitWindow(static_cast<int>(screenWidth), static_cast<int>(screenHeight), "Demo for Flocking");
	SetTargetFPS(60);

	Texture2D texture = LoadTexture("Demo/res/demo_SteeringBehaviors/fish.png");

	for (uint32_t i = 0; i < 100; i++)
	{
		flock->addBoid(VGAIL::Vec2f{ VGAIL::randomFloat(0.0f, screenWidth), VGAIL::randomFloat(0.0f, screenHeight) },
			VGAIL::Vec2f{ VGAIL::randomFloat(0.5f, 1.0f) },
			minSpeed,
			maxSpeed);
	}

	while (!WindowShouldClose())
	{
		for (VGAIL::Boid* boid : flock->boids)
		{
			stayWithinBorders(boid, turnFactor);
		}
		flock->update(GetFrameTime(), avoidFactor, matchingFactor, centeringFactor);

		// Avoid factor
		if (CheckCollisionPointRec(GetMousePosition(), avoid_textBox))
			avoid_mouseOnText = true;
		else
			avoid_mouseOnText = false;

		if (avoid_mouseOnText)
		{
			SetMouseCursor(MOUSE_CURSOR_IBEAM);
			int key = GetCharPressed();

			while (key > 0)
			{
				if ((key >= 32) && (key <= 125) && (avoid_letterCount < 10))
				{
					avoid_char[avoid_letterCount] = (char)key;
					avoid_char[avoid_letterCount + 1] = '\0';
					avoid_letterCount++;
				}

				key = GetCharPressed();
			}

			if (IsKeyPressed(KEY_BACKSPACE))
			{
				avoid_letterCount--;
				if (avoid_letterCount < 0)
					avoid_letterCount = 0;
				avoid_char[avoid_letterCount] = '\0';
			}


			if (IsKeyPressed(KEY_ENTER))
			{
				avoidFactor = atof(avoid_char);
			}
		}
		else
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);

		// Matching factor
		if (CheckCollisionPointRec(GetMousePosition(), matching_textBox))
			matching_mouseOnText = true;
		else
			matching_mouseOnText = false;

		if (matching_mouseOnText)
		{
			SetMouseCursor(MOUSE_CURSOR_IBEAM);
			int key = GetCharPressed();

			while (key > 0)
			{
				if ((key >= 32) && (key <= 125) && (matching_letterCount < 10))
				{
					matching_char[matching_letterCount] = (char)key;
					matching_char[matching_letterCount + 1] = '\0';
					matching_letterCount++;
				}

				key = GetCharPressed();
			}

			if (IsKeyPressed(KEY_BACKSPACE))
			{
				matching_letterCount--;
				if (matching_letterCount < 0)
					matching_letterCount = 0;
				matching_char[matching_letterCount] = '\0';
			}

			if (IsKeyPressed(KEY_ENTER))
			{
				matchingFactor = atof(matching_char);
			}
		}
		else
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);

		// Centering factor
		if (CheckCollisionPointRec(GetMousePosition(), centering_textBox))
			centering_mouseOnText = true;
		else
			centering_mouseOnText = false;

		if (centering_mouseOnText)
		{
			SetMouseCursor(MOUSE_CURSOR_IBEAM);
			int key = GetCharPressed();

			while (key > 0)
			{
				if ((key >= 32) && (key <= 125) && (centering_letterCount < 10))
				{
					centering_char[centering_letterCount] = (char)key;
					centering_char[centering_letterCount + 1] = '\0';
					centering_letterCount++;
				}

				key = GetCharPressed();
			}

			if (IsKeyPressed(KEY_BACKSPACE))
			{
				centering_letterCount--;
				if (centering_letterCount < 0)
					centering_letterCount = 0;
				centering_char[centering_letterCount] = '\0';
			}

			if (IsKeyPressed(KEY_ENTER))
			{
				centeringFactor = atof(centering_char);
			}
		}
		else
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
 
		BeginDrawing();
		ClearBackground(SKYBLUE);

		for (VGAIL::Boid* boid : flock->boids)
		{
			DrawTexturePro(
				texture,
				{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) },
				{ boid->getPosition().x, boid->getPosition().y, 40.0f, 40.0f },
				Vector2{ 25.0f, 25.0f },
				boid->getRotationInDegrees(),
				WHITE
			);
		}

		// Avoid factor
		DrawText("avoid factor", avoid_textBox.x, avoid_textBox.y - 35, 30, BLACK);

		DrawRectangleRec(avoid_textBox, LIGHTGRAY);
		if (avoid_mouseOnText)
			DrawRectangleLines((int)avoid_textBox.x, (int)avoid_textBox.y, (int)avoid_textBox.width, (int)avoid_textBox.height, BLACK);
		else
			DrawRectangleLines((int)avoid_textBox.x, (int)avoid_textBox.y, (int)avoid_textBox.width, (int)avoid_textBox.height, DARKGRAY);

		DrawText(avoid_char, (int)avoid_textBox.x + 5, (int)avoid_textBox.y + 8, 25, BLACK);

		// Matching factor
		DrawText("matching factor", matching_textBox.x, matching_textBox.y - 35, 30, BLACK);

		DrawRectangleRec(matching_textBox, LIGHTGRAY);
		if (matching_mouseOnText)
			DrawRectangleLines((int)matching_textBox.x, (int)matching_textBox.y, (int)matching_textBox.width, (int)matching_textBox.height, BLACK);
		else
			DrawRectangleLines((int)matching_textBox.x, (int)matching_textBox.y, (int)matching_textBox.width, (int)matching_textBox.height, DARKGRAY);

		DrawText(matching_char, (int)matching_textBox.x + 5, (int)matching_textBox.y + 8, 25, BLACK);

		// Centering factor
		DrawText("centering factor", centering_textBox.x, centering_textBox.y - 35, 30, BLACK);

		DrawRectangleRec(centering_textBox, LIGHTGRAY);
		if (centering_mouseOnText)
			DrawRectangleLines((int)centering_textBox.x, (int)centering_textBox.y, (int)centering_textBox.width, (int)centering_textBox.height, BLACK);
		else
			DrawRectangleLines((int)centering_textBox.x, (int)centering_textBox.y, (int)centering_textBox.width, (int)centering_textBox.height, DARKGRAY);

		DrawText(centering_char, (int)centering_textBox.x + 5, (int)centering_textBox.y + 8, 25, BLACK);

		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}