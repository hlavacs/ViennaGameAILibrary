#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "../../include/ViennaGameAILibrary.hpp"

using namespace VGAIL;

class TicTacToeAction;
class TicTacToeGameState;

class TicTacToeAction : public Action<TicTacToeGameState> {
private:
    std::string playerId;
    std::string actionId;
    int pos;

public:
    TicTacToeAction(int pos, std::string playerId) : pos(pos), playerId(playerId) {}

    TicTacToeAction() {}

    void execute(TicTacToeGameState& gameState) override;

    void setActionId(std::string newId) {
        actionId = newId;
    }

    int getPos() {
        return pos;
    }
};

class TicTacToeGameState : public MCTSState<TicTacToeGameState, TicTacToeAction> {
private:
    std::vector<std::string> gameBoard = {};
    using MCTSState<TicTacToeGameState, TicTacToeAction>::actions;
    using MCTSState<TicTacToeGameState, TicTacToeAction>::isTerminal;
    using MCTSState<TicTacToeGameState, TicTacToeAction>::playerTurnId;
    using MCTSState<TicTacToeGameState, TicTacToeAction>::winnerPlayerId;
    int currentTurn = 0;

public:
    TicTacToeGameState() {}

    TicTacToeGameState(std::vector<TicTacToeAction> actions, bool isTerminal, std::string playerTurnId, std::string winnerPlayerId = "") : MCTSState<TicTacToeGameState, TicTacToeAction>(actions, isTerminal, playerTurnId, winnerPlayerId), currentTurn(0) {}

    void initializeBoard() {
        gameBoard.clear();
        for (int i = 0; i < 9; i++) {
            gameBoard.push_back("Empty");
        }
    }

    void setPlayerTurnId(std::string playerId) {
        playerTurnId = playerId;
    }

    void printBoardUsingCharacters() {
        for (int i = 0; i < 3; i++) {
            for (int k = 0; k < 3; k++) {
                std::cout << gameBoard[i * 3 + k] << " | ";
            }
            std::cout << '\n';
        }
    }

    void increaseTurnCounter() {
        currentTurn++;
    }

    std::vector<std::string> getGameBoard() {
        return gameBoard;
    }

    void setEntry(int pos, std::string playerId) {
        gameBoard[pos] = playerId;
    }

    void generateActions() {
        actions.clear();
        for (int i = 0; i < 9; i++) {
            if (gameBoard[i] == "Empty") {
                int pos = i;
                std::string actionId = std::to_string(pos);
                TicTacToeAction actionToAdd(pos, playerTurnId);
                actionToAdd.setActionId(actionId);
                this->actions.push_back(actionToAdd);
            }
        }
    }

    bool getIsTerminal() override {
        if (gameBoard[4] != "Empty") {
            if (gameBoard[0] == gameBoard[4] && gameBoard[4] == gameBoard[8]) {
                winnerPlayerId = gameBoard[4];

                return true;
            }
            else if (gameBoard[2] == gameBoard[4] && gameBoard[4] == gameBoard[6]) {
                winnerPlayerId = gameBoard[4];

                return true;
            }
        }

        for (int i = 0; i < 3; i++) {
            if (gameBoard[i * 3] != "Empty") {
                if (gameBoard[i * 3] == gameBoard[i * 3 + 1] && gameBoard[i * 3] == gameBoard[i * 3 + 2]) {
                    winnerPlayerId = gameBoard[i * 3];

                    return true;
                }
            }
        }

        for (int i = 0; i < 3; i++) {
            if (gameBoard[i] != "Empty") {
                if (gameBoard[i] == gameBoard[i + 3] && gameBoard[i] == gameBoard[i + 6]) {
                    winnerPlayerId = gameBoard[i];

                    return true;
                }
            }
        }

        if (actions.size() == 0) {
            winnerPlayerId = "Draw";

            return true;
        }

        return false;
    }
};

void TicTacToeAction::execute(TicTacToeGameState& gameState) {
    if (gameState.getGameBoard()[pos] != "Empty") {
        return;
    }

    gameState.setEntry(pos, playerId);
    if (gameState.getPlayerTurnId() == "X") {
        gameState.setPlayerTurnId("O");
    }
    else {
        gameState.setPlayerTurnId("X");
    }
    gameState.generateActions();
}

int main() {
    TicTacToeGameState gameState;

    MCTS<TicTacToeGameState, TicTacToeAction> mcts;

    double timeMs = 0;
    double c_value;

    int msWins10 = 0;
    int msWins20 = 0;
    int msWins40 = 0;

    int msWins60 = 0;
    int msWins80 = 0;
    int msWins100 = 0;

    for (int i = 0; i < 1800; i++) {
        if (i < 300) {
            timeMs = 10;
            c_value = 4;
        }

        else if (i < 600) {
            timeMs = 20;
            c_value = 4;
        }

        else if (i < 900) {
            timeMs = 40;
            c_value = 4;
        }

        else if (i < 1200) {
            timeMs = 60;
            c_value = 4;
        }

        else if (i < 1500) {
            timeMs = 80;
            c_value = 4;
        }

        else if (i < 1800) {
            timeMs = 100;
            c_value = 4;
        }

        gameState.initializeBoard();
        if (i % 2 == 0) {
            gameState.setPlayerTurnId("X");
        }
        else {
            gameState.setPlayerTurnId("O");
        }
        gameState.generateActions();
        std::cout << "Iteration number: " << i + 1 << '\n';
        while (gameState.getIsTerminal() == false) {
            if (gameState.getPlayerTurnId() == "X") {
                for (auto& actions : gameState.getActions()) {
                    std::cout << actions.getPos() << '\n';
                }

                auto actionFromMCTS = mcts.runMCTS(gameState, timeMs, c_value, 1);
                actionFromMCTS.execute(gameState);
            }
            else {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(0, gameState.getActions().size() - 1);
                auto randomAction = gameState.getActions()[distr(gen)];
                randomAction.execute(gameState);
            }

            // Uncomment next line to print board after every action
            // gameState.printBoardUsingCharacters();
            gameState.increaseTurnCounter();
        }

        std::cout << "Game over, winner is: " << gameState.getWinnerPlayerId() << '\n';
        gameState.printBoardUsingCharacters();
        if (gameState.getWinnerPlayerId() == "X") {
            if (i < 300) {
                msWins10++;
            }

            else if (i < 600) {
                msWins20++;
            }

            else if (i < 900) {
                msWins40++;
            }

            else if (i < 1200) {
                msWins60++;
            }

            else if (i < 1500) {
                msWins80++;
            }

            else if (i < 1800) {
                msWins100++;
            }
        }
    }

    std::cout << "10ms wins c = 2 of 300 Threads 1: " << msWins10 << '\n';
    std::cout << "20ms wins c = 2 of 300 Threads 1: " << msWins20 << '\n';
    std::cout << "40ms wins c = 2 of 300 Threads 1: " << msWins40 << '\n';
    std::cout << "60ms wins c = 2 of 300 Threads 1: " << msWins60 << '\n';
    std::cout << "80ms wins c = 2 of 300 Threads 1: " << msWins80 << '\n';
    std::cout << "100ms wins c = 2 of 300 Threads 1: " << msWins100 << '\n';

    return 0;
}