#include "BSAI25005-BSAI25008-Game.h"
#include <algorithm>

Game::Game()
    : currentPlayerIdx(0), consecutiveSixes(0),
    status(GameStatus::MENU), rolledThisTurn(false),
    captured(false), winner(nullptr),
    gameMode(GameMode::CLASSIC), boardTheme(BoardTheme::DEFAULT)
{
    for (int p = 0; p < 4; p++)
        for (int t = 0; t < 4; t++)
            tokenHasKilled[p][t] = false;
}

Game::~Game() {
    for (auto p : players) delete p;
}

void Game::startGame(int numPlayers) {
    for (auto p : players) delete p;
    players.clear();

    Colour cols[] = { Colour::COL_RED, Colour::COL_GREEN, Colour::COL_YELLOW, Colour::COL_BLUE };
    string names[] = { "Player 1", "Player 2", "Player 3", "Player 4" };

    for (int i = 0; i < numPlayers; i++)
        players.push_back(new Player(names[i], cols[i]));

    board.reset();
    history.clear();
    currentPlayerIdx = 0;
    consecutiveSixes = 0;
    rolledThisTurn = false;
    captured = false;
    winner = nullptr;
    movableTokenIds.clear();
    invalidMoveMsg.clear();
    status = GameStatus::PLAYING;

    for (int p = 0; p < 4; p++)
        for (int t = 0; t < 4; t++)
            tokenHasKilled[p][t] = false;
}

void Game::rollDice() {
    if (status != GameStatus::PLAYING) return;
    if (rolledThisTurn) return;

    int roll = dice.roll();
    rolledThisTurn = true;
    captured = false;
    invalidMoveMsg.clear();

    if (roll == 6) {
        consecutiveSixes++;
        if (ruleEngine.isForfeit(consecutiveSixes)) {
            if (history.canUndo()) {
                MoveRecord r1 = history.popUndo();
                Player* p1 = players[r1.playerIdx];
                Token& t1 = p1->getToken(r1.tokenId);
                t1.setPosition(r1.fromPos);
                t1.setState(r1.fromState);
                t1.setStepsInHomeCol(r1.fromHomeSteps);
                if (r1.capturedToken != -1) {
                    Token& cap = players[r1.capturedPlayer]->getToken(r1.capturedToken);
                    cap.setPosition(r1.capturedFromPos);
                    cap.setState(TokenState::ACTIVE);
                    cap.setStepsInHomeCol(0);
                    players[r1.capturedPlayer]->recalcFinished();
                    tokenHasKilled[r1.playerIdx][r1.tokenId] = false;
                }
                p1->recalcFinished();
            }
            if (history.canUndo()) {
                MoveRecord r2 = history.popUndo();
                Player* p2 = players[r2.playerIdx];
                Token& t2 = p2->getToken(r2.tokenId);
                t2.setPosition(r2.fromPos);
                t2.setState(r2.fromState);
                t2.setStepsInHomeCol(r2.fromHomeSteps);
                if (r2.capturedToken != -1) {
                    Token& cap = players[r2.capturedPlayer]->getToken(r2.capturedToken);
                    cap.setPosition(r2.capturedFromPos);
                    cap.setState(TokenState::ACTIVE);
                    cap.setStepsInHomeCol(0);
                    players[r2.capturedPlayer]->recalcFinished();
                    tokenHasKilled[r2.playerIdx][r2.tokenId] = false;
                }
                p2->recalcFinished();
            }
            consecutiveSixes = 0;
            rolledThisTurn = false;
            advanceTurn();
            return;
        }
    }
    else {
        consecutiveSixes = 0;
    }
    movableTokenIds = players[currentPlayerIdx]->getMovableTokenIds(roll, board);
    if (gameMode == GameMode::LOCKED) {
        vector<int> filtered;
        for (int tid : movableTokenIds) {
            Token& tok = players[currentPlayerIdx]->getToken(tid);
            bool wouldFinish = false;
            if (tok.getState() == TokenState::ACTIVE) {
                int homeEntry = board.getHomeColStart(players[currentPlayerIdx]->getColour());
                int cur = tok.getPosition();
                for (int step = 1; step <= roll; step++) {
                    if (cur == homeEntry) { wouldFinish = true; break; }
                    cur = (cur + 1) % 52;
                }
                if (cur == homeEntry) wouldFinish = true;
            }
            else if (tok.getState() == TokenState::HOME_COLUMN) {
                int newSteps = tok.getStepsInHomeCol() + roll;
                if (newSteps >= 6) wouldFinish = true;
            }
            if (wouldFinish and !tokenHasKilled[currentPlayerIdx][tid]) {
                continue;
            }
            filtered.push_back(tid);
        }
        if (!filtered.empty() or filtered.size() == movableTokenIds.size()) {
            invalidMoveMsg.clear();
        }
        else {
            invalidMoveMsg = "Some tokens blocked: must kill an opponent to go home!";
        }
        if (filtered.empty() and !movableTokenIds.empty()) {
            invalidMoveMsg = "No piece can go home without killing an opponent first!";
        }
        movableTokenIds = filtered;
    }

    if (movableTokenIds.empty()) {
        rolledThisTurn = false;
        advanceTurn();
    }
}

