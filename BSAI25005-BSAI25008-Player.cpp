#pragma once
#include "BSAI25005-BSAI25008-Enums.h"

struct MoveRecord {
    int playerIdx       = 0;
    int tokenId         = 0;
    int fromPos         = -1;
    int toPos           = -1;
    TokenState fromState       = TokenState::BASE;
    TokenState toState         = TokenState::BASE;
    int fromHomeSteps   = 0;
    int toHomeSteps     = 0;
    int rollValue       = 0;
    int capturedPlayer  = -1; 
    int capturedToken   = -1;
    int capturedFromPos = -1;

    bool wasCapture() const { return capturedToken != -1; }
};
