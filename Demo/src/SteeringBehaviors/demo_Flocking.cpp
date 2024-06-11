// References: https://www.raylib.com/examples/text/loader.html?name=text_input_box

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"

float minSpeed = 50.0f;
float maxSpeed = 80.0f;

float turnFactor = 2.5f;
float avoidFactor = 0.05f;
float matchingFactor = 0.1f;
float centeringFactor = 0.005f;

float separationRange = 25.0f;
float perceptionRange = 50.0f;

float margin = 50.0f;

// Avoid
char avoid_char[11] = "0.05\0";
int avoid_letterCount = 4;
Rectangle avoid_textBox = { 100, 50, 225, 30 };
bool avoid_mouseOnText = false;

// Matching
char matching_char[11] = "0.05\0";
int matching_letterCount = 4;
Rectangle matching_textBox = { 400, 50, 225, 30 };
bool matching_mouseOnText = false;

// Centering
char centering_char[11] = "0.0005\0";
int centering_letterCount = 6;
Rectangle centering_textBox = { 700, 50, 225, 30 };
bool centering_mouseOnText = false;

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1200;
	uint32_t screenHeight = 800;

	VGAIL::Flocking* flock = new VGAIL::Flocking();
	flock->setBorders(static_cast<float>(screenWidth), static_cast<float>(screenHeight), margin);
	flock->setRanges(separationRange, perceptionRange);

	InitWindow(screenWidth, screenHeight, "Demo for Flocking");
	SetTargetFPS(60);
	
	Texture2D texture = LoadTexture("Demo/res/bird.png");

	for (uint32_t i = 0; i < 100; i++)
	{
		flock->addBoid(VGAIL::Vec2f{VGAIL::random(0.0f, static_cast<float>(screenWidth)), VGAIL::random(0.0f, static_cast<float>(screenHeight))}, 
						VGAIL::Vec2f{VGAIL::random(0.5f, 1.0f), VGAIL::random(0.5f, 1.0f)}, 
						minSpeed, maxSpeed);
	}

	while (!WindowShouldClose())
	{
		flock->update(GetFrameTime(), avoidFactor, matchingFactor, centeringFactor, turnFactor);

		// Avoid
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
                    avoid_char[avoid_letterCount+1] = '\0';
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

		// Matching
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
                    matching_char[matching_letterCount+1] = '\0';
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

		// Centering
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
					centering_char[centering_letterCount+1] = '\0';
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
		ClearBackground(WHITE);

		for (VGAIL::Boid* boid : flock->boids)
		{
			DrawTexturePro(
				texture, 
				{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) }, 
				{ boid->getPosition().x, boid->getPosition().y, 50.0f, 50.0f }, 
				Vector2{ 25.0f, 25.0f }, 
				boid->getRotationInDegrees() - 90.f,  
				WHITE
			);
		}

		// Avoid factor
		 DrawText("avoid factor", 100, 30, 20, GRAY);

		DrawRectangleRec(avoid_textBox, LIGHTGRAY);
		if (avoid_mouseOnText) 
			DrawRectangleLines((int)avoid_textBox.x, (int)avoid_textBox.y, (int)avoid_textBox.width, (int)avoid_textBox.height, BLACK);
		else 
			DrawRectangleLines((int)avoid_textBox.x, (int)avoid_textBox.y, (int)avoid_textBox.width, (int)avoid_textBox.height, DARKGRAY);

		DrawText(avoid_char, (int)avoid_textBox.x + 5, (int)avoid_textBox.y + 8, 20, BLACK);

		// Matching factor
		 DrawText("matching factor", 400, 30, 20, GRAY);

		DrawRectangleRec(matching_textBox, LIGHTGRAY);
		if (matching_mouseOnText) 
			DrawRectangleLines((int)matching_textBox.x, (int)matching_textBox.y, (int)matching_textBox.width, (int)matching_textBox.height, BLACK);
		else 
			DrawRectangleLines((int)matching_textBox.x, (int)matching_textBox.y, (int)matching_textBox.width, (int)matching_textBox.height, DARKGRAY);

		DrawText(matching_char, (int)matching_textBox.x + 5, (int)matching_textBox.y + 8, 20, BLACK);

		// Centering factor
		 DrawText("centering factor", 700, 30, 20, GRAY);

		DrawRectangleRec(centering_textBox, LIGHTGRAY);
		if (centering_mouseOnText) 
			DrawRectangleLines((int)centering_textBox.x, (int)centering_textBox.y, (int)centering_textBox.width, (int)centering_textBox.height, BLACK);
		else 
			DrawRectangleLines((int)centering_textBox.x, (int)centering_textBox.y, (int)centering_textBox.width, (int)centering_textBox.height, DARKGRAY);

		DrawText(centering_char, (int)centering_textBox.x + 5, (int)centering_textBox.y + 8, 20, BLACK);

		EndDrawing();
	}

	UnloadTexture(texture);

	CloseWindow();

	return 0;
}