bool Game::moveToken(int tokenId) {
    didKill = false;
    if (status != GameStatus::PLAYING) return false;
    if (!rolledThisTurn) return false;

    if (find(movableTokenIds.begin(), movableTokenIds.end(), tokenId) == movableTokenIds.end()) {
        if (gameMode == GameMode::LOCKED) {
            invalidMoveMsg = "This piece can't go home - must kill an opponent first!";
        }
        return false;
    }

    Player* p = players[currentPlayerIdx];
    Token& t = p->getToken(tokenId);
    int roll = dice.getLast();

    MoveRecord rec;
    rec.playerIdx = currentPlayerIdx;
    rec.tokenId = tokenId;
    rec.fromPos = t.getPosition();
    rec.fromState = t.getState();
    rec.fromHomeSteps = t.getStepsInHomeCol();
    rec.rollValue = roll;
    rec.capturedPlayer = -1;
    rec.capturedToken = -1;
    rec.capturedFromPos = -1;
    if (t.getState() == TokenState::BASE) {
        t.release();
        t.setPosition(board.getStartPos(p->getColour()));
    }
    else {
        int homeEntry = board.getHomeColStart(p->getColour());
        if (t.getState() == TokenState::HOME_COLUMN) {
            int newSteps = t.getStepsInHomeCol() + roll;

            if (newSteps >= 6) {
                t.setStepsInHomeCol(6);
                t.setState(TokenState::FINISHED);
                t.setPosition(999);
            }
            else {
                t.setStepsInHomeCol(newSteps);
            }
        }
        else {
            int cur = t.getPosition();
            bool enteringHome = false;
            int stepsUsed = 0;

            for (int step = 1; step <= roll; step++) {
                cur = (cur + 1) % 52;

                if (cur == homeEntry) {
                    enteringHome = true;
                    stepsUsed = step;
                    break;
                }
            }

            if (enteringHome) {
                int remainingSteps = roll - stepsUsed + 1;

                t.setState(TokenState::HOME_COLUMN);
                t.setStepsInHomeCol(remainingSteps);
                t.setPosition(0);

                if (remainingSteps >= 6) {
                    t.setState(TokenState::FINISHED);
                    t.setPosition(999);
                    t.setStepsInHomeCol(6);
                }
            }
            else {
                t.setPosition(cur);
            }
        }
    }
    if (t.getState() == TokenState::ACTIVE and !board.isSafe(t.getPosition())) {
        for (int pi = 0; pi < (int)players.size(); pi++) {
            if (pi == currentPlayerIdx) continue;

            for (int ti = 0; ti < 4; ti++) {
                Token& other = players[pi]->getToken(ti);

                if (other.getState() == TokenState::ACTIVE and
                    other.getPosition() == t.getPosition()) {

                    rec.capturedPlayer = pi;
                    rec.capturedToken = ti;
                    rec.capturedFromPos = other.getPosition();

                    other.sendToBase();
                    captured = true;

                    tokenHasKilled[currentPlayerIdx][tokenId] = true;
                    players[pi]->recalcFinished();
                    didKill = true;
                    break;
                }
            }
            if (rec.capturedToken != -1) break;
        }
    }

    rec.toPos = t.getPosition();
    rec.toState = t.getState();
    rec.toHomeSteps = t.getStepsInHomeCol();

    history.push(rec);
    p->recalcFinished();
    invalidMoveMsg.clear();
    if (p->hasWon()) {
        winner = p;
        status = GameStatus::FINISHED;
        movableTokenIds.clear();
        rolledThisTurn = false;
        return true;
    }
    bool extra = ruleEngine.grantExtraTurn(roll, captured);

    rolledThisTurn = false;
    movableTokenIds.clear();

    if (!extra) {
        advanceTurn();
    }

    return true;
}

