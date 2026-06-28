#include "BSAI25005-BSAI25008-Board.h"
#include "BSAI25005-BSAI25008-Token.h"

Board::Board() {
    safePositions = { 0, 8, 13, 21, 25, 26, 34, 38, 39, 47, 51, 12 };
    homeColStart[0] = 51; 
    homeColStart[1] = 12; 
    homeColStart[2] = 25; 
    homeColStart[3] = 38; 

    reset();
}

void Board::reset() {
    for (int i = 0; i < 76; i++) {
        squares[i].getOccupants().clear();
        bool safe = false;
        for (int s : safePositions) if (i == s) { safe = true; break; }
        if (i < 52) {
            squares[i] = Square(i, safe ? SquareType::SAFE : SquareType::NORMAL, Colour::COL_NONE);
        } else {
            Colour c = Colour::COL_NONE;
            if      (i >= 52 and i <= 57) c = Colour::COL_RED;
            else if (i >= 58 and i <= 63) c = Colour::COL_GREEN;
            else if (i >= 64 and i <= 69) c = Colour::COL_YELLOW;
            else if (i >= 70 and i <= 75) c = Colour::COL_BLUE;
            squares[i] = Square(i, SquareType::HOME_COLUMN, c);
        }
    }
}

bool Board::isSafe(int pos) const {
    if (pos < 0 or pos >= 52) return true; 
    for (int s : safePositions) if (pos == s) return true;
    return false;
}

bool Board::isHomeColumn(int pos, Colour c) const {
    if (pos < 52 or pos > 75) return false;
    return squares[pos].getColour() == c;
}

vector<Token*> Board::getOccupants(int pos) {
    if (pos < 0 or pos >= 76) return {};
    return squares[pos].getOccupants();
}

Square& Board::getSquare(int pos) {
    if (pos < 0 or pos >= 76) return squares[0];
    return squares[pos];
}

int Board::getStartPos(Colour c) const {
    switch (c) {
        case Colour::COL_RED:    return 0;
        case Colour::COL_GREEN:  return 13;
        case Colour::COL_YELLOW: return 26;
        case Colour::COL_BLUE:   return 39;
        default:             return 0;
    }
}

int Board::getHomeColStart(Colour c) const {
    switch (c) {
        case Colour::COL_RED:    return homeColStart[0];
        case Colour::COL_GREEN:  return homeColStart[1];
        case Colour::COL_YELLOW: return homeColStart[2];
        case Colour::COL_BLUE:   return homeColStart[3];
        default:             return 0;
    }
}

int Board::getHomeColSquareIdx(Colour c, int step) const {
    int base = 0;
    switch (c) {
        case Colour::COL_RED:    base = 52; break;
        case Colour::COL_GREEN:  base = 58; break;
        case Colour::COL_YELLOW: base = 64; break;
        case Colour::COL_BLUE:   base = 70; break;
        default:             base = 52; break;
    }
    return base + step;
}
