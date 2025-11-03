#include "ConnectFour.h"
#include "Logger.h"

Logger &logger = Logger::GetInstance();

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

int ConnectFour::findLowestOpenSquareYByGameState(std::string gameState, int x)
{
    int lowestY = 0;
    
    while (lowestY < ROWY - 1) {
        lowestY++;
        if (gameState[coordsToStateIndex(x, lowestY)] != '0') return lowestY - 1;
    }

    return lowestY;
}

void ConnectFour::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "square.png");

    if (gameHasAI()) setAIPlayer(RED_PLAYER); // AI will play second

    startGame();
}

bool ConnectFour::ownersAreTheSame(Player *owner1, Player *owner2, Player *owner3, Player *owner4)
{
    if (owner1 && owner2 && owner3 && owner4)
    {
        if (owner1 == owner2 && owner2 == owner3 && owner3 == owner4)
        {
            logger.Event("Player " + std::to_string(owner1->playerNumber()) + " won the game");
            _gameOptions.gameOver = true;
            return true;
        }   
    }

    return false;
}

bool ConnectFour::ownerNumbersAreTheSame(int owner1, int owner2, int owner3, int owner4)
{
    if (owner1 != 0 && owner2 != 0 && owner3 != 0 && owner4 != 0)
    {
        if (owner1 == owner2 && owner2 == owner3 && owner3 == owner4) 
        {
            return true;
        }
    }

    return false;
}

//
// Check all possible win conditions and return the winning player if there is one
//
Player* ConnectFour::checkForWinner() 
{
    Player *owner1 = nullptr;
    Player *owner2 = nullptr;
    Player *owner3 = nullptr;
    Player *owner4 = nullptr;

    // Check for winner by looking through 4x4 boxes
    for (int rowX = 0; rowX < ROWX - 3; rowX++)
    {
        for (int rowY = 0; rowY < ROWY - 3; rowY++)
        {
            // Check top line
            owner1 = ownerAt(rowX, rowY);
            owner2 = ownerAt(rowX + 1, rowY);
            owner3 = ownerAt(rowX + 2, rowY);
            owner4 = ownerAt(rowX + 3, rowY);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;

            // Check left line
            owner1 = ownerAt(rowX, rowY);
            owner2 = ownerAt(rowX, rowY + 1);
            owner3 = ownerAt(rowX, rowY + 2);
            owner4 = ownerAt(rowX, rowY + 3);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;

            // Check down diagonal
            owner1 = ownerAt(rowX, rowY);
            owner2 = ownerAt(rowX + 1, rowY + 1);
            owner3 = ownerAt(rowX + 2, rowY + 2);
            owner4 = ownerAt(rowX + 3, rowY + 3);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;

            // Check bottom line
            owner1 = ownerAt(rowX, rowY + 3);
            owner2 = ownerAt(rowX + 1, rowY + 3);
            owner3 = ownerAt(rowX + 2, rowY + 3);
            owner4 = ownerAt(rowX + 3, rowY + 3);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;

            // Check right line
            owner1 = ownerAt(rowX + 3, rowY);
            owner2 = ownerAt(rowX + 3, rowY + 1);
            owner3 = ownerAt(rowX + 3, rowY + 2);
            owner4 = ownerAt(rowX + 3, rowY + 3);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;
            
            // Check up diagonal
            owner1 = ownerAt(rowX, rowY + 3);
            owner2 = ownerAt(rowX + 1, rowY + 2);
            owner3 = ownerAt(rowX + 2, rowY + 1);
            owner4 = ownerAt(rowX + 3, rowY);
            if (ownersAreTheSame(owner1, owner2, owner3, owner4)) return owner1;
        }
    }

    return nullptr;
}

//
// Return true if the board is full and false otherwise
//
bool ConnectFour::checkForDraw()
{
    for (int rowX = 0; rowX < ROWX; rowX++)
    {
        for (int rowY = 0; rowY < ROWY; rowY++)
        {
            if (!_grid->getSquare(rowX, rowY)->bit()) return false;
        }
    }

    logger.Event("The game ended in a draw");
    _gameOptions.gameOver = true;
    return true;
}

std::string ConnectFour::initialStateString()
{
	return "000000000000000000000000000000000000000000";
}

//
// Convert the current game state to a 42-character state string representing each piece on the board
//
std::string ConnectFour::stateString() 
{
    std::string gameState = initialStateString();
    int stateIndex = 0;

    for (int rowX = 0; rowX < ROWX; rowX++)
    {
        for (int rowY = 0; rowY < ROWY; rowY++)
        {
            Bit *bit = _grid->getSquare(rowX, rowY)->bit();
            if (bit) gameState[stateIndex] = '1' + bit->getOwner()->playerNumber();
            stateIndex++;
        }
    }

    return gameState;
}

