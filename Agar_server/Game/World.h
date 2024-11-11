#include "Map.h"
#include "Cell.h"
#include "Feed.h"
#include "AI.h"
#include "Trap.h"
#include "Player.h"

#include <iostream>
#include <list>
#include <ctime>
#include <unordered_map>
#include <thread>


class World {
private:
    Map map;
    std::unordered_map<uint8_t, Player> players;
    //std::list<AI*> ais;
    std::list<Trap*> traps;
    std::list<Feed*> feeds;

    int feed_erase_count;

public:
    World();

public:
    void setUp();
    void update();

private:
    void updatePlayers();
    //void updateEnemies();
    void updateFeeds();
    void updateTraps();
    void collisionCheck();
    void playerCollisionCheck();
    //void enemyCollisionCheck();
    void trapCollisionCheck();

public:
    void randomGenFeed();
    //void randomGenEnemy();
    void randomGenTrap();
};