Player* Game::checkWin() {
    return winner;
}

void Game::advanceTurn() {
    consecutiveSixes = 0;
    rolledThisTurn = false;
    movableTokenIds.clear();
    invalidMoveMsg.clear();
    currentPlayerIdx = (currentPlayerIdx + 1) % (int)players.size();
}

void Game::undo() {
    if (!history.canUndo()) return;
    MoveRecord r = history.popUndo();

    Player* p = players[r.playerIdx];
    Token& t = p->getToken(r.tokenId);
    t.setPosition(r.fromPos);
    t.setState(r.fromState);
    t.setStepsInHomeCol(r.fromHomeSteps);
    p->recalcFinished();

    if (r.capturedToken != -1) {
        Token& cap = players[r.capturedPlayer]->getToken(r.capturedToken);
        cap.setPosition(r.capturedFromPos);
        cap.setState(TokenState::ACTIVE);
        cap.setStepsInHomeCol(0);
        players[r.capturedPlayer]->recalcFinished();
        tokenHasKilled[r.playerIdx][r.tokenId] = false;
    }

    currentPlayerIdx = r.playerIdx;
    rolledThisTurn = false;
    movableTokenIds.clear();
    invalidMoveMsg.clear();
    winner = nullptr;
    if (status == GameStatus::FINISHED) status = GameStatus::PLAYING;
}

void Game::redo() {
    if (!history.canRedo()) return;
    MoveRecord r = history.popRedo();

    Player* p = players[r.playerIdx];
    Token& t = p->getToken(r.tokenId);
    t.setPosition(r.toPos);
    t.setState(r.toState);
    t.setStepsInHomeCol(r.toHomeSteps);
    p->recalcFinished();

    if (r.capturedToken != -1) {
        players[r.capturedPlayer]->getToken(r.capturedToken).sendToBase();
        players[r.capturedPlayer]->recalcFinished();
        tokenHasKilled[r.playerIdx][r.tokenId] = true;
    }

    dice.setLast(r.rollValue);
    rolledThisTurn = false;
    movableTokenIds.clear();
    invalidMoveMsg.clear();

    if (p->hasWon()) {
        winner = p;
        status = GameStatus::FINISHED;
        currentPlayerIdx = r.playerIdx;
    }
    else {
        bool extra = ruleEngine.grantExtraTurn(r.rollValue, r.capturedToken != -1);
        if (extra) {
            currentPlayerIdx = r.playerIdx;
        }
        else {
            currentPlayerIdx = (r.playerIdx + 1) % (int)players.size();
        }
    }
}

void Game::saveGame(const string& path) {
    stateManager.save(*this, path);
}

void Game::loadGame(const 
    string& path) {
    if (stateManager.load(path, *this))
        status = GameStatus::PLAYING;
}

void Game::startReplay() {
    status = GameStatus::REPLAY;
    history.resetReplay();
    for (auto* p : players) {
        for (int t = 0; t < 4; t++) {
            p->getToken(t).sendToBase();
        }
        p->recalcFinished();
    }
}

void Game::stepReplayForward() {
    if (!history.replayHasNext()) return;
    MoveRecord r = history.replayNext();
    Player* p = players[r.playerIdx];
    Token& t = p->getToken(r.tokenId);
    t.setPosition(r.toPos);
    t.setState(r.toState);
    t.setStepsInHomeCol(r.toHomeSteps);
    p->recalcFinished();
    if (r.capturedToken != -1) {
        players[r.capturedPlayer]->getToken(r.capturedToken).sendToBase();
        players[r.capturedPlayer]->recalcFinished();
    }
}

void Game::stepReplayBackward() {
    if (!history.replayHasPrev()) return;
    MoveRecord r = history.replayPrev();
    Player* p = players[r.playerIdx];
    Token& t = p->getToken(r.tokenId);
    t.setPosition(r.fromPos);
    t.setState(r.fromState);
    t.setStepsInHomeCol(r.fromHomeSteps);
    p->recalcFinished();
    if (r.capturedToken != -1) {
        Token& cap = players[r.capturedPlayer]->getToken(r.capturedToken);
        cap.setPosition(r.capturedFromPos);
        cap.setState(TokenState::ACTIVE);
        cap.setStepsInHomeCol(0);
        players[r.capturedPlayer]->recalcFinished();
    }
}