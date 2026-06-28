#pragma once
#include <cstdlib>
#include <ctime>

class Dice {
private:
    int lastRoll;

public:
    Dice() : lastRoll(1) { srand(time(0)); }
    int roll()          { lastRoll = (rand() % 6) + 1; return lastRoll; }
    int getLast() const { return lastRoll; }
    void setLast(int v) { if (v >= 1 and v <= 6) lastRoll = v; }
};
