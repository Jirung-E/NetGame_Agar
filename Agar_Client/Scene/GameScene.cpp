#include "../Common.h"

#include "GameScene.h"

#include "../Util.h"

#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>


GameScene::GameScene() : Scene { Game }, map { },
player { Point { map.getWidth()/2.0, map.getHeight()/2.0 } }, paused { false }, cam_mode { Fixed }, show_score { false },
resume_button { L"Resume", { 20, 30 }, 60, 15 }, quit_button { L"Quit", { 20, 60 }, 60, 15 }, 
game_over_message { L"Game Over", { 10, 30 }, 80, 15 }, game_over { false }, feed_erase_count { 0 },
play_time { 0 }, start_time { clock() }, end_time { clock() } {
    player.color = Green;
    resume_button.border_color = Gray;
    resume_button.border_width = 3;
    resume_button.id = ResumeGame;
    quit_button.border_color = Gray;
    quit_button.border_width = 3;
    quit_button.id = QuitGame;
    game_over_message.text_color = Red;
    game_over_message.background_color = LightGray;
    game_over_message.bold = 4;
}


void GameScene::setUp() {
    player.cells.clear();
    player.cells.push_back(new Cell { Point { map.getWidth()/2.0, map.getHeight()/2.0 } });
    player.cells.front()->color = player.color;
    feeds.clear();
    enemies.clear();
    paused = false;
    game_over = false;
    show_score = false;
    play_time = 0;
    start_time = clock();
    end_time = clock();
    feed_erase_count = 0;
    traps.clear();

    randomGenFeed();
    randomGenFeed();
    randomGenEnemy();
    for(int i=0; i<5; i++) {
        randomGenTrap();
    }



    // 윈속 초기화
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return;
    }

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) {
        err_quit("socket()");
    }

    // connect
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);
    serveraddr.sin_port = htons(9000);

    int retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(retval == SOCKET_ERROR) {
        err_quit("connect()");
    }

    // 데이터 통신에 사용할 변수	
    char buf[6] = "Hello";

    // 서버와 데이터 통신
    retval = send(sock, buf, 6, 0);
    if(retval == SOCKET_ERROR) {
        err_quit("send()");
    }

    retval = recv(sock, buf, 6, 0);
    if(retval == SOCKET_ERROR) {
        err_quit("recv()");
    }

    // 소켓 닫기
    closesocket(sock);

    // 윈속 종료
    WSACleanup();
}


void GameScene::update(const POINT& point) {
    if(paused) {
        return;
    }
    if(!game_over) {
        end_time = clock();
        play_time += end_time - start_time;
        start_time = clock();
        updatePlayer(point);
    }
    updateFeeds();
    updateEnemy();
    updateTraps();
    collisionCheck();
}

void GameScene::togglePauseState() {
    if(paused) {
        resume();
    }
    else {
        pause();
    }
}

void GameScene::pause() {
    if(!game_over) {
        paused = true;
    }
}

void GameScene::resume() {
    if(!game_over) {
        paused = false;
        end_time = clock();
        start_time = clock();
    }
}


void GameScene::updatePlayer(const POINT& point) {
    RECT view_area = getViewArea();

    for(auto e : player.cells) {
        POINT p;
        switch(cam_mode) {
        case Fixed:
            p = e->absolutePosition(map, valid_area);
            break;
        case Dynamic:
            POINT center = { (valid_area.left + valid_area.right)/2, (valid_area.top + valid_area.bottom)/2 };
            Vector v = (e->position - player.getCenterPoint()) * (view_area.right-view_area.left) / map.getWidth();
            p = { LONG(center.x + v.x), LONG(center.y + v.y) };
            break;
        }
        Vector dir = (Point { (double)point.x, (double)point.y } - Point { (double)p.x, (double)p.y }) / 50;
        e->move(dir, map);
        e->growUp();
    }
}

