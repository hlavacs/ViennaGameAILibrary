#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "../../include/ViennaGameAILibrary.hpp"
#include <set>
#include <queue>

using namespace VGAIL;

class HexAction;
class HexGameState;

class Hex {
private:
    std::vector<std::vector<int>> gameBoard = {};
    int playingSize;
public:
    Hex() {}

    bool isConnected(int playerId) {
        std::set<std::pair<int, int>> visited;
        std::queue<std::pair<int, int>> toVisit;

        // Player 1 top to bottom
        if (playerId == 1) {
            for (int i = 0; i < playingSize; i++) {
                if (gameBoard[0][i] == 1) {
                    visited.insert({ 0, i });
                    toVisit.push({ 0, i });
                }
            }
        }

        else {
            for (int i = 0; i < playingSize; i++) {
                if (gameBoard[i][0] == 2) {
                    visited.insert({ i, 0 });
                    toVisit.push({ i, 0 });
                }
            }
        }

        while (!toVisit.empty()) {
            auto currentCell = toVisit.front();
            toVisit.pop();

            if (playerId == 1 && currentCell.first == playingSize - 1) {
                return true;
            }

            if (playerId == 2 && currentCell.second == playingSize - 1) {
                return true;
            }

            std::pair<int, int> upperLeftNeighbour = { currentCell.first - 1, currentCell.second };
            std::pair<int, int> upperRightNeighbour = { currentCell.first - 1, currentCell.second + 1 };
            std::pair<int, int> leftNeighbour = { currentCell.first, currentCell.second - 1 };
            std::pair<int, int> rightNeighbour = { currentCell.first, currentCell.second + 1 };
            std::pair<int, int> lowerLeftNeighbour = { currentCell.first + 1, currentCell.second - 1 };
            std::pair<int, int> lowerRightNeighbour = { currentCell.first + 1, currentCell.second };

            std::vector<std::pair<int, int>> neighbours = { upperLeftNeighbour, upperRightNeighbour, leftNeighbour, rightNeighbour, lowerLeftNeighbour, lowerRightNeighbour };

            for (auto& n : neighbours) {
                if (n.first >= 0 && n.first < playingSize && n.second >= 0 && n.second < playingSize && gameBoard[n.first][n.second] == playerId && visited.find({ n.first, n.second }) == visited.end()) {
                    visited.insert({ n.first, n.second });
                    toVisit.push({ n.first, n.second });
                }
            }
        }

        return false;

    }

    void setEntry(int x, int y, int playerId) {
        gameBoard[x][y] = playerId;
    }

    void initializeBoard(int size) {
        gameBoard.clear();
        playingSize = size;

        for (int i = 0; i < size; i++) {
            std::vector<int> row;
            for (int k = 0; k < size; k++) {
                row.push_back(0);
            }

            gameBoard.push_back(row);
        }
    }

    int getPlayingSize() {
        return playingSize;
    }

    std::vector<std::vector<int>> getGameBoard() {
        return gameBoard;
    }
};

class HexAction : public Action<HexGameState> {
private:
    int x;
    int y;
    int playerId;
    std::string actionId;

public:
    HexAction() {}

    HexAction(int x, int y, int playerId) : x(x), y(y), playerId(playerId) {}

    void execute(HexGameState& gameState) override;

    void setActionId(std::string action_id) {
        actionId = action_id;
    }

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }
};

class HexGameState : public MCTSState<HexGameState, HexAction> {
private:
    Hex hexGame;
    using MCTSState<HexGameState, HexAction>::actions;
    using MCTSState<HexGameState, HexAction>::isTerminal;
    using MCTSState<HexGameState, HexAction>::playerTurnId;
    using MCTSState<HexGameState, HexAction>::winnerPlayerId;
    int currentTurn = 0;

public:
    HexGameState() {}

    HexGameState(std::vector<HexAction> actions, bool isTerminal, std::string playerTurnId, std::string winnerPlayerId = "") : MCTSState<HexGameState, HexAction>(actions, isTerminal, playerTurnId, winnerPlayerId), currentTurn(0) {}

