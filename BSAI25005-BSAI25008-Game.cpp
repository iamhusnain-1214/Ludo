#pragma once
#include <iostream>
using namespace std;

enum class TokenState { BASE, ACTIVE, HOME_COLUMN, FINISHED };
enum class SquareType { NORMAL, SAFE, HOME_COLUMN, BASE, CENTRE };
enum class GameStatus { MENU, SETUP, PLAYING, REPLAY, FINISHED };
enum class UIEvent { NONE, ROLL, TOKEN_SELECT, UNDO, REDO, SAVE, LOAD, REPLAY_FWD, REPLAY_BACK, QUIT };
enum class Colour { COL_RED, COL_GREEN, COL_YELLOW, COL_BLUE, COL_NONE };
enum class GameMode { CLASSIC, LOCKED };
enum class BoardTheme { DEFAULT, OCEAN, FOREST, SUNSET };