// Checks that VGAIL::MCTS finds simple forced moves in Connect Four.
// The game rules are shared with the graphical demo (Demo/src/MCTS/ConnectFour.h).
#include "ViennaGameAILibrary.hpp"
#include "MCTS/ConnectFour.h"
#include <initializer_list>
#include <iostream>
#include <stdexcept>

using namespace ConnectFour;

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

// Plays the given columns in turn, starting with player one.
static ConnectFourState position(std::initializer_list<int> columnsPlayed) {
    ConnectFourState state;
    for (int column : columnsPlayed) ConnectFourAction(column).execute(state);
    return state;
}

// Runs the search several times. Every run must choose the expected column.
static void expectMove(const ConnectFourState& state, int expectedColumn, const char* message) {
    constexpr int runs = 5;
    constexpr double timeLimitMs = 150.0;
    for (int run = 0; run < runs; run++) {
        VGAIL::MCTS<ConnectFourState, ConnectFourAction> mcts;
        const int column = mcts.runMCTS(state, timeLimitMs, 2.0, 1).getColumn();
        if (column != expectedColumn) {
            std::cerr << message << ": expected column " << expectedColumn << ", search chose " << column << '\n';
            require(false, message);
        }
    }
}

int main() try {
    // Rules first: four chips in a row end the game and name the winner.
    {
        ConnectFourState state = position({ 0, 0, 1, 1, 2, 2, 3 });
        require(state.getIsTerminal() && state.getWinnerPlayerId() == playerOne, "horizontal line must win");
        require(state.getActions().empty(), "finished game has no actions");
    }
    {
        ConnectFourState state = position({ 4, 0, 4, 1, 4, 2, 4 });
        require(state.getIsTerminal() && state.getWinnerPlayerId() == playerOne, "vertical line must win");
    }
    {
        ConnectFourState state = position({ 0, 1, 1, 2, 2, 3, 2, 3, 3, 6, 3 });
        require(state.getIsTerminal() && state.getWinnerPlayerId() == playerOne, "diagonal line must win");
    }

    // The search takes a win that is one move away.
    expectMove(position({ 0, 0, 1, 1, 2, 2 }), 3, "take the winning move");
    // The search blocks a line of three in the bottom row.
    expectMove(position({ 0, 0, 1, 1, 2 }), 3, "block the horizontal threat");
    // The search blocks three chips on top of each other. Here the search has a tempting move of
    // its own (column 2), so it only blocks if it expects the opponent to use the winning move.
    expectMove(position({ 4, 0, 4, 1, 4 }), 4, "block the vertical threat");

    std::cout << "PASS: Connect Four rules; MCTS takes a win and blocks horizontal and vertical threats\n";
    return 0;
}
catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
}
