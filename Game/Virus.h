#pragma once

#include "Cell.h"

#include <list>


class Virus {
public:
    static const int merge_count_max;
    int merge_count;

public:
    COLORREF color;
    std::list<Cell*> cells;

public:
    Virus(const Point& position);

public:
    void setUp();

    void draw(const HDC& hdc, const Map& map, const RECT& valid_area) const;

    void move(const Vector& vector, const Map& map);
    void move(const Map& map);

    void update();

    void collideWith(Virus* other);

    void growUp();

    Point getCenterPoint() const;
    double getRadius() const;
    
    double getSize() const;

    void split();
};