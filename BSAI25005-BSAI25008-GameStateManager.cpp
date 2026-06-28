#pragma once
#include <vector>
#include <string>
#include "BSAI25005-BSAI25008-Enums.h"
#include "raylib.h"

class Board;
class Player;
class Game;

class GameRenderer {
private:
    int   screenW;
    int   screenH;
    float boardSize;
    float cell;
    Texture2D tokenTex[4];
    Texture2D diceTex[6];
    Texture2D bgTex;
    Texture2D boardTex;
    Font      customFont;
    bool      hasCustomFont;
    bool      hasTokenTex[4];
    bool      hasDiceTex[6];
    bool      hasBgTex;
    bool      hasBoardTex;
    float diceAnimTimer;
    bool  diceAnimating;
    int   diceAnimFrame;
    Color getPlayerColor(int playerIdx) const;
    Color getPlayerColorDark(int playerIdx) const;
    void  drawCell(int col, int row, Color fill, Color border) const;
    void  drawStar(float cx, float cy, float r, Color c) const;
    void  drawToken(Vector2 center, float radius, int playerIdx, int tokenId, bool highlight) const;
    void  drawDiceFace(Rectangle rect, int value, bool highlight) const;
    void  drawRoundedPanel(Rectangle r, Color fill, Color border, float roundness) const;
    void  drawTextCentered(const char* text, int x, int y, int w, int fontSize, Color c) const;
    void  drawButton(Rectangle r, const char* label, Color bg, Color fg, int fontSize) const;
    void  drawTimerBar(int sbX, int sbW, int y, float timer, float maxTimer) const;
    bool  tryLoadTexture(const char* path, Texture2D& out);
public:
    Sound diceSound;
    Sound killSound;
    Music bgMusic;
    void init(int w, int h);
    void close();
    void drawWelcome(float timer, float maxTime) const;
    void drawMenu(int selectedPlayers, bool waitingForName, int namePlayerIdx,
        const string playerNames[], const string& typingBuf,
        GameMode mode, BoardTheme theme) const;
    void drawMenuSettings(GameMode mode, BoardTheme theme) const;
    void drawGame(const Game& g, float turnTimer, float maxTimer) const;
    void drawWinScreen(const Player& p) const;
    void drawReplayScreen(const Game& g) const;
    Vector2 tokenScreenPos(int playerIdx, int tokenId,
        int logicPos, TokenState state, int homeSteps) const;
    float getCellSize()  const { return cell; }
    int   getScreenW()   const { return screenW; }
    int   getScreenH()   const { return screenH; }
    void tickDiceAnim(float dt);
    bool isDiceAnimating() const { return diceAnimating; }
    void startDiceAnim() { diceAnimating = true; diceAnimTimer = 0; diceAnimFrame = 1; }
};