//
// Return the board to the state described by the state string
//
void ConnectFour::setStateString(const std::string &s)
{
    // TODO
}

//
// Handles placing pieces in empty spots
//
bool ConnectFour::actionForEmptyHolder(BitHolder &holder)
{
    if (_gameOptions.gameOver) return false;
    if (holder.bit()) {
        logger.Error("Holder already has a bit in it");
        return false;
    }
    ChessSquare *clickedSquare = dynamic_cast<ChessSquare*>(&holder);
    if (!clickedSquare) return false;

    // Place a piece for the current player at the lowest open square in that column
    int currentPlayerIndex = getCurrentPlayer()->playerNumber();
    Bit *bit = createPiece(currentPlayerIndex == 0 ? HUMAN_PLAYER : AI_PLAYER);
    
    if (bit) {
        int rowX = clickedSquare->getColumn();
        int rowY = findLowestOpenSquareY(rowX);
        ChessSquare *lowestSquareDown = _grid->getSquare(rowX, rowY);
        lowestSquareDown->setBit(bit);
        ImVec2 pos = lowestSquareDown->getPosition();
        bit->setPosition(pos);

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
    logger.Info("Generating moves from gameState " + gameState + " for Player " + std::to_string(playerNumber));

    for (int rowX = 0; rowX < ROWX; rowX++)
    {
        if (gameState[coordsToStateIndex(rowX, 0)] == '0')
        {
            int lowestY = findLowestOpenSquareYByGameState(gameState, rowX);
            std::string move = gameState;
            int moveIndex = coordsToStateIndex(rowX, lowestY);
            move[moveIndex] = '1' + playerNumber;
            moves.push_back(move);
            logger.Warn("Possible move for Player " + std::to_string(playerNumber) + ": " + move);
        }
    }
    
    return moves;
}

int ConnectFour::coordsToStateIndex(int x, int y)
{
    return x * ROWY + y;
}

//
// A win-checking function that uses the string game state rather than the current board (used for AI decision-making)
//
Player* ConnectFour::checkForWinnerWithGameState(std::string gameState)
{
    int owner1 = 0;
    int owner2 = 0;
    int owner3 = 0;
    int owner4 = 0;

    // Check for winner by looking through 4x4 boxes
    for (int rowX = 0; rowX < ROWX - 3; rowX++)
    {
        for (int rowY = 0; rowY < ROWY - 3; rowY++)
        {
            // Check top line
            owner1 = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX + 1, rowY)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX + 2, rowY)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);

            // Check left line
            owner1 = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX, rowY + 1)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX, rowY + 2)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);

            // Check down diagonal
            owner1 = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX + 1, rowY + 1)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX + 2, rowY + 2)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);

            // Check bottom line
            owner1 = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX + 1, rowY + 3)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX + 2, rowY + 3)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);

            // Check right line
            owner1 = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX + 3, rowY + 1)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX + 3, rowY + 2)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);
            
            // Check up diagonal
            owner1 = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            owner2 = gameState[coordsToStateIndex(rowX + 1, rowY + 2)] - '0';
            owner3 = gameState[coordsToStateIndex(rowX + 2, rowY + 1)] - '0';
            owner4 = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            if (ownerNumbersAreTheSame(owner1, owner2, owner3, owner4)) return getPlayerAt(owner1 - 1);
        }
    }

    return nullptr;
}

int ConnectFour::scoreOfLine(const int *owners, int playerNumber)
{
    int score = 0;

    for (size_t i = 0; i < 4; i++)
    {
        if (owners[i] == playerNumber) score++;
        else if (owners[i] != 0 && owners[i] != playerNumber)
        {
            score = 0;
            break;
        }
    }
    if (score == 1) score = 0;
    if (score == 3) score *= 5;

    return score;
}

