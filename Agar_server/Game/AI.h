#pragma once

#include "Player.h"


class AI : public Player {
private:
    int stroll_count;
    Vector direction;

public:
    bool running;
    bool chasing;

public:
    AI(const uint8_t id, const Point& position);

public:
    void randomStroll();
};