void GameScene::updateEnemy() {
    for(auto& e : enemies) {
        e->growUp();

        std::list<Cell*> detected_cells;
        const double running_range = 2;
        const double detect_range = 1.5;
        double range = e->running ? running_range : detect_range;

        // 영역 안에 들어온 적 찾기
        // 플레이어 찾기
        if(!game_over) {
            for(auto p : player.cells) {
                for(auto o : e->cells) {
                    if((p->position - o->position).scalar() - p->getRadius() - o->getRadius() <= range) {
                        detected_cells.push_back(p);
                    }
                }
            }
        }

        // 다른 적 찾기
        for(auto& o : enemies) {
            if(e == o) {
                continue;
            }
            for(auto e_elem : e->cells) {
                for(auto o_elem : o->cells) {
                    if((o_elem->position - e_elem->position).scalar() - o_elem->getRadius() - e_elem->getRadius() <= range) {
                        detected_cells.push_back(o_elem);
                    }
                }
            }
        }

        // 트랩 찾기
        for(auto e_elem : e->cells) {
            for(auto t : traps) {
                if(e_elem->getRadius() > t->getRadius()) {
                    if((t->position - e_elem->position).scalar() - t->getRadius() - e_elem->getRadius() <= range/10) {
                        detected_cells.push_back(t);
                    }
                }
            }
        }

        e->running = false;
        e->chasing = false;
        Vector dir = { 0, 0 };
        Cell* target = nullptr;
        double max_r = 0;
        for(auto e_elem : e->cells) {
            for(auto o : detected_cells) {
                Vector to_me = e_elem->position - o->position;
                double dist = to_me.scalar() - e_elem->getRadius() - o->getRadius();
                if(o->getRadius() >= e_elem->getRadius() || o->isInvincible()) {
                    // 도망갈준비
                    if(o->isInvincible()) {
                        dir += to_me.unit() / to_me.scalar() / 10;
                    }
                    else {
                        dir += to_me.unit() / to_me.scalar();
                    }
                    e->running = true;
                }
                else {
                    // 쫒아갈준비
                    if(dist <= detect_range) {
                        if(o->getRadius() > max_r) {
                            max_r = o->getRadius();
                            target = o;
                            e->chasing = true;
                        }
                    }
                }
            }
        }

        if(e->running) {
            e->move(dir.unit(), map);
            continue;
        }
        if(e->chasing) {
            for(auto e_elem : e->cells) {
                e_elem->move((target->position - e_elem->position).unit(), map);
            }
            continue;
        }

        // 먹이를 쫒아감
        target = nullptr;
        double min_dist = 10000000;
        for(auto o : feeds) {
            Vector v = o->position - e->getCenterPoint();
            if(v.scalar() < min_dist) {
                min_dist = v.scalar();
                target = o;
            }
        }
        if(target == nullptr) {
            e->randomStroll();
            e->move(map);
        }
        else {
            for(auto e_elem : e->cells) {
                e_elem->move((target->position - e_elem->position).unit(), map);
            }
        }
    }
}

void GameScene::updateFeeds() {
    for(auto e : feeds) {
        if(e->velocity == Vector { 0, 0 }) {
            continue;
        }
        e->move(map);
    }
}

void GameScene::updateTraps() {
    for(auto e : traps) {
        e->move(map);
    }
}

void GameScene::collisionCheck() {
    if(!game_over) {
        playerCollisionCheck();
    }
    enemyCollisionCheck();
    trapCollisionCheck();
}

