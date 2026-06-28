#pragma once
#include <stack>
#include <vector>
#include "BSAI25005-BSAI25008-MoveRecord.h"

class GameHistory {
private:
    stack<MoveRecord>  undoStack;
    stack<MoveRecord>  redoStack;
    vector<MoveRecord> fullLog;
    int replayIdx;

public:
    GameHistory() : replayIdx(0) {}

    void push(MoveRecord r);
    MoveRecord popUndo();
    MoveRecord popRedo();
    bool canUndo() const;
    bool canRedo() const;
    void resetReplay() 
    {
        replayIdx = 0;
    }
    bool replayHasNext()const 
    {
        return replayIdx < (int)fullLog.size();
    }
    bool replayHasPrev()const
    {
        return replayIdx > 0;
    }
    MoveRecord replayNext();
    MoveRecord replayPrev();
    const vector<MoveRecord>& getFullLog() const { return fullLog; }
    void clear();
};