//
// Returns a score for uncompleted rows from a given player's perspective
//
int ConnectFour::calculateScore(std::string gameState, int playerNumber)
{
    int owners[4] = { 0, 0, 0, 0 };

    int score = 0;

    // Check for winner by looking through 4x4 boxes
    for (int rowX = 0; rowX < ROWX - 3; rowX++)
    {
        for (int rowY = 0; rowY < ROWY - 3; rowY++)
        {
            // Check top line
            owners[0] = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX + 1, rowY)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX + 2, rowY)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            score += scoreOfLine(owners, playerNumber + 1); // Must add one because owners are 1 or 2

            // Check left line
            owners[0] = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX, rowY + 1)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX, rowY + 2)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            score += scoreOfLine(owners, playerNumber + 1);

            // Check down diagonal
            owners[0] = gameState[coordsToStateIndex(rowX, rowY)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX + 1, rowY + 1)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX + 2, rowY + 2)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            score += scoreOfLine(owners, playerNumber + 1);

            // Check bottom line
            owners[0] = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX + 1, rowY + 3)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX + 2, rowY + 3)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            score += scoreOfLine(owners, playerNumber + 1);

            // Check right line
            owners[0] = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX + 3, rowY + 1)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX + 3, rowY + 2)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX + 3, rowY + 3)] - '0';
            score += scoreOfLine(owners, playerNumber + 1);
            
            // Check up diagonal
            owners[0] = gameState[coordsToStateIndex(rowX, rowY + 3)] - '0';
            owners[1] = gameState[coordsToStateIndex(rowX + 1, rowY + 2)] - '0';
            owners[2] = gameState[coordsToStateIndex(rowX + 2, rowY + 1)] - '0';
            owners[3] = gameState[coordsToStateIndex(rowX + 3, rowY)] - '0';
            score += scoreOfLine(owners, playerNumber + 1);
        }
    }

    return score;
}

//
// Return the value of a move
//
int ConnectFour::evaluate(std::string gameState, int playerNumber)
{
	Player *winner = checkForWinnerWithGameState(gameState);

    if (winner)
    {
        logger.Info("Player " + std::to_string(winner->playerNumber()) + " wins in gameState " + gameState);
        int winnerNumber = winner->playerNumber();
        if (winnerNumber == playerNumber) return MAX_VALUE;
        else return -MAX_VALUE;
    }

    int score = 0;
    int opponentNumber = playerNumber == 0 ? 1 : 0;
    score += calculateScore(gameState, playerNumber);
    score -= calculateScore(gameState, opponentNumber);
	
    //logger.Info("Calculated score of " + std::to_string(score));
    return score;
}

//
// Find the most optimal move by evaluating possible games stemming from that move
//
int ConnectFour::negamax(std::string gameState, int depth, int playerNumber, int alpha, int beta)
{
	if (depth == 0 || checkForWinnerWithGameState(gameState)) 
    {
        int eval = evaluate(gameState, playerNumber);
        logger.Info("Reached end of negamax with gameState " + gameState + ". Evaluation = " + std::to_string(eval));
        return eval;
    }
    std::vector<std::string> moves = generateMoves(gameState, playerNumber);
    //if (moves.empty()) return 0;

    int value = -MAX_VALUE;
    int nextPlayer = playerNumber == 0 ? 1 : 0;
    for (auto const & move : moves)
    {
        value = std::max(value, -negamax(move, depth - 1, nextPlayer, -beta, -alpha));
        alpha = std::max(alpha, value);
        if (alpha >= beta) break;
    }
    return value;
}

//
// Negamax wrapper function to get the best move for the AI
//
std::string ConnectFour::getBestMove()
{
    std::string gameState = stateString();
    std::vector<std::string> moves = generateMoves(gameState, AI_PLAYER);
    std::string bestMove = "";
    int bestEvaluation = -MAX_VALUE;

    for (auto const & move : moves) 
    {
        int evaluation = -negamax(move, 1, HUMAN_PLAYER, -MAX_VALUE, MAX_VALUE);
        logger.Event("Checking move: " + move + " Evaluation: " + std::to_string(evaluation));

        if (evaluation > bestEvaluation) 
        {
            bestMove = move;
            bestEvaluation = evaluation;
            logger.Event("AI chose a new best move: " + bestMove + " Evaluation: " + std::to_string(bestEvaluation));
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

    std::string bestMove = getBestMove();
    logger.Info("Best AI move: " + bestMove);

    std::string gameState = stateString();
    for (size_t i = 0; i < gameState.length(); i++)
    {
        if (gameState[i] != bestMove[i])
        {
            // Place correct piece in this spot
            int rowX = i / ROWY;
            int rowY = findLowestOpenSquareY(rowX);
            ChessSquare *square  = _grid->getSquare(rowX, rowY);

            if (actionForEmptyHolder(*square))
            {
            }
            else
            {
                logger.Error("updateAI(): Failed to place piece at (" + std::to_string(rowX) + ", " + std::to_string(rowY) + ")");
            }
            
            break;
        }
    }
}