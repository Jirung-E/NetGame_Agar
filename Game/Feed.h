#pragma once

#include "Cell.h"


class Feed : public Cell {
private:
    int move_count;

public:
    Feed(const Point& position, const double radius = 0.1);
    void move(const Map& map);
};