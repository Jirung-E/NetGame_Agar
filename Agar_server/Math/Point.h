#pragma once

#include "Vector.h"


class Point {
public:
    double x;
    double y;

public:
    Point operator+(const Vector& vector) const;
    Point& operator+=(const Vector& vector);
    Vector operator-(const Point& other) const;
};