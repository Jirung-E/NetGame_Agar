#include "Vector.h"

#include <math.h>


Vector Vector::operator-() {
    return { -x, -y };
}

Vector& Vector::operator+=(const Vector& other) {
    this->x += other.x;
    this->y += other.y;
    return *this;
}

Vector& Vector::operator-=(const Vector& other) {
    this->x -= other.x;
    this->y -= other.y;
    return *this;
}

Vector Vector::operator*(const double& n) const {
    return Vector { x*n, y*n };
}

Vector Vector::operator/(const double& n) const {
    return Vector { x/n, y/n };
}

Vector& Vector::operator/=(const double& n) {
    x /= n;
    y /= n;
    return *this;
}

Vector& Vector::operator*=(const double& n) {
    x *= n;
    y *= n;
    return *this;
}

bool Vector::operator==(const Vector& other) const {
    if(x == other.x && y == other.y) {
        return true;
    }
    return false;
}

Vector Vector::unit() const {
    double s = scalar();
    if(s == 0) {
        return { 0, 0 };
    }
    return *this/s;
}

double Vector::scalar() const {
    return sqrt(x*x + y*y);
}