#pragma once
#include <string>
#include "BSAI25005-BSAI25008-Game.h"
#include "BSAI25005-BSAI25008-GameRenderer.h"
#include "BSAI25005-BSAI25008-InputHandler.h"

static const int SCREEN_WIDTH = 1900;
static const int SCREEN_HEIGHT = 1080;
static const float TURN_TIMER_MAX = 15.0f;
class GameController {
private:
    Game          game;
    GameRenderer  renderer;
    InputHandler  input;
    int         menuPlayers;
    int         menuPage;
    bool        waitingForNames;
    int         nameInputIdx;
    string playerNames[4];
    string typingBuffer;
    GameMode    menuMode;
    BoardTheme  menuTheme;
    float       turnTimer;
    void runWelcome();
    void runMenu();
    void runGame();
    void runReplay();
    void handleTokenClick();
    void doAutoMove();

    Rectangle diceRect;

public:
    void run();
};