#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "../../include/ViennaGameAILibrary.hpp"

using namespace VGAIL;

class Cell;
class ConnectFourGameState;
class ConnectFourAction;

class Cell {
private:
    int x;
    int y;
    std::string playerChip = "0";

public:
    Cell() {}

    Cell(int x, int y) : x(x), y(y) {}

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }

    void setPlayerChip(std::string playerId) {
        playerChip = playerId;
    }

    std::string getPlayerChip() const {
        return playerChip;
    }
};

class ConnectFourAction : public Action<ConnectFourGameState> {
private:
    int x;
    int y;
    std::string playerId;
    std::string actionId;

public:
    ConnectFourAction(int x, int y, std::string playerId) : x(x), y(y), playerId(playerId) {}

    ConnectFourAction() {}

    void execute(ConnectFourGameState& gameState) override;

    void setActionId(std::string id) {
        actionId = id;
    }

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }
};

class ConnectFourGameState : public MCTSState<ConnectFourGameState, ConnectFourAction> {
private:
    std::vector<Cell> gameBoard = {};
    using MCTSState<ConnectFourGameState, ConnectFourAction>::actions;
    using MCTSState<ConnectFourGameState, ConnectFourAction>::isTerminal;
    using MCTSState<ConnectFourGameState, ConnectFourAction>::playerTurnId;
    using MCTSState<ConnectFourGameState, ConnectFourAction>::winnerPlayerId;
    int currentTurn = 0;

public:
    ConnectFourGameState() {}

    ConnectFourGameState(std::vector<ConnectFourAction> actions, bool isTerminal, std::string playerTurnId, std::string winnerPlayerId = "") : MCTSState<ConnectFourGameState, ConnectFourAction>(actions, isTerminal, playerTurnId, winnerPlayerId), currentTurn(0) {}

    void initializeBoard() {
        gameBoard.clear();
        for (int x = 0; x < 6; x++) {
            for (int y = 0; y < 7; y++) {
                Cell newCell(x, y);
                gameBoard.push_back(newCell);
            }
        }
    }

    void setPlayerTurnId(std::string playerId) {
        playerTurnId = playerId;
    }

    void printBoardUsingNumbers() {
        std::cout << "Player 1: 1 vs Player 2: 2, Current Turn: " << currentTurn << ", Current Player's turn is: " << playerTurnId << '\n';
        for (const auto& entries : gameBoard) {
            if (entries.getPlayerChip() == "0") {
                std::cout << "0" << " ";
            }
            else {
                std::cout << entries.getPlayerChip() << " ";
            }

            if (entries.getY() == 6) {
                std::cout << '\n';
            }
        }
    }

    void increaseTurnCounter() {
        currentTurn++;
    }

    void resetTurnCounter() {
        currentTurn = 0;
    }

    void printBoardUsingEmoji() {

    }

    void generateActions() {
        actions.clear();
        std::vector<Cell> columnCells;

        for (int i = 0; i < 7; i++) {
            columnCells.clear();
            std::for_each(gameBoard.begin(), gameBoard.end(), [&](auto& cell) { if (cell.getY() == i) { columnCells.push_back(cell); }});
            std::sort(columnCells.begin(), columnCells.end(), [](Cell a, Cell b) { return a.getX() > b.getX(); });

            for (auto& cell : columnCells) {
                if (cell.getPlayerChip() == "0") {
                    std::string actionId = std::to_string(cell.getX() * 10 + cell.getY());
                    ConnectFourAction actionToAdd(cell.getX(), cell.getY(), playerTurnId);
                    actionToAdd.setActionId(actionId);
                    this->actions.push_back(actionToAdd);
                    break;
                }
            }
        }
    }

    void printActions() {
        std::cout << "Possible actions: " << '\n';
        for (const ConnectFourAction action : actions) {
            std::cout << "X: " << action.getX() << " Y: " << action.getY() << '\n';
        }
    }

    void useUpAllActions() {
        auto tempActions = actions;
        for (auto& action : tempActions) {
            action.execute(*this);
        }
        currentTurn++;
    }

