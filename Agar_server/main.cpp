#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

#include "Common.h"
#include "Game/World.h"
#include "../protocol.h"
#include "Util.h"

using namespace std;
using namespace chrono;


const int SERVERPORT = 9000;
const int PACKETSIZEMAX = 512;

const int MAX_CLIENTS = 8;
mutex socket_mutex;
SOCKET sockets[MAX_CLIENTS];

const uint8_t TRAP_ID = 100;
const uint8_t FEED_ID = 200;

World world;


void send_packet(SOCKET socket, const char* buf, int size) {
    // 각 소켓에 데이터 전송
    for(auto& sock : sockets) {
        if(sock == INVALID_SOCKET) {
            continue;
        }

        // 데이터 전송(send()
        int retval = send(sock, buf, size, 0);
        if(retval == SOCKET_ERROR) {
            err_display("[server] send()");
        }

        //cout << "send: " << retval << endl;
    }
}


void run_game(World& world) {
    auto timer = clock();
    auto elapsed = 0;
    int update_time = fps(30);

    // 고정프레임 업데이트
    while(true) {
        elapsed += clock() - timer;
        timer = clock();
        if(elapsed < update_time) {
            continue;
        }

        elapsed -= update_time;

        world.update(update_time);

        auto players = world.getPlayers();
        auto traps = world.getTraps();
        auto feeds = world.getFeeds();

        // World 정보 전송
        SC_WORLD_PACKET packet;
        packet.type = SC_WORLD;
        packet.object_num = 0;

        for(const auto& p : players) {
            auto player = p.second;

            for(const auto& cell : player.cells) {
                SC_OBJECT obj;
                obj.id = player.id;
                obj.x = cell->position.x;
                obj.y = cell->position.y;
                obj.radius = cell->getRadius();
                obj.color = player.color;

                packet.objects.push_back(obj);
                packet.object_num++;
            }
        }

        for(const auto& cell : feeds) {
            SC_OBJECT obj;
            obj.id = FEED_ID;
            obj.x = cell->position.x;
            obj.y = cell->position.y;
            obj.radius = cell->getRadius();
            obj.color = cell->color;

            packet.objects.push_back(obj);
            packet.object_num++;
        }

        for(const auto& cell : traps) {
            SC_OBJECT obj;
            obj.id = TRAP_ID;
            obj.x = cell->position.x;
            obj.y = cell->position.y;
            obj.radius = cell->getRadius();
            obj.color = cell->color;

            packet.objects.push_back(obj);
            packet.object_num++;
        }

        packet.size = sizeof(PACKET_HEADER) + sizeof(int) + sizeof(SC_OBJECT)*packet.object_num;

        send_packet(INVALID_SOCKET, packet.serialize().data(), packet.size);
    }
}


void ProcessClient(SOCKET socket, struct sockaddr_in clientaddr, int id) {
    int retval;
    char buf[PACKETSIZEMAX];

    // 접속한 클라이언트 정보 출력
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
    printf("%s:%d\n", addr, ntohs(clientaddr.sin_port));

    SC_INIT_PACKET init_packet;
    init_packet.type = SC_INIT;
    init_packet.size = sizeof(SC_INIT_PACKET);
    init_packet.id = id;
    retval = send(socket, (const char*)&init_packet, init_packet.size, 0);
    switch(retval) {
        case SOCKET_ERROR:
            err_display("[server] send()");
            break;
        case 0:
            err_display("[server] send()");
            break;
        default:
            break;
    }

    // World에 플레이어 추가
    world.addPlayer(id);
    cout << "Players: \n";
    for(const auto& p : world.getPlayers()) {
        printf(" - id: %2d\n", p.first);
    }

    // 클라이언트와 데이터 통신
    while(true) {
        // 클라이언트 정보 수신
        retval = recv(socket, buf, PACKETSIZEMAX, 0);
        if(retval == SOCKET_ERROR) {
            err_display("[server] recv()");
            break;
        }
        else if(retval == 0) {		// 연결 종료
            break;
        }

        switch(buf[0]) {
            case CS_ACTION: {
                CS_ACTION_PACKET* p = reinterpret_cast<CS_ACTION_PACKET*>(buf);
                //cout << p->mx << ", " << p->my << endl;

                //player data update
                world.setPlayerDestination(id, Point { p->mx, p->my });

                break;
            }
        }
    }

    // World에서 플레이어 삭제
    world.removePlayer(id);
    cout << "Players: \n";
    for(const auto& p : world.getPlayers()) {
        printf(" - id: %2d\n", p.first);
    }

    // 통신 소켓 닫기
    closesocket(socket);

    socket_mutex.lock();
    sockets[id] = INVALID_SOCKET;
    socket_mutex.unlock();
}

int NetworkInitialize() {
    for(auto& sock : sockets) {
        sock = INVALID_SOCKET;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "윈속 초기화 실패" << endl;
        return 1;
    }

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        err_quit("socket()");
    }

    // bind()
    int retval;

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) {
        err_quit("bind()");
    }

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        err_quit("listen()");
    }

    // 데이터 통신에 사용할 소캣
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;

    while (true) {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // 슬롯이 빌때까지 대기
        bool wait = true;

        while(wait) {
            socket_mutex.lock();
            for(int i=0; i<MAX_CLIENTS; ++i) {
                if(sockets[i] == INVALID_SOCKET) {
                    sockets[i] = client_sock;
                    std::thread new_client_thread { ProcessClient, client_sock, clientaddr, i };
                    new_client_thread.detach();

                    wait = false;
                    break;
                }
            }
            socket_mutex.unlock();
        }
    }

    // 대기 소켓 닫기
    closesocket(listen_sock);

    WSACleanup();
}

int main() {
    // ------------------------------------- 게임 실행 -------------------------------------
    //World world;
    world.setUp();

    thread game_logic { [&]() { run_game(world); } };
    game_logic.detach();


    // ------------------------------------- 네트워크 작업 -------------------------------------
    NetworkInitialize();
}
