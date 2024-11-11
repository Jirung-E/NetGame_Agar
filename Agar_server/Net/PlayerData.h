#pragma once

#include "../Math/Point.h"
#include "../Math/Vector.h"
#include "../Game/Cell.h"


struct PlayerData {
    Point destination;
    
    COLORREF color;
    std::list<Cell*> cells;
};
