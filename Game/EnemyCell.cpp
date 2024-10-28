#include "EnemyCell.h"


EnemyCell::EnemyCell(const Point& position) : Virus { position }, stroll_count { 5 }, direction { 0, 0 },
    running { false }, chasing { false } {
    color = getRandomColor();
    cells.back()->color = color;
}


void EnemyCell::randomStroll() {
    if(stroll_count++ < 5) {
        return;
    }
    direction = Vector { getRandomNumberOf(Range { -1, 1 }, 0.1), getRandomNumberOf(Range { -1, 1 }, 0.1) };
    if(direction.scalar() > 1) {
        direction = direction.unit();
    }
    else if(direction.scalar() <= 0.1) {
        direction = { 0, 0 };
    }
    direction = direction/10;
    stroll_count = 0;
}