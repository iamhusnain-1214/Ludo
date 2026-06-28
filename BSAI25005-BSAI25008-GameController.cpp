#pragma once
#include <vector>
#include <string>
#include "BSAI25005-BSAI25008-Player.h"
#include "BSAI25005-BSAI25008-Board.h"
#include "BSAI25005-BSAI25008-Dice.h"
#include "BSAI25005-BSAI25008-GameHistory.h"
#include "BSAI25005-BSAI25008-RuleEngine.h"
#include "BSAI25005-BSAI25008-GameStateManager.h"

class Game {
private:
    vector<Player*> players;
    Board                board;
    Dice                 dice;
    GameHistory          history;
    RuleEngine           ruleEngine;
    GameStateManager     stateManager;

    int        currentPlayerIdx;
    int        consecutiveSixes;
    GameStatus status;
    bool       rolledThisTurn;
    bool       captured;
    GameMode   gameMode;
    BoardTheme boardTheme;
    bool       tokenHasKilled[4][4];
    string invalidMoveMsg;

    vector<int> movableTokenIds;
    Player* winner;

    void applyMove(const MoveRecord& m);   
    void unapplyMove(const MoveRecord& m); 

public:
    bool didKill;
    Game();
    ~Game();
    void startGame(int numPlayers);
    void rollDice();
    bool moveToken(int tokenId);
    Player* checkWin();
    void advanceTurn();
    void undo();
    void redo();
    void saveGame(const string& path);
    void loadGame(const string& path);
    void startReplay();
    void stepReplayForward();
    void stepReplayBackward();
    const vector<Player*>& getPlayers() const { return players; }
    vector<Player*>&       getPlayersRef()              { return players; }
    Board&  getBoardRef()                { return board; }
    const Board&                getBoard()             const { return board; }
    int getDiceLastRoll()      const { return dice.getLast(); }
    GameStatus getStatus()            const { return status; }
    int  getCurrentPlayerIdx()  const { return currentPlayerIdx; }
    int  getConsecutiveSixes()  const { return consecutiveSixes; }
    bool hasRolledThisTurn()    const { return rolledThisTurn; }
    const GameHistory&          getHistory()           const { return history; }
    const vector<int>&     getMovableTokenIds()   const { return movableTokenIds; }
    Player*                     getWinner()            const { return winner; }
    GameMode                    getGameMode()          const { return gameMode; }
    BoardTheme                  getBoardTheme()        const { return boardTheme; }
    const string&          getInvalidMoveMsg()    const { return invalidMoveMsg; }
    bool                        tokenHasKill(int pi, int ti) const { return tokenHasKilled[pi][ti]; }

    void setGameMode(GameMode m)   { gameMode = m; }
    void setBoardTheme(BoardTheme t) { boardTheme = t; }
    void setCurrentPlayerIdx(int i)     { currentPlayerIdx = i; }
    void setConsecutiveSixes(int n)     { consecutiveSixes = n; }
    void setDiceLastRoll(int v)         { dice.setLast(v); }
    void setRolledThisTurn(bool v)    { rolledThisTurn = v; }
    void setStatus(GameStatus s)        { status = s; }
    void clearHistory()                 { history.clear(); }
    void pushHistory(MoveRecord m)      { history.push(m); }
};
