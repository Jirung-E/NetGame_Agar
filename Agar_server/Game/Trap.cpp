#include "Trap.h"

#define _USE_MATH_DEFINES
#include <math.h>


Trap::Trap(const Point& position) : Cell { position, 0.6 }, stroll_count { 0 }, spawn_animation_count { 0 }, anim_max { 200 } {
    invincible = true;
    color = getRandomColor(0, 0, 0, Range { 50, 100 });
}


void Trap::randomStroll() {
    if(spawn_animation_count < anim_max) {
        spawn_animation_count++;
        return;
    }
    if(stroll_count++ < 150) {
        return;
    }
    velocity = Vector { getRandomNumberOf(Range { -1, 1 }, 0.1), getRandomNumberOf(Range { -1, 1 }, 0.1) };
    velocity = velocity.unit();
    velocity = velocity/2;
    stroll_count = 0;
}


void Trap::draw(const HDC& hdc, const Map& map, const RECT& valid_area) {
    RECT area = map.absoluteArea(valid_area);
    int w = area.right - area.left;
    int h = area.bottom - area.top;
    int x = area.left + position.x / map.getWidth() * w;
    int y = area.top + position.y / map.getHeight() * h;
    int r = radius / map.getWidth() * w;

    COLORREF bg = color;
    COLORREF cross = Red;
    if(spawn_animation_count < anim_max) {
        bg = bg | LightGray;
        cross = Gray;
    }
    
    SetROP2(hdc, R2_MASKPEN);

    HPEN bd_pen = CreatePen(PS_SOLID, 2, Black);
    HPEN old_pen = (HPEN)SelectObject(hdc, bd_pen);
    HBRUSH br = CreateHatchBrush(HS_DIAGCROSS, cross);
    HBRUSH old_br = (HBRUSH)SelectObject(hdc, br);
    COLORREF old_c = SetBkColor(hdc, bg);

    Ellipse(hdc, x-r, y-r, x+r, y+r);
    
    SetBkColor(hdc, old_c);
    SelectObject(hdc, old_br);
    DeleteObject(br);
    SelectObject(hdc, old_pen);
    DeleteObject(bd_pen);

    SetROP2(hdc, R2_COPYPEN);

    if(spawn_animation_count < anim_max) {
        HPEN pen = CreatePen(PS_SOLID, 4, Red);
        HPEN old_pen = (HPEN)SelectObject(hdc, pen);
        Arc(hdc, x-r, y-r, x+r, y+r,
            x, y-r,
            int(x+r*cos(-M_PI/2 + spawn_animation_count*(2*M_PI)/anim_max)), int(y+r*sin(-M_PI/2 + spawn_animation_count*(2*M_PI)/anim_max)));
        SelectObject(hdc, old_pen);
        DeleteObject(pen);
    }

}


void Trap::move(const Map& map) {
    Cell::move(map);
    randomStroll();
}


bool Trap::collideWith(const Cell* other) {
    if(spawn_animation_count < anim_max) {
        return false;
    }
    double dist_between_center = Vector(other->position - position).scalar();
    if(dist_between_center <= radius/3*2 + other->getRadius()) {
        return true;
    }
    return false;
}