#include "BSAI25005-BSAI25008-GameRenderer.h"
#include "BSAI25005-BSAI25008-Board.h"
#include "BSAI25005-BSAI25008-Player.h"
#include "BSAI25005-BSAI25008-Game.h"
#include <cmath>
#include <string>
#include <cstdio>
static const int LUDO_PATH[52][2] = {
    {1,6},{2,6},{3,6},{4,6},{5,6},
    {6,5},{6,4},{6,3},{6,2},{6,1},{6,0},
    {7,0},{8,0},
    {8,1},{8,2},{8,3},{8,4},{8,5},
    {9,6},{10,6},{11,6},{12,6},{13,6},{14,6},
    {14,7},{14,8},
    {13,8},{12,8},{11,8},{10,8},{9,8},
    {8,9},{8,10},{8,11},{8,12},{8,13},{8,14},
    {7,14},{6,14},
    {6,13},{6,12},{6,11},{6,10},{6,9},
    {5,8},{4,8},{3,8},{2,8},{1,8},{0,8},
    {0,7},{0,6}
};

static const int HOME_PATH[4][6][2] = {
    {{1,7},{2,7},{3,7},{4,7},{5,7},{6,7}},
    {{7,1},{7,2},{7,3},{7,4},{7,5},{7,6}},
    {{13,7},{12,7},{11,7},{10,7},{9,7},{8,7}},
    {{7,13},{7,12},{7,11},{7,10},{7,9},{7,8}}
};

static const float BASE_OFFSETS[4][2] = {
    {1.5f, 1.5f}, {3.5f, 1.5f}, {1.5f, 3.5f}, {3.5f, 3.5f}
};
Color GameRenderer::getPlayerColor(int idx) const {
    const Color COLS[4] = {
        {230,60,60,255},
        {60,200,90,255},
        {250,210,30,255},
        {60,130,235,255}
    };
    if (idx < 0 or idx > 3) return GRAY;
    return COLS[idx];
}

Color GameRenderer::getPlayerColorDark(int idx) const {
    const Color COLS[4] = {
        {140,20,20,255},
        {20,120,40,255},
        {170,140,10,255},
        {20,70,160,255}
    };
    if (idx < 0 or idx > 3) return DARKGRAY;
    return COLS[idx];
}

bool GameRenderer::tryLoadTexture(const char* path, Texture2D& out) {
    if (FileExists(path)) { out = LoadTexture(path); return true; }
    return false;
}

void GameRenderer::init(int w, int h) {
    screenW = w;
    screenH = h;
    boardSize = (float)h;
    cell = boardSize / 15.0f;

    InitWindow(w, h, "Double Trouble's Ludo Game");
    SetTargetFPS(60);
    hasCustomFont = false;
    if (FileExists("assets/Myfont.ttf")) {
        customFont = LoadFontEx("assets/Myfont.ttf", 64, nullptr, 0);
        hasCustomFont = true;
    }
    hasBoardTex = tryLoadTexture("assets/board.png", boardTex);
    const char* tokFiles[4] = {
        "assets/token_red.png","assets/token_green.png",
        "assets/token_yellow.png","assets/token_blue.png"
    };
    for (int i = 0; i < 4; i++)
        hasTokenTex[i] = tryLoadTexture(tokFiles[i], tokenTex[i]);
    char dp[32];
    for (int i = 0; i < 6; i++) {
        sprintf_s(dp, "assets/dice_%d.png", i + 1);
        hasDiceTex[i] = tryLoadTexture(dp, diceTex[i]);
    }

    hasBgTex = tryLoadTexture("assets/background.png", bgTex);
    diceAnimating = false;
    diceAnimTimer = 0;
    diceAnimFrame = 1;
    InitAudioDevice();
    diceSound = LoadSound("assets/dice.mp3");
    killSound = LoadSound("assets/kill.mp3");
    bgMusic = LoadMusicStream("assets/bg.mp3");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.4f);

}

void GameRenderer::close() {
    if (hasCustomFont)  UnloadFont(customFont);
    for (int i = 0; i < 4; i++) if (hasTokenTex[i]) UnloadTexture(tokenTex[i]);
    for (int i = 0; i < 6; i++) if (hasDiceTex[i])  UnloadTexture(diceTex[i]);
    if (hasBgTex)    UnloadTexture(bgTex);
    if (hasBoardTex) UnloadTexture(boardTex);
    CloseWindow();
}

void GameRenderer::tickDiceAnim(float dt) {
    UpdateMusicStream(bgMusic);
    if (!diceAnimating) return;
    diceAnimTimer += dt;
    diceAnimFrame = (diceAnimFrame % 6) + 1;
    if (diceAnimTimer >= 0.6f) diceAnimating = false;
}

void GameRenderer::drawRoundedPanel(Rectangle r, Color fill, Color border, float roundness) const {
    DrawRectangleRounded(r, roundness, 8, fill);
    DrawRectangleRoundedLines(r, roundness, 8, border);
}

void GameRenderer::drawTextCentered(const char* text, int x, int y, int w, int fontSize, Color c) const {
    if (hasCustomFont) {
        Vector2 sz = MeasureTextEx(customFont, text, (float)fontSize, 1);
        DrawTextEx(customFont, text, { x + (w - sz.x) / 2.0f, (float)y }, (float)fontSize, 1, c);
    }
    else {
        int tw = MeasureText(text, fontSize);
        DrawText(text, x + (w - tw) / 2, y, fontSize, c);
    }
}