    void initializeBoard(int size) {
        hexGame.initializeBoard(size);
    }

    void setPlayerTurnId(std::string playerId) {
        playerTurnId = playerId;
    }

    bool getIsTerminal() override {
        if (hexGame.isConnected(1)) {
            winnerPlayerId = "1";
            return true;
        }
        if (hexGame.isConnected(2)) {
            winnerPlayerId = "2";
            return true;
        }
        return false;
    }

    void generateActions() {
        actions.clear();
        for (int i = 0; i < hexGame.getPlayingSize(); i++) {
            for (int k = 0; k < hexGame.getPlayingSize(); k++) {
                if (hexGame.getGameBoard()[i][k] == 0) {
                    HexAction actionToAdd(i, k, std::stoi(playerTurnId));
                    actionToAdd.setActionId(std::to_string(i) + std::to_string(k));
                    actions.push_back(actionToAdd);
                }
            }
        }
    }

    void printHexBoard() {
        for (int i = 0; i < hexGame.getPlayingSize(); i++) {
            for (int k = 0; k < hexGame.getPlayingSize(); k++) {
                std::cout << hexGame.getGameBoard()[i][k] << " ";
            }
            std::cout << '\n';
            for (int w = 0; w < i + 1; w++) {
                std::cout << " ";
            }
        }
    }

    void increaseTurnCounter() {
        currentTurn++;
    }

    void resetTurnCounter() {
        currentTurn = 0;
    }

    void printActions() {
        std::cout << "Actions: " << '\n';
        for (auto& a : actions) {
            std::cout << "X: " << a.getX() << " Y: " << a.getY() << '\n';
        }
    }

    void setHexEntry(int x, int y, int playerId) {
        hexGame.setEntry(x, y, playerId);
    }
};

void HexAction::execute(HexGameState& gameState) {
    gameState.setHexEntry(x, y, playerId);

    if (gameState.getPlayerTurnId() == "1") {
        gameState.setPlayerTurnId("2");
    }
    else {
        gameState.setPlayerTurnId("1");
    }

    gameState.generateActions();
}

int main() {
    HexGameState gameState;

    MCTS<HexGameState, HexAction> mcts;

    int player1wins = 0;
    int player2wins = 0;

    double timeMs = 0;
    double c_value;

    int msWins10 = 0;
    int msWins20 = 0;
    int msWins40 = 0;

    int msWins60 = 0;
    int msWins80 = 0;
    int msWins100 = 0;

    for (int i = 0; i < 1800; i++) {
        std::cout << "Iteration: " << i + 1 << '\n';

        gameState.initializeBoard(11);
        if (i % 2 == 0) {
            gameState.setPlayerTurnId("1");
        }
        else {
            gameState.setPlayerTurnId("2");
        }
        gameState.generateActions();

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

        while (gameState.getIsTerminal() == false) {
            if (gameState.getPlayerTurnId() == "1") {
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
            // gameState.printHexBoard();
            gameState.increaseTurnCounter();
        }

        if (gameState.getWinnerPlayerId() == "1") {
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

        gameState.resetTurnCounter();
        gameState.printHexBoard();
        std::cout << "Winner is: " << gameState.getWinnerPlayerId() << '\n';
    }

    std::cout << "10ms wins c = 2 of 300 Threads 1: " << msWins10 << '\n';
    std::cout << "20ms wins c = 2 of 300 Threads 1: " << msWins20 << '\n';
    std::cout << "40ms wins c = 2 of 300 Threads 1: " << msWins40 << '\n';
    std::cout << "60ms wins c = 2 of 300 Threads 1: " << msWins60 << '\n';
    std::cout << "80ms wins c = 2 of 300 Threads 1: " << msWins80 << '\n';
    std::cout << "100ms wins c = 2 of 300 Threads 1: " << msWins100 << '\n';

    return 0;
}