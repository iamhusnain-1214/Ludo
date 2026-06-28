#include "BSAI25005-BSAI25008-RuleEngine.h"
#include "BSAI25005-BSAI25008-Token.h"
#include "BSAI25005-BSAI25008-Board.h"
#include "BSAI25005-BSAI25008-Player.h"

bool RuleEngine::canRelease(int roll) {
    return roll == 6;
}

bool RuleEngine::isForfeit(int consecutiveSixes) {
    return consecutiveSixes >= 3;
}

bool RuleEngine::isSafeSquare(int pos, Board& b) {
    return b.isSafe(pos);
}

bool RuleEngine::grantExtraTurn(int roll, bool captured) {
    return (roll == 6 or captured);
}

int RuleEngine::computeLandingPos(Token& t, int roll, Board& b) {
    if (t.isFinished()) return -1;
   if (t.getState() == TokenState::BASE) {
        if (roll != 6) return -1;
        return b.getStartPos(t.getColour());
    }

    if (t.getState() == TokenState::HOME_COLUMN) {
        int newSteps = t.getStepsInHomeCol() + roll;
        if (newSteps > 6) return -1; 
        return newSteps;             
    }
    int homeEntry = b.getHomeColStart(t.getColour());
    int pos = t.getPosition();

    int cur = pos;
    for (int step = 1; step <= roll; step++) {
        cur = (cur + 1) % 52;

        if (cur == homeEntry) {
            int homeSteps = roll - step + 1; 

            if (homeSteps > 6) return -1;
            return homeSteps;
        }
    }

    return cur;
}

bool RuleEngine::wouldOvershoot(Token& t, int roll, Board& b) {
    if (t.getState() == TokenState::HOME_COLUMN) {
        return (t.getStepsInHomeCol() + roll) > 6;
    }
    return false;
}

bool RuleEngine::isValidMove(Token& t, int roll, Board& b) {
    if (t.isFinished()) return false;
    if (t.getState() == TokenState::BASE) return canRelease(roll);
    if (t.getState() == TokenState::HOME_COLUMN) return !wouldOvershoot(t, roll, b);
    return computeLandingPos(t, roll, b) != -1;
}

bool RuleEngine::isCapture(Token& t, int landingPos, Board& b) {
    if (b.isSafe(landingPos)) return false;
    auto occ = b.getOccupants(landingPos);
    for (auto* other : occ) {
        if (other->getColour() != t.getColour() and other->getState() == TokenState::ACTIVE)
            return true;
    }
    return false;
}

bool RuleEngine::hasAnyMove(Player& p, int roll, Board& b) {
    for (int i = 0; i < 4; i++) {
        if (isValidMove(p.getToken(i), roll, b)) return true;
    }
    return false;
}