#include "Feed.h"


Feed::Feed(const Point& position, const double radius) : Cell { position, radius }, move_count { 25 } {
    color = getRandomColor();
}


void Feed::move(const Map& map) {
    if(move_count <= 0) {
        return;
    }

    Cell::move(map);
    move_count--;
}