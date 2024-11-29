#include "../NetworkFunction.h"

#include "GameScene.h"

#include "../Util.h"

#include <sstream>
#include <thread>
#define _USE_MATH_DEFINES
#include <math.h>


GameScene::GameScene(): 
    Scene { Game }, 
    map { },
    id { 0 },
    paused { false }, 
    cam_mode { Fixed }, 
    show_score { false },
    resume_button { L"Resume", { 20, 30 }, 60, 15 }, 
    quit_button { L"Quit", { 20, 60 }, 60, 15 }, 
    game_over_message { L"Game Over", { 10, 30 }, 80, 15 }, 
    game_over { false }, 
    play_time { 0 }, 
    start_time { clock() }, 
    end_time { clock() } 
    //sock { NULL } 
{
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
    objects.clear();

    paused = false;
    game_over = false;
    show_score = false;
    play_time = 0;
    start_time = clock();
    end_time = clock();
}


void GameScene::connect() {
    NetworkInitialize();
    
    connected = true;

    std::thread recv_thread { &GameScene::RecvPacket, this };
    recv_thread.detach();
}

void GameScene::disconnect() {
    SendExitPacket();
    connected = false;
}

CS_ACTION_PACKET GameScene::BuildActionPacket() {
	CS_ACTION_PACKET packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.header.type = CS_ACTION;
    packet.header.size = sizeof(packet);
	if (press_split) packet.flags |= 0x01;
	if (press_spit) packet.flags |= 0x02;

    // 맵 좌표로 변환
    packet.mx = player_destination.x;
    packet.my = player_destination.y;

    return packet;
}

void GameScene::SendActionPacket() {
    CS_ACTION_PACKET packet = BuildActionPacket();
	SendData(&packet, sizeof(CS_ACTION_PACKET));

	press_split = false;
	press_spit = false;
}

void GameScene::SendExitPacket()
{
    CS_EXIT_PACKET packet;
    ZeroMemory(&packet, sizeof(packet));
    packet.header.type = CS_EXIT;
    packet.header.size = sizeof(packet);

    SendData(&packet, sizeof(CS_EXIT_PACKET));
}


void GameScene::RecvPacket() {
    int retval;

    // 데이터 통신에 사용할 변수
    char buf[BUFSIZE];
    size_t len;

    while(connected) {
        // 데이터 수신
        //retval = RecvData(buf);

        //switch(retval) {
        //    case 0:
        //        break;
        //    case SOCKET_ERROR:
        //        err_quit("[client] recv()");
        //        break;
        //    default:
        //        // 데이터 처리
        //        this->ProcessPacket(buf);
        //        break;
        //}

        PACKET_HEADER* packet = ::RecvPacket();
        if(packet == nullptr) {
            break;
        }

        this->ProcessPacket(packet);

        delete packet;
    }

    NetworkFinalize();
}

void GameScene::ProcessPacket(char* buf) {
    // 패킷 분석
    
    // 패킷 처리

    char type = buf[0];
    switch(type) {
        case SC_INIT: {
            SC_INIT_PACKET* packet = (SC_INIT_PACKET*)buf;
            id = packet->id;
            break;
        }
        case SC_WORLD: {
            SC_WORLD_PACKET* packet = (SC_WORLD_PACKET*)buf;
            SC_OBJECT* objects = (SC_OBJECT*)(buf + sizeof(SC_WORLD_PACKET));

            objects_mutex.lock();

            this->objects.clear();
            player.clearCells();

            for(int i=0; i<packet->object_num; ++i) {
                SC_OBJECT& obj = objects[i];
                Cell cell { Point { obj.x, obj.y }, obj.radius };
                cell.color = obj.color;
                this->objects.push_back(cell);
                if(obj.id == id) {
                    player.addCell(&this->objects.back());
                }
                //this->objects.insert_or_assign(obj.id, cell);
            }

            objects_mutex.unlock();

            //Beep(1000, 100);
            break;
        }
    }
}

void GameScene::ProcessPacket(PACKET_HEADER* packet) {
    switch(packet->type) {
        case SC_INIT: {
            SC_INIT_PACKET* p = (SC_INIT_PACKET*)packet;
            id = p->id;
            break;
        }
        case SC_WORLD: {
            SC_WORLD_PACKET* p = (SC_WORLD_PACKET*)packet;

            objects_mutex.lock();

            this->objects.clear();
            player.clearCells();

            game_over = true;
            for(const auto& obj : p->objects) {
                Cell cell { Point { obj.x, obj.y }, obj.radius };
                cell.color = obj.color;
                this->objects.push_back(cell);
                if(obj.id == id) {
                    player.addCell(&this->objects.back());
                    game_over = false;
                }
            }

            objects_mutex.unlock();

            //Beep(1000, 100);
            break;
        }
    }
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
    RECT map_area { 0.0, 0.0, 0.0, 0.0 };
    double map_area_w = 0.0;
    double map_area_h = 0.0;

    switch(cam_mode) {
        case Fixed: {
            map_area = map.absoluteArea(valid_area);
            map_area_w = map_area.right - map_area.left;
            map_area_h = map_area.bottom - map_area.top;

            break;
        }

        case Dynamic: {
            objects_mutex.lock();
            RECT view_area = getViewArea();
            objects_mutex.unlock();

            map_area = map.absoluteArea(view_area);
            map_area_w = map_area.right - map_area.left;
            map_area_h = map_area.bottom - map_area.top;

            break;
        }

        default:
            err_quit("Invalid Camera Mode");
    }

    // 맵 좌표로 변환
    player_destination.x = ((point.x - map_area.left) / map_area_w) * map.getWidth();
    player_destination.y = ((point.y - map_area.top) / map_area_h) * map.getHeight();

	SendActionPacket();
}


void GameScene::draw(const HDC& hdc) const {
    drawBackground(hdc, White);

    objects_mutex.lock();

    RECT view_area = getViewArea();

    map.draw(hdc, view_area);

    for(auto& e : objects) {
        e.draw(hdc, map, view_area);
        //e.second.draw(hdc, map, view_area);
    }

    // 플레이어 이동방향
    if(!game_over) {
        for(auto e : player.getCells()) {
            Vector pv = player_destination - e->position;
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

    objects_mutex.unlock();

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
            double W = valid_area.left + horizontal_length/2;
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
    //ss << L"Size: " << player.getSize() * 10;
    //text = ss.str();
    //score.text = ss.str();
    //score.show(hdc, valid_area);
    //ss.str(L"");

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

    press_split = true;
    //player.split();

    return None;
}

ButtonID GameScene::clickR(const POINT& point) {
    if(paused) {
        return None;
    }
    if(game_over) {
        return None;
    }

    press_spit = true;
    //for(auto e : player.cells) {
    //    Cell* c = e->spit();
    //    if(c != nullptr) {
    //        Feed* f = new Feed { c->position, c->getRadius() };
    //        f->color = c->color;
    //        f->position = c->position;
    //        f->velocity = c->velocity;
    //        feeds.push_back(f);
    //        delete c;
    //    }
    //}

    return None;
}


void GameScene::mouseMove(const POINT& point) {
    if(paused || game_over) {
        return;
    }

    mouse_position.x = point.x;
    mouse_position.y = point.y;
}