void GameScene::playerCollisionCheck() {
    player.update();

    // 먹이를 먹음
    std::list<Feed*>::iterator feed_iter = feeds.begin();
    for(int i=0; i<feeds.size(); ++i) {
        std::list<Feed*>::iterator next = feed_iter;
        next++;
        for(auto e : player.cells) {
            if(e->collideWith(*feed_iter)) {
                e->eat(*feed_iter);
                delete* feed_iter;
                feeds.erase(feed_iter);
                break;
            }
        }
        feed_iter = next;
    }

    // 적과 충돌
    std::list<EnemyCell*>::iterator e_iter = enemies.begin();
    for(int i=0; i<enemies.size(); ++i) {
        std::list<EnemyCell*>::iterator next = e_iter;
        next++;

        std::list<Cell*>::iterator c_iter = (*e_iter)->cells.begin();
        for(int m=0; m<(*e_iter)->cells.size(); ++m) {
            std::list<Cell*>::iterator c_next = c_iter;
            c_next++;

            std::list<Cell*>::iterator p_iter = player.cells.begin();
            for(int i=0; i<player.cells.size(); ++i) {
                std::list<Cell*>::iterator p_next = p_iter;
                p_next++;

                if((*p_iter)->collideWith(*c_iter)) {
                    if((*p_iter)->getRadius() > (*c_iter)->getRadius()) {
                        (*p_iter)->eat(*c_iter);
                        delete* c_iter;
                        (*e_iter)->cells.erase(c_iter);
                    }
                    else if((*p_iter)->getRadius() < (*c_iter)->getRadius()) {
                        (*c_iter)->eat(*p_iter);
                        if(player.cells.size() > 1) {
                            delete* p_iter;
                            player.cells.erase(p_iter);
                        }
                        else {
                            // 게임오버
                            game_over = true;
                        }
                    }
                    break;
                }

                p_iter = p_next;
            }

            c_iter = c_next;
        }

        e_iter = next;
    }
}