void GameRenderer::drawButton(Rectangle r, const char* label, Color bg, Color fg, int fontSize) const {
    DrawRectangleRounded({ r.x + 3, r.y + 3, r.width, r.height }, 0.3f, 8, { 0,0,0,60 });
    DrawRectangleRounded(r, 0.3f, 8, bg);
    DrawRectangleRoundedLines(r, 0.3f, 8, { 255,255,255,60 });
    drawTextCentered(label, (int)r.x, (int)(r.y + r.height / 2 - fontSize / 2), (int)r.width, fontSize, fg);
}

void GameRenderer::drawTimerBar(int sbX, int sbW, int y, float timer, float maxTimer) const {
    float ratio = timer / maxTimer;
    if (ratio < 0) ratio = 0;
    int barW = sbW - 20;
    DrawRectangleRounded({ (float)(sbX + 10),(float)y,(float)barW,14 }, 0.5f, 8, { 40,38,60,255 });
    Color barCol = ratio > 0.4f ? Color{ 80,220,100,255 } : Color{ 240,80,60,255 };
    DrawRectangleRounded({ (float)(sbX + 10),(float)y,(float)(barW * ratio),14 }, 0.5f, 8, barCol);
    DrawRectangleRoundedLines({ (float)(sbX + 10),(float)y,(float)barW,14 }, 0.5f, 8, { 100,90,140,255 });

    char tbuf[16];
    sprintf_s(tbuf, "%.0fs", timer > 0 ? timer : 0.0f);
    int tw = MeasureText(tbuf, 11);
    DrawText(tbuf, sbX + sbW / 2 - tw / 2, y + 1, 11, WHITE);
}

void GameRenderer::drawCell(int col, int row, Color fill, Color border) const {
    float x = col * cell;
    float y = row * cell;
    DrawRectangleV({ x, y }, { cell, cell }, fill);
    DrawRectangleLines((int)x, (int)y, (int)cell, (int)cell, border);
}

void GameRenderer::drawStar(float cx, float cy, float r, Color c) const {
    const int pts = 5;
    for (int i = 0; i < pts * 2; i++) {
        float a1 = (float)i * 3.14159f / pts - 3.14159f / 2;
        float a2 = (float)(i + 1) * 3.14159f / pts - 3.14159f / 2;
        float r1 = (i % 2 == 0) ? r : r * 0.45f;
        float r2 = (i % 2 == 0) ? r * 0.45f : r;
        DrawTriangle({ cx, cy },
            { cx + r1 * cosf(a1), cy + r1 * sinf(a1) },
            { cx + r2 * cosf(a2), cy + r2 * sinf(a2) }, c);
    }
}

void GameRenderer::drawDiceFace(Rectangle rect, int value, bool highlight) const {
    DrawRectangleRounded({ rect.x + 4, rect.y + 4, rect.width, rect.height }, 0.18f, 8, { 0,0,0,60 });
    Color bodyCol = highlight ? Color{ 255,250,220,255 } : WHITE;
    DrawRectangleRounded(rect, 0.18f, 8, bodyCol);
    DrawRectangleRoundedLines(rect, 0.18f, 8, highlight ? ORANGE : DARKGRAY);

    if (value < 1 or value > 6) {
        int fs = (int)(rect.width * 0.55f);
        DrawText("?", (int)(rect.x + rect.width / 2 - fs / 4), (int)(rect.y + rect.height / 2 - fs / 2), fs, DARKGRAY);
        return;
    }
    if (hasDiceTex[value - 1]) {
        DrawTexturePro(diceTex[value - 1],
            { 0,0,(float)diceTex[value - 1].width,(float)diceTex[value - 1].height },
            { rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8 }, { 0,0 }, 0, WHITE);
        return;
    }
    float dotR = rect.width * 0.10f;
    auto dot = [&](float fx, float fy) {
        DrawCircle((int)(rect.x + fx * rect.width), (int)(rect.y + fy * rect.height), (int)dotR, { 40,40,40,255 });
        };
    switch (value) {
    case 1: dot(0.5f, 0.5f); break;
    case 2: dot(0.28f, 0.28f); dot(0.72f, 0.72f); break;
    case 3: dot(0.28f, 0.28f); dot(0.5f, 0.5f); dot(0.72f, 0.72f); break;
    case 4: dot(0.28f, 0.28f); dot(0.72f, 0.28f); dot(0.28f, 0.72f); dot(0.72f, 0.72f); break;
    case 5: dot(0.28f, 0.28f); dot(0.72f, 0.28f); dot(0.5f, 0.5f); dot(0.28f, 0.72f); dot(0.72f, 0.72f); break;
    case 6: dot(0.28f, 0.22f); dot(0.72f, 0.22f); dot(0.28f, 0.5f); dot(0.72f, 0.5f); dot(0.28f, 0.78f); dot(0.72f, 0.78f); break;
    }
}

void GameRenderer::drawToken(Vector2 center, float radius, int playerIdx, int tokenId, bool highlight) const {
    Color col = getPlayerColor(playerIdx);
    Color darkCol = getPlayerColorDark(playerIdx);

    if (highlight)
        DrawCircle((int)center.x, (int)center.y, radius + 6, { 255,255,100,200 });

    if (hasTokenTex[playerIdx]) {
        float sz = radius * 2.3f;
        DrawTexturePro(tokenTex[playerIdx],
            { 0,0,(float)tokenTex[playerIdx].width,(float)tokenTex[playerIdx].height },
            { center.x - sz / 2, center.y - sz / 2, sz, sz }, { 0,0 }, 0, WHITE);
        return;
    }

    DrawCircle((int)(center.x + 2), (int)(center.y + 3), (int)radius, { 0,0,0,60 });
    DrawCircleV(center, radius, col);
    DrawCircleV({ center.x - radius * 0.25f, center.y - radius * 0.25f }, radius * 0.35f, { 255,255,255,80 });
    DrawCircleLines((int)center.x, (int)center.y, (int)radius, darkCol);
    char buf[4]; sprintf_s(buf, "%d", tokenId + 1);
    int fs = (int)(radius * 1.0f);
    int tw = MeasureText(buf, fs);
    DrawText(buf, (int)(center.x - tw / 2), (int)(center.y - fs / 2), fs, WHITE);
}

