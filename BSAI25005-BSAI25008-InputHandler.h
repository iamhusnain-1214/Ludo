#include "BSAI25005-BSAI25008-InputHandler.h"

UIEvent InputHandler::pollEvents(bool rolledThisTurn, bool inReplay, Rectangle diceRect) {
    if (inReplay) {
        if (IsKeyPressed(KEY_RIGHT))  return UIEvent::REPLAY_FWD;
        if (IsKeyPressed(KEY_LEFT))   return UIEvent::REPLAY_BACK;
        if (IsKeyPressed(KEY_ESCAPE)) return UIEvent::QUIT;
        return UIEvent::NONE;
    }
    if (IsKeyPressed(KEY_ESCAPE)) return UIEvent::QUIT;

    if (IsKeyPressed(KEY_U)) return UIEvent::UNDO;
    if (IsKeyPressed(KEY_R)) return UIEvent::REDO;
    if (IsKeyPressed(KEY_S)) return UIEvent::SAVE;
    if (IsKeyPressed(KEY_L)) return UIEvent::LOAD;

    if (!rolledThisTurn) {
        if (IsKeyPressed(KEY_SPACE)) return UIEvent::ROLL;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            if (CheckCollisionPointRec(mp, diceRect)) return UIEvent::ROLL;
      
            return UIEvent::NONE;
        }
    } else {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return UIEvent::TOKEN_SELECT;
    }

    return UIEvent::NONE;
}

void InputHandler::updateTyping(string& buf) {
    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= 32 and ch < 128 and (int)buf.size() < 16) {
            buf += (char)ch;
        }
        ch = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) and !buf.empty()) {
        buf.pop_back();
    }
}

bool InputHandler::clickedCircle(Vector2 center, float radius) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return CheckCollisionPointCircle(GetMousePosition(), center, radius);
    }
    return false;
}

int InputHandler::playerCountDelta() {
    if (IsKeyPressed(KEY_UP))   return 1;
    if (IsKeyPressed(KEY_DOWN)) return -1;
    return 0;
}
