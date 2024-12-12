#pragma once

#include "Cell.h"

#include <list>


class Player {
public:
    const uint8_t id;

    static const int merge_count_max;
    int merge_count;

    COLORREF color;
    std::list<Cell*> cells;

    Point destination;

private:
    std::string name;

public:
    Player(const uint8_t id, const Point& position);

public:
    void move(const Vector& vector, const Map& map);
    void move(const Map& map);

    void update();

    void collideWith(Player& other);

    void growUp();

    Point getCenterPoint() const;
    double getRadius() const;
    
    double getSize() const;

    void split();

    void setName(const std::string& name);
    std::string getName() const;
};