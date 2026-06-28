#pragma once
#include "BSAI25005-BSAI25008-Enums.h"

class Token;
class Board;
class Player;

class RuleEngine {
public:
    static bool isValidMove(Token& t, int roll, Board& b);
    static bool canRelease(int roll);
    static bool wouldOvershoot(Token& t, int roll, Board& b);
    static bool isCapture(Token& t, int landingPos, Board& b);
    static bool grantExtraTurn(int roll, bool captured);
    static bool isForfeit(int consecutiveSixes);
    static bool hasAnyMove(Player& p, int roll, Board& b);
    static bool isSafeSquare(int pos, Board& b);
    static int computeLandingPos(Token& t, int roll, Board& b);
};
