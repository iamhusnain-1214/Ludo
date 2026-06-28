#include "BSAI25005-BSAI25008-GameHistory.h"

void GameHistory::push(MoveRecord r) {
    undoStack.push(r);
    fullLog.push_back(r);
    while (!redoStack.empty()) redoStack.pop();
}

MoveRecord GameHistory::popUndo() {
    if (undoStack.empty()) return MoveRecord();
    MoveRecord r = undoStack.top();
    undoStack.pop();
    redoStack.push(r);
    return r;
}

MoveRecord GameHistory::popRedo() {
    if (redoStack.empty()) return MoveRecord();
    MoveRecord r = redoStack.top();
    redoStack.pop();
    undoStack.push(r);
    return r;
}

bool GameHistory::canUndo() const { return !undoStack.empty(); }
bool GameHistory::canRedo() const { return !redoStack.empty(); }

MoveRecord GameHistory::replayNext() {
    if (replayHasNext()) return fullLog[replayIdx++];
    return MoveRecord();
}

MoveRecord GameHistory::replayPrev() {
    if (replayHasPrev()) return fullLog[--replayIdx];
    return MoveRecord();
}

void GameHistory::clear() {
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
    fullLog.clear();
    replayIdx = 0;
}
