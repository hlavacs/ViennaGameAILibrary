/**
* The Vienna Game AI Library
*
* Demo for Monte Carlo Tree Search (MCTS): play Connect Four against VGAIL::MCTS.
*
* The game rules are in MCTS/ConnectFour.h. This file contains only the window, the input
* and the drawing. The search runs in a second thread, so the window stays responsive while
* the computer thinks.
*
* Controls
*   Left mouse button   drop a chip into the column under the mouse
*   UP / DOWN           more / less thinking time for the search
*   LEFT / RIGHT        fewer / more search threads
*   C                   change the exploration value c
*   A                   let the search play both sides (watch mode)
*   N                   new game, the other player starts
*/

#include "ViennaGameAILibrary.hpp"
#include "MCTS/ConnectFour.h"
#include "raylib.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <future>
#include <sstream>
#include <string>
#include <thread>

using namespace ConnectFour;

// The result of one search, handed from the search thread to the main thread.
struct SearchResult
{
	int column = -1;
	long long simulatedMoves = 0;
	double seconds = 0.0;
};

// Runs one search. This function is executed in a second thread.
// The game state is passed by value, so the search works on its own copy.
static SearchResult runSearch(ConnectFourState state, double timeLimitMs, double c, int threads)
{
	const long long movesBefore = executedMoves.load();
	const auto start = std::chrono::steady_clock::now();

	VGAIL::MCTS<ConnectFourState, ConnectFourAction> mcts;
	ConnectFourAction best = mcts.runMCTS(state, timeLimitMs, c, threads);

	SearchResult result;
	result.column = best.getColumn();
	result.simulatedMoves = executedMoves.load() - movesBefore;
	result.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
	return result;
}

