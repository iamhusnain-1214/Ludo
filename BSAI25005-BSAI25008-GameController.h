#include "BSAI25005-BSAI25008-GameController.h"
#include <cstring>
#include <algorithm>
#include <cstdlib>  
void GameController::run() {
    renderer.init(SCREEN_WIDTH, SCREEN_HEIGHT);
    runWelcome();

    while (!WindowShouldClose()) {
        switch (game.getStatus()) {
        case GameStatus::MENU:
            runMenu();
            break;
        case GameStatus::SETUP:
            runMenu();
            break;
        case GameStatus::PLAYING:
        case GameStatus::FINISHED:
            runGame();
            break;
        case GameStatus::REPLAY:
            runReplay();
            break;
        }
    }
    renderer.close();
}

void GameController::runWelcome() {
    float timer = 0.0f;
    const float SHOW_TIME = 3.0f; 

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        timer += dt;

        if (IsKeyPressed(KEY_ENTER) or IsMouseButtonPressed(MOUSE_BUTTON_LEFT) or timer >= SHOW_TIME) {
            game.setStatus(GameStatus::MENU);
            break;
        }

        BeginDrawing();
        renderer.drawWelcome(timer, SHOW_TIME);
        EndDrawing();
    }
}
void GameController::runMenu() {
    SetExitKey(NULL);

    menuPlayers = 2;
    menuPage = 0;
    waitingForNames = false;
    nameInputIdx = 0;
    typingBuffer = "";
    menuMode = GameMode::CLASSIC;
    menuTheme = BoardTheme::DEFAULT;

    for (int i = 0; i < 4; i++) playerNames[i] = "";

    while (!WindowShouldClose()) {
        int cx = SCREEN_WIDTH / 2;
        int cy = SCREEN_HEIGHT / 2;

        if (menuPage == 0 and !waitingForNames) {

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mp = GetMousePosition();
                for (int n = 2; n <= 4; n++) {
                    Rectangle btn = {
                        (float)(cx - 90 + (n - 3) * 200),
                        (float)(cy - 80),
                        160, 70
                    };
                    if (CheckCollisionPointRec(mp, btn)) {
                        menuPlayers = n;
                    }
                }

                Rectangle newGameBtn = { (float)(cx - 160), (float)(cy + 30), 320, 70 };
                Rectangle loadBtn = { (float)(cx - 160), (float)(cy + 120), 320, 60 };
                Rectangle settingsBtn = { (float)(cx - 160), (float)(cy + 200), 320, 52 };

                if (CheckCollisionPointRec(mp, newGameBtn)) {
                    waitingForNames = true;
                    nameInputIdx = 0;
                    typingBuffer = "";
                }

                if (CheckCollisionPointRec(mp, loadBtn)) {
                    game.loadGame("savegame.ludo");
                    if (game.getStatus() == GameStatus::PLAYING) return;
                }

                if (CheckCollisionPointRec(mp, settingsBtn)) {
                    menuPage = 1;
                }
            }

            if (IsKeyPressed(KEY_L)) {
                game.loadGame("savegame.ludo");
                if (game.getStatus() == GameStatus::PLAYING) return;
            }
        }

        else if (menuPage == 1) {

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mp = GetMousePosition();
                Rectangle classicBtn = { (float)(cx - 180), (float)(cy - 60), 160, 52 };
                Rectangle lockedBtn = { (float)(cx + 20),  (float)(cy - 60), 160, 52 };

                if (CheckCollisionPointRec(mp, classicBtn))
                    menuMode = GameMode::CLASSIC;

                if (CheckCollisionPointRec(mp, lockedBtn))
                    menuMode = GameMode::LOCKED;

                for (int th = 0; th < 4; th++) {
                    Rectangle thBtn = {
                        (float)(cx - 330 + th * 170),
                        (float)(cy + 100),   
                        150, 48
                    };
                    if (CheckCollisionPointRec(mp, thBtn)) {
                        menuTheme = (BoardTheme)th;
                    }
                }
                Rectangle backBtn = {
                    (float)(cx - 160),
                    (float)(cy + 180),   
                    320, 52
                };

                if (CheckCollisionPointRec(mp, backBtn)) {
                    menuPage = 0;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                menuPage = 0;
            }
        }
        else if (waitingForNames) {

            input.updateTyping(typingBuffer);

            if (IsKeyPressed(KEY_ENTER)) {
                if (!typingBuffer.empty()) {
                    playerNames[nameInputIdx] = typingBuffer;
                    typingBuffer = "";
                    nameInputIdx++;

                    if (nameInputIdx >= menuPlayers) {
                        game.startGame(menuPlayers);
                        game.setGameMode(menuMode);
                        game.setBoardTheme(menuTheme);

                        auto& players = game.getPlayersRef();
                        Colour cols[] = {
                            Colour::COL_RED,
                            Colour::COL_GREEN,
                            Colour::COL_YELLOW,
                            Colour::COL_BLUE
                        };

                        for (int i = 0; i < menuPlayers and i < (int)players.size(); i++) {
                            delete players[i];
                            players[i] = new Player(playerNames[i], cols[i]);
                        }

                        return;
                    }
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                waitingForNames = false;
                nameInputIdx = 0;
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);

        if (menuPage == 1)
            renderer.drawMenuSettings(menuMode, menuTheme);
        else
            renderer.drawMenu(menuPlayers, waitingForNames, nameInputIdx,
                playerNames, typingBuffer, menuMode, menuTheme);

        EndDrawing();
    }
}
void GameController::handleTokenClick() {
    Vector2 mp = GetMousePosition();
    const auto& players = game.getPlayers();
    int curPI = game.getCurrentPlayerIdx();
    if (curPI >= (int)players.size()) return;

    float r = renderer.getCellSize() * 0.42f;
    const auto& movable = game.getMovableTokenIds();

    for (int ti : movable) {
        const Token& tok = players[curPI]->getToken(ti);
        Vector2 sp = renderer.tokenScreenPos(curPI, ti, tok.getPosition(), tok.getState(), tok.getStepsInHomeCol());
        if (CheckCollisionPointCircle(mp, sp, r)) {
            game.moveToken(ti);

            if (game.didKill) {
                PlaySound(renderer.killSound);
            }
            turnTimer = TURN_TIMER_MAX;
            return;
        }
    }
}

void GameController::doAutoMove() {
    if (!game.hasRolledThisTurn()) {
        renderer.startDiceAnim();
        game.rollDice();
        turnTimer = TURN_TIMER_MAX;
        return;
    }
    const auto& movable = game.getMovableTokenIds();
    if (!movable.empty()) {
        game.moveToken(movable[0]);
    }
    turnTimer = TURN_TIMER_MAX;
}

void GameController::runGame() {
    int sbX = (int)(renderer.getCellSize() * 15);
    int sbW = SCREEN_WIDTH - sbX;
    diceRect = { (float)(sbX + sbW / 2 - 55), 100 + 18 + 8, 110, 110 };

    turnTimer = TURN_TIMER_MAX;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        renderer.tickDiceAnim(dt);
        if (game.getStatus() == GameStatus::PLAYING) {
            turnTimer -= dt;
            if (turnTimer <= 0.0f) {
                doAutoMove();
            }
        }
        bool inReplay = (game.getStatus() == GameStatus::REPLAY);
        UIEvent e = input.pollEvents(game.hasRolledThisTurn(), inReplay, diceRect);

        switch (e) {
        case UIEvent::ROLL:
            if (game.getStatus() == GameStatus::PLAYING) {
                renderer.startDiceAnim();
                game.rollDice();
                PlaySound(renderer.diceSound);
                turnTimer = TURN_TIMER_MAX;
            }
            break;

        case UIEvent::TOKEN_SELECT:
            if (game.getStatus() == GameStatus::PLAYING and game.hasRolledThisTurn())
                handleTokenClick();
            break;

        case UIEvent::UNDO:   game.undo(); turnTimer = TURN_TIMER_MAX; break;
        case UIEvent::REDO:   game.redo(); turnTimer = TURN_TIMER_MAX; break;
        case UIEvent::SAVE:   game.saveGame("savegame.ludo"); break;
        case UIEvent::LOAD:   game.loadGame("savegame.ludo"); turnTimer = TURN_TIMER_MAX; break;

        case UIEvent::REPLAY_FWD:  game.stepReplayForward(); break;
        case UIEvent::REPLAY_BACK: game.stepReplayBackward(); break;

        case UIEvent::QUIT:
            if (game.getStatus() == GameStatus::FINISHED) {
                game.setStatus(GameStatus::MENU);
                return;
            }
            else if (game.getStatus() == GameStatus::REPLAY) {
                game.setStatus(GameStatus::FINISHED);
            }
            else {
                game.setStatus(GameStatus::MENU);
                return;
            }
            break;

        default: break;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            int sbX2 = (int)(renderer.getCellSize() * 15);
            int sbW2 = SCREEN_WIDTH - sbX2;

            int btnY = SCREEN_HEIGHT - 230;
            int btnX = sbX2 + 10;
            int btnW = sbW2 - 20;
            int btnH = 36;
            int gap = 44;

            Rectangle btnSave = { (float)btnX, (float)btnY,         (float)(btnW / 2 - 4), (float)btnH };
            Rectangle btnLoad = { (float)(btnX + btnW / 2 + 4), (float)btnY, (float)(btnW / 2 - 4), (float)btnH };
            Rectangle btnUndo = { (float)btnX, (float)(btnY + gap),  (float)(btnW / 2 - 4), (float)btnH };
            Rectangle btnRedo = { (float)(btnX + btnW / 2 + 4), (float)(btnY + gap), (float)(btnW / 2 - 4), (float)btnH };
            Rectangle btnQuit = { (float)btnX, (float)(btnY + gap * 2),(float)btnW, (float)btnH };

            if (CheckCollisionPointRec(mp, btnSave)) game.saveGame("savegame.ludo");
            if (CheckCollisionPointRec(mp, btnLoad)) { game.loadGame("savegame.ludo"); turnTimer = TURN_TIMER_MAX; }
            if (CheckCollisionPointRec(mp, btnUndo)) { game.undo(); turnTimer = TURN_TIMER_MAX; }
            if (CheckCollisionPointRec(mp, btnRedo)) { game.redo(); turnTimer = TURN_TIMER_MAX; }
            if (CheckCollisionPointRec(mp, btnQuit)) { game.setStatus(GameStatus::MENU); return; }
        }

        if (IsKeyPressed(KEY_P)) {
            if (game.getStatus() == GameStatus::FINISHED or game.getStatus() == GameStatus::PLAYING)
                game.startReplay();
        }
        BeginDrawing();
        ClearBackground(BLACK);

        if (game.getStatus() == GameStatus::REPLAY) {
            renderer.drawReplayScreen(game);
        }
        else {
            renderer.drawGame(game, turnTimer, TURN_TIMER_MAX);
            if (game.getStatus() == GameStatus::FINISHED and game.getWinner()) {
                renderer.drawWinScreen(*game.getWinner());
            }
        }

        EndDrawing();

        if (game.getStatus() == GameStatus::MENU) return;
    }
}

void GameController::runReplay() {
    runGame();
}