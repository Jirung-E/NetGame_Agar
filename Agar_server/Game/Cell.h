#pragma once

#include "Object.h"
#include "Map.h"

#include <list>


class Cell : public Object {
public:
    COLORREF color;
    static const double max_radius;
    static const double min_radius;

protected:
    double radius;
    double target_radius;
    double prev_radius;
    int trans_count;
    int accel_count;
    bool invincible;

public:
    Cell(const Point& position, const double radius = 0.3);

public:
    void setUp();

    void move(const Vector& vector, const Map& map);
    void move(const Map& map);

    bool collideWith(const Cell* other);

    void merge(Cell* cell);
    void eat(Cell* cell);
    void growUp();

    double getRadius() const;

    Cell* spit();
    Cell* split();
    std::list<Cell*> explode();

    bool isInvincible() const;
};