int main()
{
	constexpr int screenWidth = 1200;
	constexpr int screenHeight = 800;

	// Board geometry: 7 x 6 cells of 100 pixels.
	constexpr float cellSize = 100.0f;
	constexpr float boardX = 50.0f;
	constexpr float boardY = 150.0f;
	constexpr float chipRadius = 40.0f;
	constexpr float panelX = 800.0f;

	const Color boardColor = { 40, 80, 170, 255 };
	const Color holeColor = { 235, 238, 245, 255 };
	const Color humanColor = { 220, 60, 50, 255 };
	const Color computerColor = { 245, 200, 40, 255 };

	// Player one is the human and plays red. Player two is the search and plays yellow.
	ConnectFourState game;
	bool humanStarts = true;
	bool watchMode = false;

	// Settings of the search that can be changed with the keyboard.
	const std::array<double, 7> timeLimits = { 50.0, 100.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0 };
	const std::array<double, 5> explorationValues = { 0.5, 1.0, 2.0, 4.0, 8.0 };
	int timeIndex = 4;
	int explorationIndex = 2;
	int threads = 1;
	const int maxThreads = std::max(1, std::min(8, static_cast<int>(std::thread::hardware_concurrency())));

	// State of the running search and of the last finished search.
	std::future<SearchResult> search;
	bool searching = false;
	double searchStartTime = 0.0;
	double searchTimeLimitMs = 0.0;
	SearchResult lastResult;
	std::string lastSearchPlayer;

	// A chip that is falling into its place. The move is applied when it has arrived.
	bool falling = false;
	int fallingColumn = 0;
	float fallingY = 0.0f;
	float fallingTargetY = 0.0f;
	Color fallingColor = WHITE;

	InitWindow(screenWidth, screenHeight, "Demo for Monte Carlo Tree Search");
	SetTargetFPS(60);

	// The demos are started from the repository root. If the font is not found, raylib uses its default font.
	Font font = LoadFontEx("Demo/res/Signika-SemiBold.ttf", 40, 0, 0);

	auto cellCenter = [&](int column, int row)
		{
			return Vector2{ boardX + (column + 0.5f) * cellSize, boardY + (rows - 1 - row + 0.5f) * cellSize };
		};

	auto startFalling = [&](int column)
		{
			falling = true;
			fallingColumn = column;
			fallingY = boardY - cellSize * 0.5f;
			fallingTargetY = cellCenter(column, game.getHeight(column)).y;
			fallingColor = (game.getPlayerTurnId() == playerOne) ? humanColor : computerColor;
		};

	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		const bool gameOver = game.getIsTerminal();
		const bool humanTurn = !gameOver && !watchMode && game.getPlayerTurnId() == playerOne;
		const bool searchTurn = !gameOver && !humanTurn;

		// ---------- Settings ----------
		if (IsKeyPressed(KEY_UP)) timeIndex = std::min(timeIndex + 1, static_cast<int>(timeLimits.size()) - 1);
		if (IsKeyPressed(KEY_DOWN)) timeIndex = std::max(timeIndex - 1, 0);
		if (IsKeyPressed(KEY_RIGHT)) threads = std::min(threads + 1, maxThreads);
		if (IsKeyPressed(KEY_LEFT)) threads = std::max(threads - 1, 1);
		if (IsKeyPressed(KEY_C)) explorationIndex = (explorationIndex + 1) % static_cast<int>(explorationValues.size());
		if (IsKeyPressed(KEY_A)) watchMode = !watchMode;

		// A new game can only start when no search is running, because the search still needs its result slot.
		if (IsKeyPressed(KEY_N) && !searching)
		{
			humanStarts = !humanStarts;
			game.reset(humanStarts ? playerOne : playerTwo);
			falling = false;
			lastResult = SearchResult{};
			lastSearchPlayer.clear();
		}

		// ---------- Human move ----------
		const Vector2 mouse = GetMousePosition();
		int hoverColumn = -1;
		if (mouse.x >= boardX && mouse.x < boardX + columns * cellSize)
		{
			hoverColumn = static_cast<int>((mouse.x - boardX) / cellSize);
		}

		if (humanTurn && !falling && hoverColumn >= 0 && !game.isColumnFull(hoverColumn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			startFalling(hoverColumn);
		}

		// ---------- Search move ----------
		// Start the search in a second thread. The main thread keeps drawing.
		if (searchTurn && !falling && !searching)
		{
			searchTimeLimitMs = timeLimits[timeIndex];
			searchStartTime = GetTime();
			lastSearchPlayer = game.getPlayerTurnId();
			search = std::async(std::launch::async, runSearch, game, searchTimeLimitMs, explorationValues[explorationIndex], threads);
			searching = true;
		}

		// Ask without waiting whether the search has finished.
		if (searching && search.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			lastResult = search.get();
			searching = false;
			if (!game.getIsTerminal() && !game.isColumnFull(lastResult.column))
			{
				startFalling(lastResult.column);
			}
		}

		// ---------- Falling chip ----------
		if (falling)
		{
			fallingY += 1800.0f * delta;
			if (fallingY >= fallingTargetY)
			{
				falling = false;
				ConnectFourAction(fallingColumn).execute(game);
			}
		}

		// ---------- Drawing ----------
		BeginDrawing();
		ClearBackground(RAYWHITE);

		DrawTextEx(font, "Monte Carlo Tree Search: Connect Four", Vector2{ boardX, 20.0f }, 40, 1.0f, BLACK);

		// Status line.
		std::string status;
		Color statusColor = BLACK;
		if (gameOver)
		{
			if (game.getWinnerPlayerId() == draw) status = "Draw. Press N for a new game.";
			else if (watchMode) status = (game.getWinnerPlayerId() == playerOne) ? "Red wins. Press N for a new game." : "Yellow wins. Press N for a new game.";
			else status = (game.getWinnerPlayerId() == playerOne) ? "You win! Press N for a new game." : "The search wins. Press N for a new game.";
		}
		else if (humanTurn) status = "Your turn (red): click a column.";
		else status = (game.getPlayerTurnId() == playerOne) ? "The search thinks for red ..." : "The search thinks for yellow ...";
		DrawTextEx(font, status.c_str(), Vector2{ boardX, 70.0f }, 30, 1.0f, statusColor);

		// Chip above the board that shows where the human would drop.
		if (humanTurn && !falling && hoverColumn >= 0 && !game.isColumnFull(hoverColumn))
		{
			DrawCircleV(Vector2{ boardX + (hoverColumn + 0.5f) * cellSize, boardY - cellSize * 0.35f }, chipRadius * 0.6f, Fade(humanColor, 0.6f));
		}

		// Board: one blue square per cell with a round hole, which is empty or holds a chip.
		for (int column = 0; column < columns; column++)
		{
			for (int row = 0; row < rows; row++)
			{
				const Vector2 center = cellCenter(column, row);
				const int chip = game.getCell(column, row);

				DrawRectangleRec(Rectangle{ center.x - cellSize * 0.5f, center.y - cellSize * 0.5f, cellSize, cellSize }, boardColor);
				if (chip == 1) DrawCircleV(center, chipRadius, humanColor);
				else if (chip == 2) DrawCircleV(center, chipRadius, computerColor);
				else DrawCircleV(center, chipRadius, holeColor);

				if (chip != 0 && game.isWinningCell(column, row))
				{
					DrawRing(center, chipRadius - 12.0f, chipRadius - 4.0f, 0.0f, 360.0f, 48, WHITE);
				}
			}
		}

		// The chip that is on its way slides over the board to its cell.
		if (falling)
		{
			DrawCircleV(Vector2{ boardX + (fallingColumn + 0.5f) * cellSize, fallingY }, chipRadius, fallingColor);
		}

		// Small marker under the column of the last move.
		if (game.getLastColumn() >= 0)
		{
			const float x = boardX + (game.getLastColumn() + 0.5f) * cellSize;
			DrawTriangle(Vector2{ x, boardY + rows * cellSize + 8.0f }, Vector2{ x - 12.0f, boardY + rows * cellSize + 28.0f },
				Vector2{ x + 12.0f, boardY + rows * cellSize + 28.0f }, DARKGRAY);
		}

		// ---------- Panel with the settings and the numbers of the search ----------
		float y = 150.0f;
		auto line = [&](const std::string& text, float size = 26.0f, Color color = BLACK)
			{
				DrawTextEx(font, text.c_str(), Vector2{ panelX, y }, size, 1.0f, color);
				y += size + 8.0f;
			};

		std::stringstream ss;
		line("Search settings", 30.0f);
		ss << "Thinking time: " << timeLimits[timeIndex] / 1000.0 << " s";
		line(ss.str()); ss.str("");
		ss << "Threads: " << threads;
		line(ss.str()); ss.str("");
		ss << "Exploration c: " << explorationValues[explorationIndex];
		line(ss.str()); ss.str("");
		line(watchMode ? "Mode: search against search" : "Mode: you against the search");

		y += 20.0f;
		line("Last search", 30.0f);
		if (searching)
		{
			// Progress bar for the running search.
			const float progress = static_cast<float>(std::min(1.0, (GetTime() - searchStartTime) * 1000.0 / searchTimeLimitMs));
			DrawRectangleRec(Rectangle{ panelX, y, 340.0f, 22.0f }, LIGHTGRAY);
			DrawRectangleRec(Rectangle{ panelX, y, 340.0f * progress, 22.0f }, (lastSearchPlayer == playerOne) ? humanColor : computerColor);
			y += 34.0f;
			line("thinking ...");
		}
		else if (lastResult.column >= 0)
		{
			ss << "Chosen column: " << lastResult.column + 1;
			line(ss.str()); ss.str("");
			ss << "Simulated moves: " << lastResult.simulatedMoves;
			line(ss.str()); ss.str("");
			if (lastResult.seconds > 0.0)
			{
				ss << "Per second: " << static_cast<long long>(static_cast<double>(lastResult.simulatedMoves) / lastResult.seconds);
				line(ss.str()); ss.str("");
			}
		}
		else
		{
			line("none yet");
		}

		y += 20.0f;
		line("Keys", 30.0f);
		line("UP / DOWN: thinking time", 22.0f, DARKGRAY);
		line("LEFT / RIGHT: threads", 22.0f, DARKGRAY);
		line("C: exploration value", 22.0f, DARKGRAY);
		line("A: watch mode on / off", 22.0f, DARKGRAY);
		line("N: new game, other player starts", 22.0f, DARKGRAY);

		EndDrawing();
	}

	// A running search cannot be stopped from outside. Wait for it before the program ends.
	if (searching)
	{
		search.wait();
	}

	UnloadFont(font);
	CloseWindow();

	return 0;
}
