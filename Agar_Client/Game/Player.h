#pragma once

#include "Cell.h"

#include <list>


class Player {
private:
    std::list<Cell*> cells;

public:
    void addCell(Cell* cell);
    void clearCells();

    void draw(const HDC& hdc, const Map& map, const RECT& valid_area) const;

    // 플레이어 셀 집합의 중심
    Point getCenterPoint() const;
    // 플레이어 셀 집합의 반지름
    double getRadius() const;

    // 플레이어 셀 집합의 총 크기
    double getSize() const;
};