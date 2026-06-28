#include "BSAI25005-BSAI25008-Token.h"

Token::Token(int id, Colour c)
    : id(id), colour(c), position(-1), state(TokenState::BASE), stepsInHomeCol(0) {}

void Token::move(int steps) {
    if (state == TokenState::ACTIVE) {
        position = (position + steps) % 52;
    } else if (state == TokenState::HOME_COLUMN) {
        stepsInHomeCol += steps;
        if (stepsInHomeCol >= 6) {
            stepsInHomeCol = 6;
            state = TokenState::FINISHED;
            position = 999;
        }
    }
}
void Token::sendToBase() {
    position = -1;
    stepsInHomeCol = 0;
    state = TokenState::BASE;
}

void Token::release() {
    state = TokenState::ACTIVE;
    stepsInHomeCol = 0;
}

bool Token::isInBase()   const { return state == TokenState::BASE; }
bool Token::isFinished() const { return state == TokenState::FINISHED; }

int        Token::getPosition()       const { return position; }
TokenState Token::getState() const
{
    return state;
}
int        Token::getStepsInHomeCol() const { return stepsInHomeCol; }
int        Token::getId()  const { return id; }
Colour     Token::getColour()const { return colour; }
void Token::setPosition(int p)        { position = p; }
void Token::setState(TokenState s)    { state = s; }
void Token::setStepsInHomeCol(int s)  { stepsInHomeCol = s; }
