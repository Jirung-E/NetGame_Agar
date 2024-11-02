#pragma once


class Vector {
public:
    double x;
    double y;

public:
    Vector operator-();
    Vector& operator+=(const Vector& other);
    Vector& operator-=(const Vector& other);
    Vector operator*(const double& n) const;
    Vector operator/(const double& n) const;
    Vector& operator/=(const double& n);
    Vector& operator*=(const double& n);
    bool operator==(const Vector& other) const;

    Vector unit() const;
    double scalar() const;
};