Vector2 GameRenderer::tokenScreenPos(int playerIdx, int tokenId,
    int logicPos, TokenState state, int homeSteps) const {
    float half = cell / 2.0f;
    if (state == TokenState::BASE) {
        float ox = BASE_OFFSETS[tokenId][0];
        float oy = BASE_OFFSETS[tokenId][1];
        float qx[4] = { 0, 9, 9, 0 };
        float qy[4] = { 0, 0, 9, 9 };
        return { (qx[playerIdx] + ox) * cell, (qy[playerIdx] + oy) * cell };
    }
    if (state == TokenState::ACTIVE) {
        if (logicPos < 0 or logicPos >= 52) return { 0,0 };
        return { LUDO_PATH[logicPos][0] * cell + half, LUDO_PATH[logicPos][1] * cell + half };
    }
    if (state == TokenState::HOME_COLUMN) {
        int step = homeSteps - 1;
        if (step < 0) step = 0;
        if (step > 5) step = 5;
        return { HOME_PATH[playerIdx][step][0] * cell + half, HOME_PATH[playerIdx][step][1] * cell + half };
    }
    float cx = 7 * cell + half, cy = 7 * cell + half;
    float dx[4] = { -half * 0.4f, half * 0.4f, -half * 0.4f, half * 0.4f };
    float dy[4] = { -half * 0.4f, -half * 0.4f, half * 0.4f, half * 0.4f };
    return { cx + dx[playerIdx], cy + dy[playerIdx] };
}

