#include "BSAI25005-BSAI25008-Player.h"
#include "BSAI25005-BSAI25008-Board.h"
#include "BSAI25005-BSAI25008-RuleEngine.h"
using namespace std;
Player::Player(string n, Colour c)
    : name(n), colour(c), finishedCount(0),
      tokens{ Token(0, c), Token(1, c), Token(2, c), Token(3, c) }
{}

Token& Player::getToken(int id) {
    if (id >= 0 and id < 4) return tokens[id];
    return tokens[0];
}

const Token& Player::getToken(int id) const {
    if (id >= 0 and id < 4) return tokens[id];
    return tokens[0];
}

bool Player::hasWon() const {
    for (int i = 0; i < 4; i++)
        if (!tokens[i].isFinished()) return false;
    return true;
}

vector<int> Player::getMovableTokenIds(int roll, Board& b) {
    vector<int> result;
    for (int i = 0; i < 4; i++)
        if (RuleEngine::isValidMove(const_cast<Token&>(tokens[i]), roll, b))
            result.push_back(i);
    return result;
}

string Player::getName()  const { return name; }
Colour Player::getColour() const { return colour; }
int Player::getFinishedCount() const { return finishedCount; }

void Player::recalcFinished() {
    finishedCount = 0;
    for (int i = 0; i < 4; i++)
        if (tokens[i].isFinished()) finishedCount++;
}
