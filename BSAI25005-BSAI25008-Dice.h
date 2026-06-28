#pragma once
#include <vector>
#include "BSAI25005-BSAI25008-Square.h"
#include "BSAI25005-BSAI25008-Enums.h"

class Token;
class Board {
private:
    Square squares[76];  
    vector<int>  safePositions;
    int homeColStart[4];
public:
    Board();

    bool isSafe(int pos)                     const;
    bool isHomeColumn(int pos, Colour c)     const;

    vector<Token*> getOccupants(int pos);

    Square& getSquare(int pos);
    void    reset();
    int getStartPos(Colour c)     const;
    int getHomeColStart(Colour c) const;
    int getHomeColSquareIdx(Colour c, int step) const; 
};