    std::vector<Cell>& getGameBoard() {
        return gameBoard;
    }

    bool isMyChip(std::string nextPlayerChip, std::string playerChip) const {
        if (playerChip != "0" && playerChip == nextPlayerChip) {
            return true;
        }

        return false;
    }

    void setWinner(const std::string winnerId) {
        winnerPlayerId = winnerId;
    }

    std::string getHorizontalWinChip(const Cell& cell) {
        std::vector<Cell> fourInARow;
        std::for_each(gameBoard.begin(), gameBoard.end(), [&](auto& cellToAdd) { if (cellToAdd.getX() == cell.getX()) { fourInARow.push_back(cellToAdd); }});
        std::sort(fourInARow.begin(), fourInARow.end(), [](Cell a, Cell b) { return a.getY() < b.getY(); });

        int currentScore = 0;
        bool sameChip = false;
        for (int i = 0; i < 6; i++) {
            sameChip = isMyChip(fourInARow[i + 1].getPlayerChip(), fourInARow[i].getPlayerChip());
            if (sameChip == true) {
                currentScore++;
            }
            else {
                currentScore = 0;
            }

            if (currentScore == 3) {
                return fourInARow[i].getPlayerChip();
            }
        }

        return "";
    }

    std::string getVerticalWinChip(const Cell& cell) {
        std::vector<Cell> fourInARow;
        std::for_each(gameBoard.begin(), gameBoard.end(), [&](auto& cellToAdd) { if (cellToAdd.getY() == cell.getY()) { fourInARow.push_back(cellToAdd); }});
        std::sort(fourInARow.begin(), fourInARow.end(), [](Cell a, Cell b) { return a.getX() < b.getX(); });

        int currentScore = 0;
        bool sameChip = false;
        for (int i = 0; i < 5; i++) {
            sameChip = isMyChip(fourInARow[i + 1].getPlayerChip(), fourInARow[i].getPlayerChip());
            if (sameChip == true) {
                currentScore++;
            }
            else {
                currentScore = 0;
            }

            if (currentScore == 3) {
                return fourInARow[i].getPlayerChip();
            }
        }

        return "";
    }

    std::string getDiagonalWinChip(const Cell& cell) {
        std::string chip = "";
        int counter = 0;
        for (int i = 0; i < 4; i++) {
            auto nextCell = std::find_if(gameBoard.begin(), gameBoard.end(), [&](auto& c) { return c.getX() == cell.getX() - i && c.getY() == cell.getY() + i; });
            if (nextCell != gameBoard.end()) {
                if (isMyChip(cell.getPlayerChip(), nextCell->getPlayerChip())) {
                    counter++;
                }

                else {
                    counter = 0;
                }
            }

            if (counter == 4) {
                return cell.getPlayerChip();
            }
        }

        counter = 0;

        for (int i = 0; i < 4; i++) {
            auto nextCell = std::find_if(gameBoard.begin(), gameBoard.end(), [&](auto& c) { return c.getX() == cell.getX() + i && c.getY() == cell.getY() + i; });
            if (nextCell != gameBoard.end()) {
                if (isMyChip(cell.getPlayerChip(), nextCell->getPlayerChip())) {
                    counter++;
                }

                else {
                    counter = 0;
                }
            }

            if (counter == 4) {
                return cell.getPlayerChip();
            }
        }

        counter = 0;

        for (int i = 0; i < 4; i++) {
            auto nextCell = std::find_if(gameBoard.begin(), gameBoard.end(), [&](auto& c) { return c.getX() == cell.getX() + i && c.getY() == cell.getY() - i; });
            if (nextCell != gameBoard.end()) {
                if (isMyChip(cell.getPlayerChip(), nextCell->getPlayerChip())) {
                    counter++;
                }

                else {
                    counter = 0;
                }
            }

            if (counter == 4) {
                return cell.getPlayerChip();
            }
        }

        counter = 0;

        for (int i = 0; i < 4; i++) {
            auto nextCell = std::find_if(gameBoard.begin(), gameBoard.end(), [&](auto& c) { return c.getX() == cell.getX() - i && c.getY() == cell.getY() - i; });
            if (nextCell != gameBoard.end()) {
                if (isMyChip(cell.getPlayerChip(), nextCell->getPlayerChip())) {
                    counter++;
                }

                else {
                    counter = 0;
                }
            }

            if (counter == 4) {
                return cell.getPlayerChip();
            }
        }

        return "";
    }

