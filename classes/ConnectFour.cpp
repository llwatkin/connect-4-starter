#include "ConnectFour.h"

ConnectFour::ConnectFour()
{
    _grid = new Grid(ROWX, ROWY);
}

ConnectFour::~ConnectFour()
{
    delete _grid;
}

Bit* ConnectFour::createPiece(int playerNumber)
{
    // Depending on playerNumber load the "yellow.png" or the "red.png" graphic
    Bit *bit = new Bit();

    bit->LoadTextureFromFile(playerNumber == AI_PLAYER ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber == AI_PLAYER ? 1 : 0));

    return bit;
}

Player* ConnectFour::ownerAt(int x, int y)
{
    if (x < 0 || x >= ROWX || y < 0 || y >= ROWY) return nullptr;
    ChessSquare *square = _grid->getSquare(x, y);
    if (!square || !square->bit()) return nullptr;
    return square->bit()->getOwner();
}

//
// Finds the lowest open space in the given row and returns its Y index
//
int ConnectFour::findLowestOpenSquareY(int x)
{
    int lowestY = 0;
    
    while (lowestY < ROWY - 1) {
        lowestY++;
        if (ownerAt(x, lowestY) != nullptr) return lowestY - 1;
    }

    return lowestY;
}

void ConnectFour::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "square.png");

    //if (gameHasAI()) setAIPlayer(RED_PLAYER); // AI will play second

    startGame();
}
    
Player* ConnectFour::checkForWinner() 
{
    // Iterate through the board spaces
    // For each space, check backwards and forwards for four of the same pieces in a row
    // Also check the up diagonal and down diagonal
    return nullptr;
}

bool ConnectFour::checkForDraw()
{
    // Iterate through grid
    // If any spot is open, there is no draw
    return false;
}

std::string ConnectFour::initialStateString()
{
	return "000000000000000000000000000000000000000000";
}

std::string ConnectFour::stateString() 
{
    return "000000000000000000000000000000000000000000";
}

void ConnectFour::setStateString(const std::string &s)
{
    // Return the board to the state described by the state string
}

//
// Handles placing pieces in empty spots
//
bool ConnectFour::actionForEmptyHolder(BitHolder &holder)
{
    // Place a token for the current player at the bottommost spot in the column they click in
    if (holder.bit()) return false;
    ChessSquare *clickedSquare = dynamic_cast<ChessSquare*>(&holder);
    if (!clickedSquare) return false;

    // Place a piece for the current player at this lowest square
    int currentPlayerIndex = getCurrentPlayer()->playerNumber();
    Bit *bit = createPiece(currentPlayerIndex == 0 ? HUMAN_PLAYER : AI_PLAYER);
    
    if (bit) {
        int rowX = clickedSquare->getColumn();
        int rowY = findLowestOpenSquareY(rowX);
        ChessSquare *lowestSquareDown = _grid->getSquare(rowX, rowY);
        lowestSquareDown->setBit(bit);
        ImVec2 pos = lowestSquareDown->getPosition();
        bit->setPosition(pos);

        Logger &logger = Logger::GetInstance();
        logger.Event("Player " + std::to_string(currentPlayerIndex) + " placed a piece at (" + std::to_string(rowX) + ", " + std::to_string(rowY) + ")");

        endTurn();
        return true;
    }   

    return false;
}

bool ConnectFour::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // No moving in Connect Four
    return false;
}

bool ConnectFour::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    // No moving in Connect Four
    return false;
}

void ConnectFour::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
    _gameOptions.gameOver = false; // Reset so we can play a new game
}

void ConnectFour::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    // No moving in Connect Four
}

//
// Generate a vector of possible game states from the given player's perspective
//
std::vector<std::string> ConnectFour::generateMoves(std::string gameState, int playerNumber)
{
	// Return state strings for every possible move from the current player's perspective
    std::vector<std::string> moves;

    for (size_t i = 0; i < gameState.length(); i++)
    {
        if (gameState[i] == '0') // TODO: make sure to get the lowest Y for this X
        {
            std::string move = gameState;
            move[i] = '1' + playerNumber;
            moves.push_back(move);
        }
    }
    
    return moves;
}

//
// A win-checking function that uses the string game state rather than the current board (used for AI decision-making)
//
Player* ConnectFour::checkForWinnerWithGameState(std::string gameState)
{
    return nullptr;
}

//
// Return the value of a move depending on who wins the game (1 if player wins, -1 if opponent wins, 0 if draw)
//
int ConnectFour::evaluate(std::string gameState, int playerNumber)
{
	Player *winner = checkForWinnerWithGameState(gameState);

    if (winner)
    {
        int winnerNumber = winner->playerNumber();
        if (winnerNumber == playerNumber) return 1;
        else return -1;
    }
	
    return 0;
}

//
// Find the most optimal move by evaluating possible games stemming from that move
//
int ConnectFour::negamax(std::string gameState, int depth, int playerNumber)
{
	if (depth == 0 || checkForWinnerWithGameState(gameState)) return evaluate(gameState, playerNumber);
    std::vector<std::string> moves = generateMoves(gameState, playerNumber);
    if (moves.empty()) return 0;

    int value = -2;
    int nextPlayer = playerNumber == 0 ? 1 : 0;
    for (auto const & move : moves) value = std::max(value, -negamax(move, depth - 1, nextPlayer));
    return value;
}

//
// Negamax wrapper function to get the best move for the AI
//
std::string ConnectFour::getBestMove()
{
    std::string gameState = stateString();
    std::vector<std::string> moves = generateMoves(gameState, AI_PLAYER);
    std::string bestMove = "000000000";
    int bestEvaluation = -2;

    for (auto const & move : moves) 
    {
        int evaluation = -negamax(move, 9, HUMAN_PLAYER);

        Logger &logger = Logger::GetInstance();
        logger.Info("Checking move: " + move + " Evaluation: " + std::to_string(evaluation));

        if (evaluation > bestEvaluation) 
        {
            bestMove = move;
            bestEvaluation = evaluation;

            logger.Event("Chose a new best move: " + bestMove + " Evaluation: " + std::to_string(bestEvaluation));
        }
    }

    return bestMove;
}

//
// Called by the AI upon AI player's turn
//
void ConnectFour::updateAI()
{
    if (_gameOptions.gameOver) return;
    if (_gameOptions.AIPlaying) return;
    else
    {
        _gameOptions.AIPlaying = true;

        std::string bestMove = getBestMove();

        Logger &logger = Logger::GetInstance();
        logger.Info("Best AI move: " + bestMove);

        std::string gameState = stateString();
        for (size_t i = 0; i < gameState.length(); i++)
        {
            if (gameState[i] != bestMove[i])
            {
                // Place correct piece in this spot
                int rowX = i % 7;
                int rowY = findLowestOpenSquareY(rowX);
                ChessSquare *square  = _grid->getSquare(rowX, rowY);

                if (actionForEmptyHolder(*square)) 
                {
                    _gameOptions.AIPlaying = false;
                    endTurn();
                    logger.Event("AI placed a piece at (" + std::to_string(rowX) + ", " + std::to_string(rowY) + ")");

                }
                else
                {
                    logger.Error("updateAI(): Failed to place piece at (" + std::to_string(rowX) + ", " + std::to_string(rowY) + ")");
                }
                
                return;
            }
        }
    }
}