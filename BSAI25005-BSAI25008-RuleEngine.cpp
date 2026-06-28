#pragma once
#include <vector>
#include <string>
#include "BSAI25005-BSAI25008-Token.h"
using namespace std;
class Board;

class Player {
private:
    string name;
    Colour colour;
    Token  tokens[4];
    int finishedCount;

public:
    Player(string n, Colour c);
    Token&    getToken(int id);
    const Token&   getToken(int id) const;
    bool   hasWon()  const;
    vector<int>    getMovableTokenIds(int roll, Board& b);
    string getName() const;
    Colour getColour()       const;
    int    getFinishedCount() const;
    void   recalcFinished();       
};
