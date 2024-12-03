#include "Player.h"


void Player::addCell(Cell* cell) {
    cells.push_back(cell);
}

void Player::clearCells() {
    // 오브젝트 리스트의 원소에 대한 포인터 이므로 delete불필요
    cells.clear();
}

const std::list<Cell*>& Player::getCells() const {
    return cells;
}


void Player::draw(const HDC& hdc, const Map& map, const RECT& valid_area) const {
    for(auto e : cells) {
        e->draw(hdc, map, valid_area);
    }
}


Point Player::getCenterPoint() const {
    Point center { 0, 0 };
    for(auto e : cells) {
        center.x += e->position.x;
        center.y += e->position.y;
    }
    int size = cells.size();
    center.x /= size;
    center.y /= size;
    return center;
}

double Player::getRadius() const {
    Point p = getCenterPoint();
    double max = 0;
    for(auto e : cells) {
        if(e->getRadius() > max) {
            max = e->getRadius();
        }
    }
    for(auto e : cells) {
        if((e->position - p).scalar() + e->getRadius() > max) {
            max = (e->position - p).scalar() + e->getRadius();
        }
    }
    return max;
}

double Player::getSize() const {
    double result = 0;
    for(auto e : cells) {
        result += pow(e->getRadius(), 2);
    }
    return result;
}
