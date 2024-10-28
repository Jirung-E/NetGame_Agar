#pragma once

#include "Virus.h"


class EnemyCell : public Virus {
private:
    int stroll_count;
    Vector direction;

public:
    bool running;
    bool chasing;

public:
    EnemyCell(const Point& position);

public:
    void randomStroll();
};