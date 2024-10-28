#include "Virus.h"


const int Virus::merge_count_max = 500;

Virus::Virus(const Point& position) : color { White }, merge_count { 0 } {
    cells.push_back(new Cell { position });
}


void Virus::setUp() {
    for(auto e : cells) {
        e->setUp();
    }
}

void Virus::draw(const HDC& hdc, const Map& map, const RECT& valid_area) const {
    for(auto e : cells) {
        e->draw(hdc, map, valid_area);
    }
}


void Virus::move(const Vector& vector, const Map& map) {
    for(auto e : cells) {
        e->move(vector, map);
    }
}

void Virus::move(const Map& map) {
    for(auto e : cells) {
        e->move(map);
    }
}

void Virus::update() {
    if(merge_count++ < merge_count_max) {
        for(auto e : cells) {
            for(auto o : cells) {
                if(e == o) {
                    continue;
                }
                if((e->position - o->position).scalar() < e->getRadius() + o->getRadius()) {
                    double d = e->getRadius() + o->getRadius() - (e->position - o->position).scalar();
                    e->position += (e->position - o->position).unit() * d/2;
                    o->position += (o->position - e->position).unit() * d/2;
                }
            }
        }
        return;
    }

    for(auto e : cells) {
        e->accelerate = { 0, 0 };
    }

    std::list<Cell*>::iterator iter = cells.begin();
    for(int i=0; i<cells.size(); ++i) {
        std::list<Cell*>::iterator next = iter;
        next++;

        std::list<Cell*>::iterator other_iter = cells.begin();
        for(int k=0; k<cells.size(); ++k) {
            if(iter == other_iter) {
                other_iter++;
                continue;
            }

            if((*other_iter)->collideWith(*iter)) {
                if((*other_iter)->getRadius() >= (*iter)->getRadius()) {
                    (*other_iter)->merge(*iter);
                    delete *iter;
                    cells.erase(iter);
                }
                break;
            }

            other_iter++;
        }

        iter = next;
    }
}

void Virus::collideWith(Virus* other){
    std::list<Cell*>::iterator e_iter = cells.begin();
    for(int i=0; i<cells.size(); ++i) {
        std::list<Cell*>::iterator e_next = e_iter;
        e_next++;

        std::list<Cell*>::iterator other_iter = other->cells.begin();
        for(int k=0; k<other->cells.size(); ++k) {
            std::list<Cell*>::iterator other_next = other_iter;
            other_next++;

            if((*e_iter)->collideWith(*other_iter)) {
                if((*e_iter)->getRadius() > (*other_iter)->getRadius()) {
                    (*e_iter)->eat(*other_iter);
                    delete* other_iter;
                    other->cells.erase(other_iter);
                }
                else if((*e_iter)->getRadius() < (*other_iter)->getRadius()) {
                    (*other_iter)->eat(*e_iter);
                    delete* e_iter;
                    cells.erase(e_iter);
                    break;
                }
            }

            other_iter = other_next;
        }

        e_iter = e_next;
    }
}


void Virus::growUp() {
    for(auto e : cells) {
        e->growUp();
    }
}

Point Virus::getCenterPoint() const {
    Point center { 0, 0 };
    for(auto e : cells) {
        center.x += e->position.x;
        center.y += e->position.y;
    }
    center.x /= cells.size();
    center.y /= cells.size();
    return center;
}

double Virus::getRadius() const {
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


double Virus::getSize() const {
    double result = 0;
    for(auto e : cells) {
        result += pow(e->getRadius(), 2);
    }
    return result;
}


void Virus::split() {
    merge_count = 0;
    int size = cells.size();
    std::list<Cell*>::iterator iter = cells.begin();
    for(int i=0; i<size; i++) {
        Cell* c = (*iter)->split();
        if(c != nullptr) {
            cells.push_back(c);
        }
        iter++;
    }
}