void GameRenderer::drawWelcome(float timer, float maxTime) const {
    DrawRectangle(0, 0, screenW, screenH, { 18, 14, 35, 255 });
    DrawCircle(screenW / 4, screenH / 4, 200, { 220,60,60,40 });
    DrawCircle(screenW * 3 / 4, screenH / 4, 180, { 60,200,90,40 });
    DrawCircle(screenW / 4, screenH * 3 / 4, 190, { 60,130,235,40 });
    DrawCircle(screenW * 3 / 4, screenH * 3 / 4, 210, { 250,210,30,40 });

    int cx = screenW / 2;
    int cy = screenH / 2;

    const char* line1 = "Double Trouble's";
    const char* line2 = "LUDO GAME";
    const char* sub = "Welcome!";
    float pulse = 1.0f + 0.04f * sinf(timer * 3.0f);
    int titleSize = (int)(72 * pulse);
    int line1Size = 36;
    int tw2 = MeasureText(line2, titleSize);
    DrawText(line2, cx - tw2 / 2 + 5, cy - 60 + 5, titleSize, { 0,0,0,80 });
    DrawText(line2, cx - tw2 / 2 + 2, cy - 60 + 2, titleSize, { 180,100,255,120 });
    DrawText(line2, cx - tw2 / 2, cy - 60, titleSize, { 240,210,255,255 });
    int tw1 = MeasureText(line1, line1Size);
    DrawText(line1, cx - tw1 / 2, cy - 110, line1Size, { 200,180,255,255 });
    int tsub = MeasureText(sub, 28);
    DrawText(sub, cx - tsub / 2, cy + 40, 28, { 255,215,80,255 });
    float alpha = (sinf(timer * 4.0f) + 1.0f) * 0.5f * 255;
    const char* hint = "Press ENTER or Click to Start";
    int thint = MeasureText(hint, 20);
    DrawText(hint, cx - thint / 2, cy + 100, 20, { 180,170,230,(unsigned char)alpha });
    int barW = 400;
    float ratio = timer / maxTime;
    DrawRectangleRounded({ (float)(cx - barW / 2), (float)(screenH - 60), (float)barW, 10 }, 0.5f, 8, { 40,38,60,255 });
    DrawRectangleRounded({ (float)(cx - barW / 2), (float)(screenH - 60), (float)(barW * ratio), 10 }, 0.5f, 8, { 200,160,255,255 });
}
void GameRenderer::drawMenu(int selectedPlayers, bool waitingForName, int namePlayerIdx,
    const string playerNames[], const string& typingBuf,
    GameMode mode, BoardTheme theme) const {

    DrawRectangle(0, 0, screenW, screenH, { 18,14,35,255 });
    DrawCircle(0, 0, 300, { 220,60,60,25 });
    DrawCircle(screenW, 0, 300, { 60,200,90,25 });
    DrawCircle(0, screenH, 300, { 60,130,235,25 });
    DrawCircle(screenW, screenH, 300, { 250,210,30,25 });

    int cx = screenW / 2;
    int cy = screenH / 2;

    Rectangle titlePanel = { (float)(cx - 280), 40, 560, 110 };
    DrawRectangleRounded(titlePanel, 0.3f, 10, { 30,20,55,200 });
    DrawRectangleRoundedLines(titlePanel, 0.3f, 10, { 180,130,255,255 });

    const char* title = "Double Trouble's LUDO";
    int titleSz = 44;
    if (hasCustomFont) {
        Vector2 tsz = MeasureTextEx(customFont, title, (float)titleSz, 1);
        DrawTextEx(customFont, title, { cx - tsz.x / 2 + 3, 58 }, (float)titleSz, 1, { 100,60,160,255 });
        DrawTextEx(customFont, title, { cx - tsz.x / 2,     55 }, (float)titleSz, 1, { 220,190,255,255 });
    }
    else {
        int tw = MeasureText(title, titleSz);
        DrawText(title, cx - tw / 2 + 3, 58, titleSz, { 100,60,160,255 });
        DrawText(title, cx - tw / 2, 55, titleSz, { 220,190,255,255 });
    }

    DrawLineEx({ (float)(cx - 260), 175 }, { (float)(cx + 260), 175 }, 2, { 140,100,220,180 });

    if (!waitingForName) {
        const char* lbl = "How many players?";
        int lw = MeasureText(lbl, 22);
        DrawText(lbl, cx - lw / 2, cy - 150, 22, { 200,185,240,255 });

        Color btnColors[] = { {220,60,60,255},{60,200,90,255},{250,210,30,255},{60,130,235,255} };
        for (int n = 2; n <= 4; n++) {
            bool sel = (n == selectedPlayers);
            Rectangle btn = { (float)(cx - 90 + (n - 3) * 200), (float)(cy - 80), 160, 70 };
            Color bg = sel ? btnColors[n - 1] : Color{ 45,42,75,255 };
            Color bd = sel ? Color{ 255,255,255,200 } : Color{ 100,90,150,255 };
            DrawRectangleRounded({ btn.x + 3, btn.y + 3, btn.width, btn.height }, 0.3f, 8, { 0,0,0,60 });
            DrawRectangleRounded(btn, 0.3f, 8, bg);
            DrawRectangleRoundedLines(btn, 0.3f, 8, bd);
            char buf[4]; sprintf_s(buf, "%d", n);
            drawTextCentered(buf, (int)btn.x, (int)(btn.y + 16), (int)btn.width, 36, sel ? WHITE : Color{ 160,150,210,255 });
        }

        Rectangle newGameBtn = { (float)(cx - 160), (float)(cy + 30), 320, 70 };
        drawButton(newGameBtn, "NEW GAME", { 50,180,80,255 }, WHITE, 28);

        Rectangle loadBtn = { (float)(cx - 160), (float)(cy + 120), 320, 60 };
        drawButton(loadBtn, "LOAD GAME", { 50,100,200,255 }, WHITE, 24);

        Rectangle settingsBtn = { (float)(cx - 160), (float)(cy + 200), 320, 52 };
        drawButton(settingsBtn, "GAME SETTINGS", { 80,60,140,255 }, { 200,185,255,255 }, 20);

        const char* hint = "Select players, then NEW GAME";
        int hw = MeasureText(hint, 15);
        DrawText(hint, cx - hw / 2, screenH - 55, 15, { 140,130,180,190 });
    }
    else {
        const char* enterLbl = "Enter Player Names";
        int elw = MeasureText(enterLbl, 30);
        DrawText(enterLbl, cx - elw / 2, cy - 200, 30, { 220,190,255,255 });

        Color pc = getPlayerColor(namePlayerIdx);
        const char* colNames[] = { "RED","GREEN","YELLOW","BLUE" };
        DrawCircle(cx, cy - 135, 28, pc);
        char plLbl[32]; sprintf_s(plLbl, "Player %d  (%s)", namePlayerIdx + 1, colNames[namePlayerIdx]);
        int plW = MeasureText(plLbl, 20);
        DrawText(plLbl, cx - plW / 2, cy - 96, 20, pc);

        for (int i = 0; i < namePlayerIdx; i++) {
            Color ic = getPlayerColor(i);
            char nb[48]; sprintf_s(nb, "P%d: %s", i + 1, playerNames[i].c_str());
            int nbW = MeasureText(nb, 18);
            DrawCircle(cx - nbW / 2 - 16, cy - 60 + i * 30 + 9, 8, ic);
            DrawText(nb, cx - nbW / 2, cy - 60 + i * 30, 18, { 200,190,230,255 });
        }

        Rectangle inputBox = { (float)(cx - 180), (float)(cy + 60), 360, 52 };
        DrawRectangleRounded(inputBox, 0.2f, 8, { 35,32,58,255 });
        DrawRectangleRoundedLines(inputBox, 0.2f, 8, pc);
        string display = typingBuf + "_";
        if (hasCustomFont)
            DrawTextEx(customFont, display.c_str(), { inputBox.x + 14, inputBox.y + 12 }, 26, 1, WHITE);
        else
            DrawText(display.c_str(), (int)(inputBox.x + 14), (int)(inputBox.y + 14), 24, WHITE);

        if (typingBuf.empty()) {
            const char* hint2 = "Name cannot be empty!";
            int h2w = MeasureText(hint2, 14);
            DrawText(hint2, cx - h2w / 2, cy + 122, 14, { 240,100,100,255 });
        }
        else {
            const char* hint2 = "Press ENTER to confirm";
            int h2w = MeasureText(hint2, 14);
            DrawText(hint2, cx - h2w / 2, cy + 122, 14, { 160,150,220,200 });
        }
        DrawText("Press ESC to go back", 20, screenH - 40, 14, { 120,110,160,180 });
    }
}

