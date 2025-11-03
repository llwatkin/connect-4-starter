#pragma once
#include "Game.h"

class ConnectFour : public Game
{
public:
    ConnectFour();
    ~ConnectFour();

    // Required virtual methods from Game base class
    void        setUpBoard() override;
    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        stopGame() override;
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    // AI methods
	std::vector<std::string> generateMoves(std::string gameState, int playerNumber);
    Player*     checkForWinnerWithGameState(std::string gameState);
    int         evaluate(std::string gameState, int playerNumber);
    int         negamax(std::string gameState, int depth, int playerNumber);
    std::string getBestMove();
    void        updateAI() override;
    bool        gameHasAI() override { return true; } // Set to true when AI is implemented
    Grid* getGrid() override { return _grid; }

private:
    // Constants
    static const int ROWX = 7;
    static const int ROWY = 6;
    static const int YELLOW_PLAYER = 0; // Yellow goes first in Connect Four
    static const int RED_PLAYER = 1;

    // Helper methods
    Bit*        createPiece(int pieceType);     
    int         findLowestOpenSquareY(int x);
    int         coordsToStateIndex(int x, int y);
    bool        ownersAreTheSame(Player *owner1, Player *owner2, Player *owner3, Player *owner4);
    bool        ownerNumbersAreTheSame(int owner1, int owner2, int owner3, int owner4);
    Player*     ownerAt(int x, int y);

    // Board representation
    Grid*        _grid;
};