void GameScene::enemyCollisionCheck() {
    for(auto e : enemies) {
        e->update();
    }

    // 적이 먹이를 먹음
    for(auto& e : enemies) {
        for(auto o : e->cells) {
            std::list<Feed*>::iterator feed_iter = feeds.begin();
            for(int i=0; i<feeds.size(); ++i) {
                std::list<Feed*>::iterator next = feed_iter;
                next++;
                if(o->collideWith(*feed_iter)) {
                    o->eat(*feed_iter);
                    delete* feed_iter;
                    feeds.erase(feed_iter);
                }
                feed_iter = next;
            }
        }
    }

    // 적이 적과 충돌
    std::list<EnemyCell*>::iterator e_iter = enemies.begin();
    for(int i=0; i<enemies.size(); ++i) {
        std::list<EnemyCell*>::iterator other_iter = enemies.begin();
        bool e_iter_deleted = false;
        for(int k=0; k<enemies.size(); ++k) {
            if(other_iter == e_iter) {
                continue;
            }
            bool other_iter_deleted = false;
            (*e_iter)->collideWith(*other_iter);

            if((*other_iter)->cells.empty()) {
                std::list<EnemyCell*>::iterator other_next = other_iter;
                other_next++;
                delete* other_iter;
                enemies.erase(other_iter);
                other_iter = other_next;
                other_iter_deleted = true;
            }
            else if((*e_iter)->cells.empty()) {
                std::list<EnemyCell*>::iterator e_next = e_iter;
                e_next++;
                delete* e_iter;
                enemies.erase(e_iter);
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

void GameScene::trapCollisionCheck() {
    std::list<Trap*>::iterator iter = traps.begin();
    for(int i=0; i<traps.size(); ++i) {
        std::list<Trap*>::iterator next = iter;
        next++;

        bool erased = false;
        for(auto p : player.cells) {
            if((*iter)->collideWith(p)) {
                if((*iter)->getRadius() < p->getRadius()) {
                    std::list<Cell*> frag = p->explode();
                    for(auto f : frag) {
                        player.cells.push_back(f);
                    }
                    if(player.merge_count > Virus::merge_count_max/10*9) {
                        if(player.merge_count >= Virus::merge_count_max) {
                            player.merge_count = 0;
                        }
                        else {
                            player.merge_count = Virus::merge_count_max/10*9;
                        }
                    }
                    erased = true;
                    break;
                }
            }
        }
        if(!erased) {
            for(auto e : enemies) {
                for(auto e_elem : e->cells) {
                    if((*iter)->collideWith(e_elem)) {
                        if((*iter)->getRadius() < e_elem->getRadius()) {
                            std::list<Cell*> frag = e_elem->explode();
                            for(auto f : frag) {
                                e->cells.push_back(f);
                            }
                            if(e->merge_count > Virus::merge_count_max/10*9) {
                                if(e->merge_count >= Virus::merge_count_max) {
                                    e->merge_count = 0;
                                }
                                else {
                                    e->merge_count = Virus::merge_count_max/10*9;
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
        }

        if(erased) {
            delete *iter;
            traps.erase(iter);
        }

        iter = next;
    }
}


void GameScene::draw(const HDC& hdc) const {
    drawBackground(hdc, White);

    RECT view_area = getViewArea();

    map.draw(hdc, view_area);
    for(auto e : feeds) {
        e->draw(hdc, map, view_area);
    }
    for(auto e : enemies) {
        e->draw(hdc, map, view_area);
    }
    if(!game_over) {
        player.draw(hdc, map, view_area);
    }
    for(auto e : traps) {
        e->draw(hdc, map, view_area);
    }

    // 플레이어 이동방향
    if(!game_over) {
        for(auto e : player.cells) {
            Vector pv = e->velocity;
            if(pv.scalar() > 1) {
                pv = pv.unit();
            }
            Vector v = pv * (view_area.right-view_area.left)/map.getWidth() * e->getRadius()*2;
            if(v.scalar() != 0) {
                HPEN pen = CreatePen(PS_SOLID, (view_area.right-view_area.left)/map.getWidth() * e->getRadius() / 3, LightGray);
                HPEN old = (HPEN)SelectObject(hdc, pen);

                SetROP2(hdc, R2_MASKPEN);

                POINT p = e->absolutePosition(map, view_area);
                MoveToEx(hdc, p.x+v.x, p.y+v.y, NULL);
                Vector v1 = { -v.x/3, -v.y/3 };
                double th = atan(v.y/v.x);
                if(v.x < 0) th += M_PI;
                LineTo(hdc, p.x+v.x - v1.scalar()*cos(-M_PI/4 + th), p.y+v.y - v1.scalar()*sin(-M_PI/4 + th));
                MoveToEx(hdc, p.x+v.x, p.y+v.y, NULL);
                LineTo(hdc, p.x+v.x - v1.scalar()*cos(M_PI/4 + th), p.y+v.y - v1.scalar()*sin(M_PI/4 + th));

                SetROP2(hdc, R2_COPYPEN);

                SelectObject(hdc, old);
                DeleteObject(pen);
            }
        }
    }

    if(show_score) {
        drawScore(hdc);
    }

    if(paused) {
        drawPauseScene(hdc);
    }
    else if(game_over) {
        drawGameOverScene(hdc);
    }
}

RECT GameScene::getViewArea() const {
    switch(cam_mode) {
    case Fixed:
        return valid_area;
    case Dynamic:
        {
            // 보이는 영역의 크기 설정
            double r0 = Cell::min_radius;
            //double rm = Cell::max_radius;
            double rm = sqrt(pow(horizontal_length/map.getWidth(), 2) + pow(vertical_length/map.getHeight(), 2))/2;
            double w = horizontal_length/map.getWidth() * (-(rm-20*r0) / pow(r0-rm, 2) * pow(sqrt(player.getSize()) - rm, 2) + rm);
            //double w = horizontal_length/map.getWidth() * (10*r0*pow(M_E, -(player.getRadius()-r0)) + 10*player.getRadius());
            double W = valid_area.left + horizontal_length/2.0;
            double A = W*(W/w - 1);

            // 플레이어가 중앙에 오도록 평행이동
            Point p = player.getCenterPoint();
            p += Vector { -map.getWidth()/2.0, -map.getHeight()/2.0 };
            double px = W/w * horizontal_length/map.getWidth() * p.x;
            double py = W/w * horizontal_length/map.getWidth() * p.y;

            return {
                LONG(floor(valid_area.left - px - A)),
                LONG(floor(valid_area.top - py - A)),
                LONG(floor(valid_area.right - px + A)),
                LONG(floor(valid_area.bottom - py + A))
            };
        }
    }
    return valid_area;
}

void GameScene::drawScore(const HDC& hdc) const {
    TextBox score { L"", { 0, 0 }, 50, 5 };
    score.transparent_background = true;
    score.transparent_border = true;
    score.align = DT_LEFT;
    score.square = false;
    score.bold = 4;
    score.italic = true;

    tstring text;
    std::basic_stringstream<TCHAR> ss;

    // 크기 출력
    ss << L"Size: " << player.getSize() * 10;
    text = ss.str();
    score.text = ss.str();
    score.show(hdc, valid_area);
    ss.str(L"");

    // 플레이 시간 출력
    ss << L"PlayTime: " << play_time/1000 << "\"";
    text = ss.str();
    score.text = ss.str();
    score.position.y += 5;
    score.show(hdc, valid_area);
    ss.str(L"");
}

void GameScene::drawPauseScene(const HDC& hdc) const {
    resume_button.show(hdc, valid_area);
    quit_button.show(hdc, valid_area);
}

void GameScene::drawGameOverScene(const HDC& hdc) const {
    game_over_message.show(hdc, valid_area);
    quit_button.show(hdc, valid_area);

    TextBox score { L"", { 0, 45 }, 100, 7 };
    score.transparent_background = true;
    score.transparent_border = true;
    score.bold = 4;

    tstring text;
    std::basic_stringstream<TCHAR> ss;

    // 플레이 시간 출력
    ss << L"PlayTime: " << play_time/1000 << "\"";
    text = ss.str();
    score.text = ss.str();
    score.show(hdc, valid_area);
    ss.str(L"");
}


void GameScene::setCameraMode(const CameraMode& mode) {
    if(paused) {
        return;
    }
    cam_mode = mode;
}

void GameScene::randomGenFeed() {
    if(paused) {
        return;
    }

    feed_erase_count++;
    if(feeds.size() > 200 || feed_erase_count >= 7) {
        feed_erase_count = 0;
        // 오래된거(앞에 10개) 제거
        for(int i=0; i<10; ++i) {
            if(feeds.size() > 0) {
                feeds.pop_front();
            }
        }
    }

    for(int i=0; i<20; ++i) {
        feeds.push_back(new Feed { Point { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1), 
                                           getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) } });
    }
}

void GameScene::randomGenEnemy() {
    if(paused) {
        return;
    }

    if(enemies.size() > 10) {
        return;
    }

    for(int i=0; i<1; ++i) {
        enemies.push_back(new EnemyCell { Point { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1),
                                                  getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) } });
        enemies.back()->color = getRandomColor();
    }
}

void GameScene::randomGenTrap() {
    if(paused) {
        return;
    }

    if(traps.size() >= (map.getWidth() + map.getHeight())/8) {
        return;
    }

    traps.push_back(new Trap { Point { getRandomNumberOf(Range { 1.0, (double)map.getWidth()-1 }, 0.1),
                                       getRandomNumberOf(Range { 1.0, (double)map.getHeight()-1 }, 0.1) } });
}


ButtonID GameScene::clickL(const POINT& point) {
    if(paused) {
        RECT r = resume_button.absoluteArea(valid_area);
        if(PtInRect(&r, point)) {
            return resume_button.id;
        }
        r = quit_button.absoluteArea(valid_area);
        if(PtInRect(&r, point)) {
            return quit_button.id;
        }
        return None;
    }
    if(game_over) {
        RECT r = quit_button.absoluteArea(valid_area);
        if(PtInRect(&r, point)) {
            return quit_button.id;
        }
        return None;
    }

    player.split();

    return None;
}

ButtonID GameScene::clickR(const POINT& point) {
    if(paused) {
        return None;
    }
    if(game_over) {
        return None;
    }

    for(auto e : player.cells) {
        Cell* c = e->spit();
        if(c != nullptr) {
            Feed* f = new Feed { c->position, c->getRadius() };
            f->color = c->color;
            f->position = c->position;
            f->velocity = c->velocity;
            feeds.push_back(f);
            delete c;
        }
    }

    return None;
}