void GameRenderer::drawMenuSettings(GameMode mode, BoardTheme theme) const {
    DrawRectangle(0, 0, screenW, screenH, { 18,14,35,255 });
    DrawCircle(0, 0, 300, { 220,60,60,20 });
    DrawCircle(screenW, 0, 300, { 60,200,90,20 });
    DrawCircle(0, screenH, 300, { 60,130,235,20 });
    DrawCircle(screenW, screenH, 300, { 250,210,30,20 });

    int cx = screenW / 2;
    int cy = screenH / 2;

    Rectangle titlePanel = { (float)(cx - 260), 40, 520, 110 };
    DrawRectangleRounded(titlePanel, 0.3f, 10, { 30,20,55,200 });
    DrawRectangleRoundedLines(titlePanel, 0.3f, 10, { 180,130,255,255 });
    const char* title = "Game Settings";
    int tw = MeasureText(title, 44);
    DrawText(title, cx - tw / 2 + 3, 58, 44, { 100,60,160,255 });
    DrawText(title, cx - tw / 2, 55, 44, { 220,190,255,255 });

    DrawLineEx({ (float)(cx - 260), 175 }, { (float)(cx + 260), 175 }, 2, { 140,100,220,180 });

    const char* modeLbl = "Game Mode";
    int mlw = MeasureText(modeLbl, 22);
    DrawText(modeLbl, cx - mlw / 2, cy - 120, 22, { 200,185,240,255 });

    Rectangle classicBtn = { (float)(cx - 180), (float)(cy - 60), 160, 52 };
    Rectangle lockedBtn = { (float)(cx + 20),  (float)(cy - 60), 160, 52 };
    drawButton(classicBtn, "CLASSIC", (mode == GameMode::CLASSIC) ? Color{ 50,180,80,255 } : Color{ 45,42,75,255 }, WHITE, 20);
    drawButton(lockedBtn, "LOCKED", (mode == GameMode::LOCKED) ? Color{ 200,80,50,255 } : Color{ 45,42,75,255 }, WHITE, 20);

    const char* modeDesc = (mode == GameMode::CLASSIC) ? "Classic: tokens can go home freely" : "Locked: token must kill to go home";
    Color modeDescCol = (mode == GameMode::CLASSIC) ? Color{ 140,220,140,200 } : Color{ 255,180,80,220 };
    int mdw = MeasureText(modeDesc, 15);
    DrawText(modeDesc, cx - mdw / 2, cy + 8, 15, modeDescCol);

    DrawLineEx({ (float)(cx - 260), float(cy) + 40 }, { (float)(cx + 260), float(cy) + 40 }, 1, { 80,70,120,120 });

    const char* themeLbl = "Board Theme";
    int tlw = MeasureText(themeLbl, 22);
    DrawText(themeLbl, cx - tlw / 2, cy + 58, 22, { 200,185,240,255 });

    const char* themeNames[] = { "DEFAULT","OCEAN","FOREST","SUNSET" };
    Color themeHighlights[] = {
        {200,180,255,255},{60,160,220,255},{50,160,80,255},{240,140,50,255}
    };
    for (int th = 0; th < 4; th++) {
        Rectangle thBtn = { (float)(cx - 330 + th * 170), (float)(cy + 100), 150, 48 };
        bool sel = ((int)theme == th);
        drawButton(thBtn, themeNames[th], sel ? themeHighlights[th] : Color{ 45,42,75,255 }, WHITE, 16);
    }

    Rectangle backBtn = { (float)(cx - 160), (float)(cy + 180), 320, 52 };
    drawButton(backBtn, "BACK TO MENU", { 60,55,100,255 }, { 200,185,255,255 }, 20);

    DrawText("ESC to go back", 20, screenH - 40, 14, { 120,110,160,180 });
}

