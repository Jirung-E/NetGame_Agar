#pragma once

#include "../Math/Point.h"


class Object {
public:
    Point position;
    Vector velocity;
    Vector accelerate;

public:
    Object(const Point& position);
};