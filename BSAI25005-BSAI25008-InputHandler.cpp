#pragma once
#include <string>
using namespace std;
class Game;

class GameStateManager {
public:
    bool save(const Game& g, const string& path);
    bool load(const string& path, Game& g);
    bool validateFile(const string& path);
};