void GameRenderer::drawGame(const Game& g, float turnTimer, float maxTimer) const {
    if (hasBgTex) {
        DrawTexturePro(bgTex, { 0,0,(float)bgTex.width,(float)bgTex.height },
            { 0,0,(float)screenW,(float)screenH }, { 0,0 }, 0, WHITE);
    }
    else {
        DrawRectangle(0, 0, screenW, screenH, { 22,18,40,255 });
    }
    if (hasBoardTex) {
        DrawTexturePro(boardTex,
            { 0,0,(float)boardTex.width,(float)boardTex.height },
            { 0,0,boardSize,boardSize }, { 0,0 }, 0, WHITE);
    }
    else {
        Color baseColors[4] = {
            {230,70,70,255},{70,200,100,255},{240,215,50,255},{70,140,230,255}
        };
        Color baseLightColors[4] = {
            {250,185,185,255},{185,245,205,255},{250,242,185,255},{185,215,252,255}
        };

        BoardTheme theme = g.getBoardTheme();
        if (theme == BoardTheme::OCEAN) {
            baseColors[0] = { 200,60,100,255 }; baseColors[1] = { 40,160,200,255 };
            baseColors[2] = { 50,200,190,255 }; baseColors[3] = { 30,80,180,255 };
            baseLightColors[0] = { 255,180,210,255 }; baseLightColors[1] = { 160,230,255,255 };
            baseLightColors[2] = { 180,245,240,255 }; baseLightColors[3] = { 160,200,255,255 };
        }
        else if (theme == BoardTheme::FOREST) {
            baseColors[0] = { 180,60,60,255 }; baseColors[1] = { 40,130,50,255 };
            baseColors[2] = { 160,130,40,255 }; baseColors[3] = { 60,100,60,255 };
            baseLightColors[0] = { 240,200,190,255 }; baseLightColors[1] = { 185,230,195,255 };
            baseLightColors[2] = { 230,220,175,255 }; baseLightColors[3] = { 185,215,190,255 };
        }
        else if (theme == BoardTheme::SUNSET) {
            baseColors[0] = { 220,80,40,255 }; baseColors[1] = { 200,140,40,255 };
            baseColors[2] = { 180,60,100,255 }; baseColors[3] = { 140,60,160,255 };
            baseLightColors[0] = { 255,210,185,255 }; baseLightColors[1] = { 255,235,185,255 };
            baseLightColors[2] = { 255,185,210,255 }; baseLightColors[3] = { 220,185,245,255 };
        }

        DrawRectangle(0, 0, (int)boardSize, (int)boardSize, { 248,244,235,255 });

        for (int row = 0; row < 15; row++) {
            for (int col = 0; col < 15; col++) {
                Color fill = { 248,244,235,255 };
                Color border = { 190,180,168,255 };

                if (row < 6 and col < 6) { fill = baseColors[0]; border = getPlayerColorDark(0); }
                else if (row < 6 and col > 8) { fill = baseColors[1]; border = getPlayerColorDark(1); }
                else if (row > 8 and col > 8) { fill = baseColors[2]; border = getPlayerColorDark(2); }
                else if (row > 8 and col < 6) { fill = baseColors[3]; border = getPlayerColorDark(3); }
                else if (row == 7 and col > 0 and col < 6)  fill = baseLightColors[0];
                else if (col == 7 and row > 0 and row < 6)  fill = baseLightColors[1];
                else if (row == 7 and col > 8 and col < 14) fill = baseLightColors[2];
                else if (col == 7 and row > 8 and row < 14) fill = baseLightColors[3];
                else if (row == 6 and col == 1)  fill = baseColors[0];
                else if (row == 1 and col == 8)  fill = baseColors[1];
                else if (row == 8 and col == 13) fill = baseColors[2];
                else if (row == 13 and col == 6)  fill = baseColors[3];
                else if (row == 8 and col == 2)  fill = baseColors[0];
                else if (row == 2 and col == 6)  fill = baseColors[1];
                else if (row == 6 and col == 12) fill = baseColors[2];
                else if (row == 12 and col == 8) fill = baseColors[3];

                else if (row >= 6 and row <= 8 and col >= 6 and col <= 8) {
                    fill = { 45,42,70,255 }; border = { 20,18,40,255 };
                
                }
                drawCell(col, row, fill, border);
            }
        }
        float baseInnerR = cell * 2.0f;
        DrawCircle((int)(3 * cell), (int)(3 * cell), baseInnerR, baseLightColors[0]);
        DrawCircle((int)(12 * cell), (int)(3 * cell), baseInnerR, baseLightColors[1]);
        DrawCircle((int)(12 * cell), (int)(12 * cell), baseInnerR, baseLightColors[2]);
        DrawCircle((int)(3 * cell), (int)(12 * cell), baseInnerR, baseLightColors[3]);
        static const int SAFE_POS[] = { 0, 8, 12, 13, 21, 25, 26, 34, 38, 39, 47, 51 };
        for (int si : SAFE_POS) {
            int col = LUDO_PATH[si][0];
            int row = LUDO_PATH[si][1];
            drawStar(col * cell + cell / 2.0f, row * cell + cell / 2.0f, cell * 0.32f, { 255,215,0,220 });
        }

        Vector2 ctr = { 7.5f * cell, 7.5f * cell };
        DrawTriangle({ 6 * cell,6 * cell }, { 6 * cell,9 * cell }, ctr, { 220,50,50,255 });  
        DrawTriangle({ 6 * cell,6 * cell }, { 9 * cell,6 * cell }, ctr, { 50,180,80,255 });  
        DrawTriangle({ 9 * cell,6 * cell }, { 9 * cell,9 * cell }, ctr, { 240,200,30,255 }); 
        DrawTriangle({ 6 * cell,9 * cell }, { 9 * cell,9 * cell }, ctr, { 50,120,220,255 }); 
    }
    const auto& movable = g.getMovableTokenIds();
    int curPI = g.getCurrentPlayerIdx();
    const auto& players = g.getPlayers();

    if (g.hasRolledThisTurn() and !movable.empty()) {
        if (curPI < (int)players.size()) {
            for (int tid : movable) {
                Token& t = const_cast<Token&>(players[curPI]->getToken(tid));
                Vector2 dest = tokenScreenPos(curPI, tid, t.getPosition(), t.getState(), t.getStepsInHomeCol());
                DrawCircle((int)dest.x, (int)dest.y, (int)(cell * 0.48f), { 255,255,100,110 });
            }
        }
    }
    float tokenR = cell * 0.40f;
    for (int pi = 0; pi < (int)players.size(); pi++) {
        int countOnSquare[52] = {};
        for (int ti = 0; ti < 4; ti++) {
            const Token& tok = players[pi]->getToken(ti);
            int  lpos = tok.getPosition();
            auto state = tok.getState();
            int  hsteps = tok.getStepsInHomeCol();

            Vector2 base = tokenScreenPos(pi, ti, lpos, state, hsteps);
            float ox = 0, oy = 0;
            if (state == TokenState::ACTIVE and lpos >= 0 and lpos < 52) {
                int cnt = countOnSquare[lpos]++;
                ox = (cnt % 2) * (cell * 0.22f) - (cell * 0.11f);
                oy = (cnt / 2) * (cell * 0.22f) - (cell * 0.11f);
            }
            Vector2 pos = { base.x + ox, base.y + oy };
            bool hl = false;
            if (pi == curPI)
                for (int mid : movable) if (mid == ti) { hl = true; break; }
            drawToken(pos, tokenR, pi, ti, hl and g.hasRolledThisTurn());
        }
    }
    int sbX = (int)boardSize;
    int sbW = screenW - sbX;
    DrawRectangleRounded({ (float)sbX, 0, (float)sbW, (float)screenH }, 0, 0, { 24,20,42,255 });
    DrawRectangle(sbX, 0, 4, screenH, { 160,100,255,255 });

    const char* titleStr = "DOUBLE TROUBLE";
    int titleSz = 26;
    if (hasCustomFont) {
        Vector2 tsz = MeasureTextEx(customFont, titleStr, (float)titleSz, 1);
        DrawTextEx(customFont, titleStr, { sbX + (sbW - tsz.x) / 2.0f, 14 }, (float)titleSz, 1, { 220,190,255,255 });
    }
    else {
        int tw = MeasureText(titleStr, titleSz);
        DrawText(titleStr, sbX + (sbW - tw) / 2, 14, titleSz, { 220,190,255,255 });
    }
    DrawLineEx({ (float)(sbX + 16), 48 }, { (float)(sbX + sbW - 16), 48 }, 1.5f, { 90,70,140,255 });
    const char* modeBadge = (g.getGameMode() == GameMode::LOCKED) ? "LOCKED MODE" : "CLASSIC MODE";
    Color modeBadgeCol = (g.getGameMode() == GameMode::LOCKED) ? Color{ 200,80,50,255 } : Color{ 50,180,80,255 };
    int mbw = MeasureText(modeBadge, 11);
    DrawRectangleRounded({ (float)(sbX + sbW / 2 - mbw / 2 - 6), 50, (float)(mbw + 12), 16 }, 0.4f, 6, modeBadgeCol);
    DrawText(modeBadge, sbX + sbW / 2 - mbw / 2, 53, 11, WHITE);

    int pY = 70;

    if (g.getStatus() == GameStatus::PLAYING and curPI < (int)players.size()) {
        Player* cp = players[curPI];
        Color pc = getPlayerColor(curPI);

        DrawText("CURRENT TURN", sbX + 16, pY, 12, { 160,145,210,255 }); pY += 18;
        DrawRectangleRounded({ (float)(sbX + 8),(float)pY,(float)(sbW - 16),38 }, 0.3f, 8, pc);
        drawTextCentered(cp->getName().c_str(), sbX + 9, pY + 9, sbW - 16, 20, { 0,0,0,80 });
        drawTextCentered(cp->getName().c_str(), sbX + 8, pY + 8, sbW - 16, 20, WHITE);
        pY += 48;
        DrawText("TIME LEFT", sbX + 16, pY, 12, { 160,145,210,255 }); pY += 16;
        drawTimerBar(sbX, sbW, pY, turnTimer, maxTimer);
        pY += 26;
        DrawText("(Auto-plays at 0)", sbX + 16, pY, 11, { 140,130,180,200 }); pY += 18;

        DrawLineEx({ (float)(sbX + 16), (float)pY }, { (float)(sbX + sbW - 16), (float)pY }, 1, { 70,60,110,255 }); pY += 8;
        DrawText("DICE", sbX + 16, pY, 12, { 160,145,210,255 }); pY += 16;

        int diceVal = diceAnimating ? diceAnimFrame : g.getDiceLastRoll();
        float diceSize = sbW * 0.45f;
        Rectangle diceR = { (float)(sbX + sbW / 2 - diceSize / 2), (float)pY, diceSize, diceSize };
        drawDiceFace(diceR, diceVal, g.hasRolledThisTurn());
        pY += (int)diceSize + 10;

        if (!g.hasRolledThisTurn()) {
            Rectangle rollBtn = { (float)(sbX + 16),(float)pY,(float)(sbW - 32),40 };
            drawButton(rollBtn, "ROLL DICE", { 100,60,200,255 }, WHITE, 18);
            pY += 50;
        }
        else {
            const char* msg = "Click a token!";
            int mw = MeasureText(msg, 14);
            DrawText(msg, sbX + (sbW - mw) / 2, pY, 14, { 255,220,80,255 }); pY += 24;

            const string& inv = g.getInvalidMoveMsg();
            if (!inv.empty()) {
                int fs = 11;
                int maxW = sbW - 20;
                string line1, line2;
                string word;
                string cur;
                for (char c : inv) {
                    if (c == ' ') {
                        if (MeasureText((cur + " " + word).c_str(), fs) < maxW) {
                            cur = cur.empty() ? word : cur + " " + word;
                        }
                        else {
                            line1 = cur;
                            cur = word;
                        }
                        word.clear();
                    }
                    else {
                        word += c;
                    }
                }
                if (!word.empty()) {
                    if (MeasureText((cur + " " + word).c_str(), fs) < maxW)
                        cur = cur.empty() ? word : cur + " " + word;
                    else { line1 = cur; cur = word; }
                }
                if (line1.empty()) { line1 = cur; cur = ""; }
                else { line2 = cur; }
                DrawRectangleRounded({ (float)(sbX + 8),(float)pY,(float)(sbW - 16),line2.empty() ? 18.0f : 34.0f }, 0.3f, 6, { 80,20,20,200 });
                DrawText(line1.c_str(), sbX + 12, pY + 2, fs, { 255,150,100,255 });
                if (!line2.empty()) DrawText(line2.c_str(), sbX + 12, pY + 16, fs, { 255,150,100,255 });
                pY += line2.empty() ? 24 : 40;
            }
        }
    }
    DrawLineEx({ (float)(sbX + 16),(float)pY }, { (float)(sbX + sbW - 16),(float)pY }, 1, { 70,60,110,255 }); pY += 8;
    DrawText("PLAYERS", sbX + 16, pY, 12, { 160,145,210,255 }); pY += 18;

    for (int pi = 0; pi < (int)players.size(); pi++) {
        Player* p = players[pi];
        Color pc = getPlayerColor(pi);
        bool isCur = (pi == curPI and g.getStatus() == GameStatus::PLAYING);

        Color rowBg = isCur ? Color{ 55,48,88,255 } : Color{ 36,32,58,255 };
        DrawRectangleRounded({ (float)(sbX + 6),(float)pY,(float)(sbW - 12),38 }, 0.25f, 6, rowBg);
        if (isCur)
            DrawRectangleRoundedLines({ (float)(sbX + 6),(float)pY,(float)(sbW - 12),38 }, 0.25f, 6, pc);

        DrawCircle(sbX + 22, pY + 19, 7, pc);
        DrawText(p->getName().c_str(), sbX + 36, pY + 6, 15, WHITE);
        int dotX = sbX + sbW - 16;
        for (int ti = 3; ti >= 0; ti--) {
            const Token& tok = p->getToken(ti);
            Color dc = tok.isFinished() ? pc :
                (tok.getState() == TokenState::ACTIVE ? WHITE : Color{ 70,70,80,255 });
            DrawCircle(dotX, pY + 19, 5, dc);
            dotX -= 14;
        }
        char fcBuf[8]; sprintf_s(fcBuf, "%d/4", p->getFinishedCount());
        DrawText(fcBuf, sbX + 36, pY + 22, 12, { 155,148,185,255 });
        pY += 44;
    }
    int btnY = screenH - 240;
    int btnX = sbX + 10;
    int btnW = sbW - 20;
    int btnH = 36;
    int gap = 44;

    DrawLineEx({ (float)(sbX + 16),(float)(btnY - 6) }, { (float)(sbX + sbW - 16),(float)(btnY - 6) }, 1, { 70,60,110,255 });

    Rectangle btnSave = { (float)btnX,            (float)btnY,        (float)(btnW / 2 - 4), (float)btnH };
    Rectangle btnLoad = { (float)(btnX + btnW / 2 + 4), (float)btnY,        (float)(btnW / 2 - 4), (float)btnH };
    drawButton(btnSave, "SAVE [S]", { 45,140,80,255 }, WHITE, 13);
    drawButton(btnLoad, "LOAD [L]", { 40,90,160,255 }, WHITE, 13);
    Rectangle btnUndo = { (float)btnX,            (float)(btnY + gap),  (float)(btnW / 2 - 4), (float)btnH };
    Rectangle btnRedo = { (float)(btnX + btnW / 2 + 4), (float)(btnY + gap),  (float)(btnW / 2 - 4), (float)btnH };
    drawButton(btnUndo, "UNDO [U]", { 120,80,50,255 }, WHITE, 13);
    drawButton(btnRedo, "REDO [R]", { 80,60,140,255 }, WHITE, 13);
    Rectangle btnQuit = { (float)btnX, (float)(btnY + gap * 2), (float)btnW, (float)btnH };
    drawButton(btnQuit, "QUIT [ESC]", { 160,40,60,255 }, WHITE, 14);
    DrawText("[P] Replay Mode", sbX + 16, btnY + gap * 3 + 2, 12, { 120,110,170,200 });
}

