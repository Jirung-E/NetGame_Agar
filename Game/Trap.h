#include "Cell.h"


class Trap : public Cell {
private:
    int stroll_count;

private:
    int spawn_animation_count;
    const int anim_max;

public:
    Trap(const Point& position);

public:
    void randomStroll();

    void draw(const HDC& hdc, const Map& map, const RECT& valid_area);

    void move(const Map& map);

    bool collideWith(const Cell* other);
};