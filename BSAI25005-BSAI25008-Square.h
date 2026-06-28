#include "BSAI25005-BSAI25008-Square.h"
#include "BSAI25005-BSAI25008-Token.h"
#include <algorithm>

Square::Square()
    : position(0), type(SquareType::NORMAL), colour(Colour::COL_NONE) {}

Square::Square(int pos, SquareType t, Colour c)
    : position(pos), type(t), colour(c) {}

void Square::addToken(Token* t) {
    if (t) occupants.push_back(t);
}

void Square::removeToken(Token* t) {
    auto it = find(occupants.begin(), occupants.end(), t);
    if (it != occupants.end()) occupants.erase(it);
}

bool Square::isEmpty() const { return occupants.empty(); }

bool Square::hasEnemy(Colour c) const {
    for (auto* t : occupants)
        if (t->getColour() != c) return true;
    return false;
}

int        Square::getPosition() const { return position; }
SquareType Square::getType()     const { return type; }
Colour     Square::getColour()   const { return colour; }

vector<Token*>& Square::getOccupants() { return occupants; }
