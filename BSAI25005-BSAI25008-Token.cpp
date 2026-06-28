#pragma once
#include <vector>
#include "BSAI25005-BSAI25008-Enums.h"

class Token;

class Square {
private:
    int         position;
    SquareType  type;
    Colour      colour;
    vector<Token*>  occupants;

public:
    Square();
    Square(int pos, SquareType t, Colour c);

    void addToken(Token* t);
    void removeToken(Token* t);
    bool isEmpty()           const;
    bool hasEnemy(Colour c)  const;
    int        getPosition() const;
    SquareType getType()     const;
    Colour     getColour()   const;

    vector<Token*>& getOccupants();
};