    bool getIsTerminal() override {
        if (actions.size() == 0) {
            return true;
        }
        std::vector<Cell> filledCells;
        std::for_each(gameBoard.begin(), gameBoard.end(), [&](auto& cell) { if (cell.getPlayerChip() != "0") { filledCells.push_back(cell); }});

        std::string checkWin = "";
        for (const auto& cell : filledCells) {
            checkWin = getHorizontalWinChip(cell);
            if (checkWin != "") {
                setWinnerPlayerId(checkWin);
                return true;
            }

            checkWin = getVerticalWinChip(cell);
            if (checkWin != "") {
                setWinnerPlayerId(checkWin);
                return true;
            }

            checkWin = getDiagonalWinChip(cell);
            if (checkWin != "") {
                setWinnerPlayerId(checkWin);
                return true;
            }
        }

        return false;
    }
};

void ConnectFourAction::execute(ConnectFourGameState& gameState) {
    for (auto& entries : gameState.getGameBoard()) {
        if (entries.getX() == x && entries.getY() == y) {
            entries.setPlayerChip(playerId);
            if (gameState.getPlayerTurnId() == "1") {
                gameState.setPlayerTurnId("2");
            }
            else {
                gameState.setPlayerTurnId("1");
            }
        }
    }

    gameState.generateActions();
}



int main() {
    ConnectFourGameState gameState;

    MCTS<ConnectFourGameState, ConnectFourAction> mcts;

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

        else if (i < 3500) {
            timeMs = 300;
            c_value = 1;
        }

        else if (i < 4000) {
            timeMs = 500;
            c_value = 1;
        }

        std::cout << "Iteration number: " << i + 1 << " of MCTS vs random actions in Connect Four" << '\n';
        gameState.initializeBoard();
        if (i % 2 == 0) {
            gameState.setPlayerTurnId("1");
        }
        else {
            gameState.setPlayerTurnId("2");
        }
        gameState.generateActions();
        while (gameState.getIsTerminal() == false) {
            if (gameState.getPlayerTurnId() == "1") {
                try {
                    if (i < 1800) {
                        auto actionFromMCTS = mcts.runMCTS(gameState, timeMs, c_value, 1);
                        actionFromMCTS.execute(gameState);
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Exception: " << e.what() << '\n';
                    break;
                }
            }
            else {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(0, gameState.getActions().size() - 1);
                auto randomAction = gameState.getActions()[distr(gen)];
                randomAction.execute(gameState);
            }

            // Uncomment next line to see board after every executed action
            // gameState.printBoardUsingNumbers();
            gameState.increaseTurnCounter();
        }

        gameState.resetTurnCounter();
        std::cout << "Game over" << '\n';
        gameState.printBoardUsingNumbers();
        std::cout << "The winner is Player " << gameState.getWinnerPlayerId() << '\n';
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
    }

    std::cout << "10ms wins c = 2 of 300 Threads 1: " << msWins10 << '\n';
    std::cout << "20ms wins c = 2 of 300 Threads 1: " << msWins20 << '\n';
    std::cout << "40ms wins c = 2 of 300 Threads 1: " << msWins40 << '\n';
    std::cout << "60ms wins c = 2 of 300 Threads 1: " << msWins60 << '\n';
    std::cout << "80ms wins c = 2 of 300 Threads 1: " << msWins80 << '\n';
    std::cout << "100ms wins c = 2 of 300 Threads 1: " << msWins100 << '\n';

    return 0;
}