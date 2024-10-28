#include "Cell.h"

#define _USE_MATH_DEFINES
#include <math.h>


const double Cell::min_radius = 0.3;
const double Cell::max_radius = 5;

Cell::Cell(const Point& position, const double radius) : Object { position }, radius { radius }, color { White }, 
target_radius { radius }, prev_radius { radius }, trans_count { 0 }, accel_count { 0 }, invincible { false } {

}


void Cell::setUp() {
    radius = min_radius;
    target_radius = radius;
    prev_radius = radius;
    trans_count = 0;
    accel_count = 0;
    velocity = { 0, 0 };
}


void Cell::draw(const HDC& hdc, const Map& map, const RECT& valid_area) const {
    RECT area = map.absoluteArea(valid_area);
    int w = area.right - area.left;
    int h = area.bottom - area.top;
    int x = area.left + position.x / map.getWidth() * w;
    int y = area.top + position.y / map.getHeight() * h;
    int r = radius / map.getWidth() * w;

    HBRUSH bg_br = CreateSolidBrush(color);
    HPEN bd_pen = CreatePen(PS_SOLID, 2, color & DarkGray);
    HBRUSH old_br = (HBRUSH)SelectObject(hdc, bg_br);
    HPEN old_pen = (HPEN)SelectObject(hdc, bd_pen);

    Ellipse(hdc, x-r, y-r, x+r, y+r);

    SelectObject(hdc, old_br);
    SelectObject(hdc, old_pen);
    DeleteObject(bd_pen);
    DeleteObject(bg_br);
}

POINT Cell::absolutePosition(const Map& map, const RECT& valid_area) const {
    RECT area = map.absoluteArea(valid_area);
    int w = area.right - area.left;
    int h = area.bottom - area.top;
    int x = area.left + position.x / map.getWidth() * w;
    int y = area.top + position.y / map.getHeight() * h;
    return { x, y };
}

void Cell::move(const Vector& vector, const Map& map) {
    velocity = vector;
    if(velocity.scalar() > 1) {
        velocity = velocity.unit();
    }
    else if(velocity.scalar() <= 0.1) {
        velocity = { 0, 0 };
    }
    move(map);
}

void Cell::move(const Map& map) {
    if(accel_count < 20) {
        accel_count++;
        velocity += accelerate;
    }
    position += velocity / 10 / (0.7+radius);

    if(position.x - radius <= 0) {
        position.x = radius;
    }
    else if(position.x + radius >= map.getWidth()) {
        position.x = map.getWidth() - radius;
    }
    if(position.y - radius <= 0) {
        position.y = radius;
    }
    else if(position.y + radius >= map.getHeight()) {
        position.y = map.getHeight() - radius;
    }
}


bool Cell::collideWith(const Cell* other) {
    double dist_between_center = Vector(other->position - position).scalar();
    if(radius >= other->radius) {
        if(dist_between_center <= radius + other->radius/3) {
            return true;
        }
    }
    else {
        if(dist_between_center <= radius/3 + other->radius) {
            return true;
        }
    }
    return false;
}

void Cell::merge(Cell* cell) {
    target_radius = sqrt(pow(target_radius, 2) + pow(cell->radius, 2));
    if(target_radius > max_radius) {
        target_radius = max_radius;
    }
    radius = target_radius;
    prev_radius = radius;
}

void Cell::eat(Cell* cell) {
    target_radius = sqrt(pow(target_radius, 2) + pow(cell->radius, 2));
    if(target_radius > max_radius) {
        target_radius = max_radius;
    }
    prev_radius = radius;
    trans_count = 0;
}

void Cell::growUp() {
    if(trans_count > 30) {
        return;
    }
    radius = -(target_radius-prev_radius)*pow(trans_count++/30.0-1, 6) + target_radius;
}

double Cell::getRadius() const {
    return radius;
}

Cell* Cell::spit() {
    if(radius <= min_radius*1.5) {
        return nullptr;
    }

    target_radius = sqrt(pow(target_radius, 2) - pow(min_radius/1.5, 2));
    prev_radius = radius;
    trans_count = 0;

    Cell* cell = new Cell { position + velocity.unit()*(radius + min_radius/2), min_radius/1.5 };
    cell->velocity = velocity.unit();
    cell->color = color;
    return cell;
}

Cell* Cell::split() {
    if(radius <= min_radius*2) {
        return nullptr;
    }

    target_radius = target_radius/sqrt(2.0);
    radius = target_radius;
    prev_radius = radius;

    Cell* cell = new Cell { position, target_radius };
    cell->accelerate = velocity.unit()*radius;
    cell->color = color;
    return cell;
}

std::list<Cell*> Cell::explode() {
    std::list<Cell*> frag;

    target_radius = target_radius/sqrt(6.0);
    radius = target_radius;
    prev_radius = radius;

    accelerate = { 0, -1*radius*3 };
    position.y -= radius*2;
    accel_count = 0;
    
    int frag_num = 6;

    for(int i=1; i<frag_num; ++i) {
        Vector dir = { cos(-M_PI/2 + i*(2*M_PI)/frag_num), sin(-M_PI/2 + i*(2*M_PI)/frag_num) };
        Cell* cell = new Cell { position + dir.unit()*radius*2, target_radius };
        cell->accelerate = dir*radius*3;
        cell->color = color;
        frag.push_back(cell);
    }

    return frag;
}

bool Cell::isInvincible() const {
    return invincible;
}
