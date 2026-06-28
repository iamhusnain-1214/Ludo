#include "BSAI25005-BSAI25008-GameStateManager.h"
#include "BSAI25005-BSAI25008-Game.h"
#include <fstream>
#include <string>
using namespace std;
bool GameStateManager::validateFile(const string& path) {
    ifstream fin(path);
    if (!fin) return false;
    string line;
    getline(fin, line);
    return (line == "---- LUDO SAVE FILE v1 ----");
}

bool GameStateManager::save(const Game& g, const string& path) {
    ofstream fout("save.txt");
    if (!fout) return false;

    const auto& players = g.getPlayers();

    fout << "---- LUDO SAVE FILE v1 ----\n";
    fout << "PLAYERS " << players.size() << "\n";

    for (int i = 0; i < (int)players.size(); i++) {
        Player* p = players[i];
        fout << "PLAYER " << i << " " << p->getName() << " " << (int)p->getColour() << "\n";
        for (int t = 0; t < 4; t++) {
            const Token& tok = p->getToken(t);
            fout << "TOKEN " << t << " "
                << tok.getPosition() << " "
                << (int)tok.getState() << " "
                << tok.getStepsInHomeCol() << "\n";
        }
    }

    fout << "CURRENT_PLAYER " << g.getCurrentPlayerIdx() << "\n";
    fout << "CONSECUTIVE_SIXES " << g.getConsecutiveSixes() << "\n";
    fout << "LAST_ROLL " << g.getDiceLastRoll() << "\n";
    fout << "ROLLED_THIS_TURN " << (g.hasRolledThisTurn() ? 1 : 0) << "\n";

    const auto& log = g.getHistory().getFullLog();
    fout << "HISTORY_COUNT " << log.size() << "\n";
    for (const auto& m : log) {
        fout << "MOVE "
            << m.playerIdx << " "
            << m.tokenId << " "
            << m.fromPos << " "
            << m.toPos << " "
            << (int)m.fromState << " "
            << (int)m.toState << " "
            << m.fromHomeSteps << " "
            << m.toHomeSteps << " "
            << m.rollValue << " "
            << m.capturedPlayer << " "
            << m.capturedToken << " "
            << m.capturedFromPos << "\n";
    }

    fout << "---- END ----\n";
    fout.close();
    return true;
}

bool GameStateManager::load(const string& path, Game& g) {
    ifstream fin("save.txt");
    if (!fin) return false;

    string word, line;
    getline(fin, line);   // skip header

    int numPlayers = 0;
    fin >> word >> numPlayers;
    getline(fin, line);

    g.startGame(numPlayers);
    auto& players = g.getPlayersRef();

    for (int i = 0; i < numPlayers; i++) {
        int idx, colInt;
        string name;
        fin >> word >> idx >> name >> colInt;
        getline(fin, line);

        for (int t = 0; t < 4; t++) {
            int tid, pos, stateInt, homeSteps;
            fin >> word >> tid >> pos >> stateInt >> homeSteps;
            getline(fin, line);
            Token& tok = players[i]->getToken(tid);
            tok.setPosition(pos);
            tok.setState((TokenState)stateInt);
            tok.setStepsInHomeCol(homeSteps);
        }
        players[i]->recalcFinished();
    }

    int curPlayer = 0, consSixes = 0, lastRoll = 1, rolled = 0;
    fin >> word >> curPlayer;    getline(fin, line);
    fin >> word >> consSixes;    getline(fin, line);
    fin >> word >> lastRoll;     getline(fin, line);
    fin >> word >> rolled;       getline(fin, line);

    g.setCurrentPlayerIdx(curPlayer);
    g.setConsecutiveSixes(consSixes);
    g.setDiceLastRoll(lastRoll);
    g.setRolledThisTurn(rolled == 1);

    int histCount = 0;
    fin >> word >> histCount;    getline(fin, line);

    g.clearHistory();
    for (int i = 0; i < histCount; i++) {
        MoveRecord m;
        int fs, ts;
        fin >> word
            >> m.playerIdx
            >> m.tokenId
            >> m.fromPos
            >> m.toPos
            >> fs
            >> ts
            >> m.fromHomeSteps
            >> m.toHomeSteps
            >> m.rollValue
            >> m.capturedPlayer
            >> m.capturedToken
            >> m.capturedFromPos;
        getline(fin, line);
        m.fromState = (TokenState)fs;
        m.toState = (TokenState)ts;
        g.pushHistory(m);
    }

    fin.close();
    return true;
}