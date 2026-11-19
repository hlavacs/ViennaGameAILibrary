/**
* The Vienna Game AI Library
*
* Connect Four game rules for the Monte Carlo Tree Search demo.
*
* The two classes below are everything that VGAIL::MCTS needs from a game:
*   - ConnectFourAction : one move. execute() applies it to a game state.
*   - ConnectFourState  : a position. It lists the legal moves, knows whose turn it is,
*                         and knows whether the game is over and who has won.
*/

#pragma once

#include "ViennaGameAILibrary.hpp"

#include <array>
#include <atomic>
#include <string>
#include <vector>

namespace ConnectFour
{
	constexpr int columns = 7;
	constexpr int rows = 6;

	// Player ids. VGAIL::MCTS compares these strings to find out who has won a simulated game.
	inline const std::string playerOne = "1";
	inline const std::string playerTwo = "2";
	inline const std::string draw = "Draw";

	// Counts every move that is executed, also the moves inside the simulations of the search.
	// The demo shows this number to make the work of the search visible.
	inline std::atomic<long long> executedMoves{ 0 };

	class ConnectFourState;

	class ConnectFourAction : public VGAIL::Action<ConnectFourState>
	{
	public:
		ConnectFourAction() {}
		explicit ConnectFourAction(int column) : m_column(column) {}

		// Drops a chip of the player whose turn it is into the column of this action.
		void execute(ConnectFourState& gameState) override;

		int getColumn() const { return m_column; }

	private:
		int m_column = 0;
	};

	class ConnectFourState : public VGAIL::MCTSState<ConnectFourState, ConnectFourAction>
	{
	public:
		ConnectFourState() { reset(playerOne); }

		// Empties the board. startingPlayer makes the first move.
		void reset(const std::string& startingPlayer)
		{
			m_board.fill(0);
			m_heights.fill(0);
			m_winningCells.fill(-1);
			m_moveCount = 0;
			m_lastColumn = -1;
			isTerminal = false;
			winnerPlayerId = "";
			playerTurnId = startingPlayer;
			generateActions();
		}

		// 0 = empty, 1 = chip of player one, 2 = chip of player two. Row 0 is the bottom row.
		int getCell(int column, int row) const { return m_board[index(column, row)]; }
		int getHeight(int column) const { return m_heights[column]; }
		int getMoveCount() const { return m_moveCount; }
		int getLastColumn() const { return m_lastColumn; }
		bool isColumnFull(int column) const { return m_heights[column] >= rows; }

		// The four cells of the winning line as cell indices (column + row * columns), or -1.
		const std::array<int, 4>& getWinningCells() const { return m_winningCells; }
		bool isWinningCell(int column, int row) const
		{
			for (int cell : m_winningCells)
			{
				if (cell == index(column, row)) return true;
			}
			return false;
		}

		// Called by ConnectFourAction::execute(). Returns false if the move is not legal.
		bool dropChip(int column)
		{
			if (isTerminal || column < 0 || column >= columns || isColumnFull(column)) return false;

			const int chip = (playerTurnId == playerOne) ? 1 : 2;
			const int row = m_heights[column];
			m_board[index(column, row)] = static_cast<char>(chip);
			m_heights[column]++;
			m_moveCount++;
			m_lastColumn = column;

			if (completesLine(column, row, chip))
			{
				isTerminal = true;
				winnerPlayerId = playerTurnId;
			}
			else if (m_moveCount == columns * rows)
			{
				isTerminal = true;
				winnerPlayerId = draw;
			}

			// The other player is next. The search reads this id to decide for whom a result counts.
			playerTurnId = (playerTurnId == playerOne) ? playerTwo : playerOne;
			generateActions();
			return true;
		}

	private:
		using VGAIL::MCTSState<ConnectFourState, ConnectFourAction>::actions;
		using VGAIL::MCTSState<ConnectFourState, ConnectFourAction>::isTerminal;
		using VGAIL::MCTSState<ConnectFourState, ConnectFourAction>::playerTurnId;
		using VGAIL::MCTSState<ConnectFourState, ConnectFourAction>::winnerPlayerId;

		static int index(int column, int row) { return column + row * columns; }

		// One action per column that still has room. A finished game has no actions.
		void generateActions()
		{
			actions.clear();
			if (isTerminal) return;
			for (int column = 0; column < columns; column++)
			{
				if (!isColumnFull(column)) actions.push_back(ConnectFourAction(column));
			}
		}

		// Only lines through the chip that was just placed can be new, so only they are checked.
		bool completesLine(int column, int row, int chip)
		{
			constexpr int directions[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
			for (const auto& direction : directions)
			{
				// Walk to the first chip of the line, then count forward.
				int startColumn = column, startRow = row;
				while (isChip(startColumn - direction[0], startRow - direction[1], chip))
				{
					startColumn -= direction[0];
					startRow -= direction[1];
				}
				int length = 0;
				while (isChip(startColumn + length * direction[0], startRow + length * direction[1], chip))
				{
					length++;
				}
				if (length >= 4)
				{
					for (int i = 0; i < 4; i++)
					{
						m_winningCells[i] = index(startColumn + i * direction[0], startRow + i * direction[1]);
					}
					return true;
				}
			}
			return false;
		}

		bool isChip(int column, int row, int chip) const
		{
			return column >= 0 && column < columns && row >= 0 && row < rows && m_board[index(column, row)] == chip;
		}

		std::array<char, columns * rows> m_board{};
		std::array<int, columns> m_heights{};
		std::array<int, 4> m_winningCells{};
		int m_moveCount = 0;
		int m_lastColumn = -1;
	};

	inline void ConnectFourAction::execute(ConnectFourState& gameState)
	{
		executedMoves.fetch_add(1, std::memory_order_relaxed);
		gameState.dropChip(m_column);
	}
}