void GameRenderer::drawWinScreen(const Player& p) const {
    DrawRectangle(0, 0, screenW, screenH, { 0,0,0,170 });

    int pw = 560, ph = 300;
    int px = screenW / 2 - pw / 2;
    int py = screenH / 2 - ph / 2;
    Rectangle panel = { (float)px,(float)py,(float)pw,(float)ph };
    DrawRectangleRounded({ (float)(px - 4),(float)(py - 4),(float)(pw + 8),(float)(ph + 8) }, 0.15f, 10, { 200,180,255,80 });
    DrawRectangleRounded(panel, 0.15f, 10, { 28,22,50,245 });
    DrawRectangleRoundedLines(panel, 0.15f, 10, { 220,200,255,255 });
    DrawText("WINNER!", screenW / 2 - MeasureText("WINNER!", 64) / 2, py + 24, 64, { 255,215,0,255 });

    int pidx = 0;
    const char* colNames[] = { "RED","GREEN","YELLOW","BLUE" };
    for (int i = 0; i < 4; i++)
        if ((int)p.getColour() == i) { pidx = i; break; }

    Color pc = getPlayerColor(pidx);
    DrawText(p.getName().c_str(), screenW / 2 - MeasureText(p.getName().c_str(), 42) / 2, py + 108, 42, pc);

    char sub[64]; sprintf_s(sub, "(%s) has won the game!", colNames[pidx]);
    DrawText(sub, screenW / 2 - MeasureText(sub, 20) / 2, py + 165, 20, { 200,190,235,255 });

    DrawText("[P] Replay   [ESC] Menu", screenW / 2 - MeasureText("[P] Replay   [ESC] Menu", 16) / 2, py + 224, 16, { 155,148,200,255 });
}

void GameRenderer::drawReplayScreen(const Game& g) const {
    drawGame(g, 0, 1);

    int bY = screenH - 56;
    DrawRectangle(0, bY, (int)boardSize, 56, { 0,0,0,190 });
    DrawText("REPLAY MODE", 12, bY + 8, 18, { 220,200,255,255 });
    DrawText("[RIGHT] Forward   [LEFT] Back   [ESC] Exit", 12, bY + 30, 14, { 170,160,210,255 });

    const auto& log = g.getHistory().getFullLog();
    if (!log.empty()) {
        int tw = (int)boardSize - 220;
        DrawRectangle(200, bY + 14, tw, 8, { 55,50,90,255 });
    }
    (void)log;
}