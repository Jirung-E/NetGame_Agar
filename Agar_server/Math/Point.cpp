#include "Point.h"


Point Point::operator+(const Vector& vector) const {
    return Point { x + vector.x, y + vector.y };
}

Point& Point::operator+=(const Vector& vector) {
    *this = *this + vector;
    return *this;
}

Vector Point::operator-(const Point& other) const {
    return Vector { x - other.x, y - other.y };
}