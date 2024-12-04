#include "World.h"


World::World():
    map { },
    trap_gen_interval { 4000 },
    trap_gen_timer { 0 },
    feed_gen_interval { 2000 },
    feed_gen_timer { 0 }
{

}


void World::setUp() {
    for(int i=0; i<5; i++) {
        randomGenFeed();
        randomGenTrap();
    }
}

void World::update(int elapsed) {
    trap_gen_timer += elapsed;
    feed_gen_timer += elapsed;
    if(trap_gen_timer >= trap_gen_interval) {
        trap_gen_timer -= trap_gen_interval;
        randomGenTrap();
    }
    if(feed_gen_timer >= feed_gen_interval) {
        feed_gen_timer -= feed_gen_interval;
        randomGenFeed();
    }

    updatePlayers();
    updateFeeds();
    updateTraps();
    collisionCheck();
}


void World::addPlayer(uint8_t id) {
    Point p { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1),
              getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) 
    };
    players.emplace(id, id, p);
}

void World::removePlayer(uint8_t id) {
    players.erase(id);
}

const std::unordered_map<uint8_t, Player>& World::getPlayers() const {
    return players;
}


void World::splitPlayer(uint8_t id) {
    auto it = players.find(id);
    if(it != players.end()) {
        it->second.split();
    }
}

void World::spitPlayer(uint8_t id) {
    auto it = players.find(id);
    if(it == players.end()) {
        return;
    }

    auto& player = it->second;
    for(auto e : player.cells) {
        Cell* c = e->spit();
        if(c != nullptr) {
            Feed* f = new Feed { c->position, c->getRadius() };
            f->color = player.color;
            f->position = c->position;
            f->velocity = c->velocity;
            feeds.push_back(f);
            delete c;
        }
    }
}


void World::setPlayerDestination(uint8_t id, const Point& dest) {
    auto it = players.find(id);
    if(it != players.end()) {
        it->second.destination = dest;
    }
}


const std::list<Trap*>& World::getTraps() const {
    return traps;
}

const std::list<Feed*>& World::getFeeds() const {
    return feeds;
}


void World::updatePlayers() {
    for(auto& t_player : players) {
        auto& player = t_player.second;
        for(auto& e : player.cells) {
            Vector dir { player.destination - e->position };
            e->move(dir, map);  // destinationø° µ˚∂Û ¿Ãµø
            e->growUp();
        }
        player.update();
    }
}


void World::updateFeeds() {
    for(auto e : feeds) {
        if(e->velocity == Vector { 0, 0 }) {
            continue;
        }
        e->move(map);
    }
}

void World::updateTraps() {
    for(auto e : traps) {
        e->move(map);
    }
}


void World::collisionCheck() {
    playerCollisionCheck();
    trapCollisionCheck();
}

void World::playerCollisionCheck() {
    // ∏‘¿Ã∏¶ ∏‘¿Ω
    for(auto& p : players) {
        auto& player = p.second;
        for(auto cell : player.cells) {
            std::list<Feed*>::iterator feed_iter = feeds.begin();
            for(int i=0; i<feeds.size(); ++i) {
                std::list<Feed*>::iterator next = feed_iter;
                next++;
                if(cell->collideWith(*feed_iter)) {
                    cell->eat(*feed_iter);
                    delete* feed_iter;
                    feeds.erase(feed_iter);
                }
                feed_iter = next;
            }
        }
    }

    auto e_iter = players.begin();
    for(int i=0; i<players.size(); ++i) {
        auto other_iter = players.begin();
        bool e_iter_deleted = false;

        for(int k=0; k<players.size(); ++k) {
            if(other_iter == e_iter) {
                continue;
            }

            auto& self = e_iter->second;
            auto& other = other_iter->second;

            bool other_iter_deleted = false;
            self.collideWith(other);
    
            if(other.cells.empty()) {
                auto other_next = other_iter;
                other_next++;
                //delete *other_iter;
                players.erase(other_iter);
                other_iter = other_next;
                other_iter_deleted = true;
            }
            else if(self.cells.empty()) {
                auto e_next = e_iter;
                e_next++;
                //delete* e_iter;
                players.erase(e_iter);
                e_iter = e_next;
                e_iter_deleted = true;
                break;
            }
    
            if(!other_iter_deleted) {
                other_iter++;
            }
            other_iter_deleted = false;
        }

        if(!e_iter_deleted) {
            e_iter++;
        }
        e_iter_deleted = false;
    }
}


void World::trapCollisionCheck() {
    std::list<Trap*>::iterator iter = traps.begin();
    for(int i=0; i<traps.size(); ++i) {
        std::list<Trap*>::iterator next = iter;
        next++;

        bool erased = false;

        for(auto& e : players) {
            auto& player = e.second;
            for(auto e_elem : player.cells) {
                if((*iter)->collideWith(e_elem)) {
                    if((*iter)->getRadius() < e_elem->getRadius()) {
                        std::list<Cell*> frag = e_elem->explode();
                        for(auto f : frag) {
                            player.cells.push_back(f);
                        }
                        if(player.merge_count > Player::merge_count_max/10*9) {
                            if(player.merge_count >= Player::merge_count_max) {
                                player.merge_count = 0;
                            }
                            else {
                                player.merge_count = Player::merge_count_max/10*9;
                            }
                        }
                        erased = true;
                        break;
                    }
                }
            }
            if(erased) {
                break;
            }
        }

        if(erased) {
            delete* iter;
            traps.erase(iter);
        }

        iter = next;
    }
}


void World::randomGenFeed() {
    if(feeds.size() > 500) {
        return;
    }

    for(int i=0; i<20; ++i) {
        feeds.push_back(new Feed { Point { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1),
                                           getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) } });
    }
}

void World::randomGenTrap() {
    if(traps.size() >= (map.getWidth() + map.getHeight())/8) {
        return;
    }

    traps.push_back(new Trap { Point { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1),
                                       getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) } });
}
