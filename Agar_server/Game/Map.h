#pragma once

#include "../Util.h"


class Map {
private:
    int width;
    int height;
    COLORREF background_color;
    COLORREF border_color;
    int border_width;

public:
    Map();

public:
    void draw(const HDC& hdc, const RECT& valid_area) const;
    RECT absoluteArea(const RECT& valid_area) const;

    int getWidth() const;
    int getHeight() const;
};