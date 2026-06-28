#pragma once
#include <string>
#include "BSAI25005-BSAI25008-Enums.h"
#include "raylib.h"

class InputHandler {
public:
    UIEvent pollEvents(bool rolledThisTurn, bool inReplay, Rectangle diceRect);
    void    updateTyping(string& buf);
    bool    clickedCircle(Vector2 center, float radius);
    int     playerCountDelta(); 
};
