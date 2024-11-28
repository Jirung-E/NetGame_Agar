#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "Common.h"
#include "Game/World.h"
#include "../protocol.h"
#include "Util.h"

using namespace std;
using namespace chrono;

// 상수 및 전역 변수 정의
const int SERVERPORT = 9000;
const int PACKETSIZEMAX = 512;
const int MAX_CLIENTS = 8;

mutex socket_mutex;
SOCKET sockets[MAX_CLIENTS];

const uint8_t TRAP_ID = 100;
const uint8_t FEED_ID = 200;

World world;

// 작업 큐 정의
struct Packet {
    int id;
    vector<char> data;
};

queue<Packet> packet_queue;
mutex queue_mutex;
condition_variable queue_condition;

// 패킷을 전송하는 함수
void send_packet(SOCKET socket, const char* buf, int size) {
    for (auto& sock : sockets) {
        if (sock == INVALID_SOCKET) {
            continue;
        }

        int retval = send(sock, buf, size, 0);
        if (retval == SOCKET_ERROR) {
            err_display("[server] send()");
        }
    }
}

// 게임 로직 처리
void run_game(World& world) {
    auto timer = clock();
    auto elapsed = 0;
    int update_time = fps(30);

    while (true) {
        elapsed += clock() - timer;
        timer = clock();
        if (elapsed < update_time) {
            continue;
        }

        elapsed -= update_time;
        world.update(update_time);

        auto players = world.getPlayers();
        auto traps = world.getTraps();
        auto feeds = world.getFeeds();

        SC_WORLD_PACKET packet;
        packet.type = SC_WORLD;
        packet.object_num = 0;

        for (const auto& p : players) {
            auto player = p.second;
            for (const auto& cell : player.cells) {
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

        for (const auto& cell : feeds) {
            SC_OBJECT obj;
            obj.id = FEED_ID;
            obj.x = cell->position.x;
            obj.y = cell->position.y;
            obj.radius = cell->getRadius();
            obj.color = cell->color;

            packet.objects.push_back(obj);
            packet.object_num++;
        }

        for (const auto& cell : traps) {
            SC_OBJECT obj;
            obj.id = TRAP_ID;
            obj.x = cell->position.x;
            obj.y = cell->position.y;
            obj.radius = cell->getRadius();
            obj.color = cell->color;

            packet.objects.push_back(obj);
            packet.object_num++;
        }

        packet.size = sizeof(PACKET_HEADER) + sizeof(int) + sizeof(SC_OBJECT) * packet.object_num;
        send_packet(INVALID_SOCKET, packet.serialize().data(), packet.size);
    }
}


void ProcessPacket(int id, char* buf) {
    char packetType = buf[0];

    switch (packetType) {
    case CS_ACTION: {
        CS_ACTION_PACKET* p = reinterpret_cast<CS_ACTION_PACKET*>(buf);

        world.setPlayerDestination(id, Point{ p->mx, p->my });
        if (p->flags & 0b01) {
            world.splitPlayer(id);
        }
        if (p->flags & 0b10) {
            world.spitPlayer(id);
        }
        break;
    }
    case CS_EXIT: {
        // closesocket -> ProcessClient에서 처리
        // world에서 제거 -> ProcessClient에서 처리
        break;
    }
    }
}

// 패킷 처리 스레드
void process_packet_thread() {
    while (true) {
        unique_lock<mutex> lock(queue_mutex);
        queue_condition.wait(lock, [] { return !packet_queue.empty(); });

        Packet packet = packet_queue.front();
        packet_queue.pop();
        lock.unlock();

        // 패킷 처리
        ProcessPacket(packet.id, packet.data.data());
    }
}
// 클라이언트 패킷 처리
void ProcessClient(SOCKET socket, struct sockaddr_in clientaddr, int id) {
    int retval;
    char buf[PACKETSIZEMAX];

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
    printf("%s:%d\n", addr, ntohs(clientaddr.sin_port));

    SC_INIT_PACKET init_packet;
    init_packet.type = SC_INIT;
    init_packet.size = sizeof(SC_INIT_PACKET);
    init_packet.id = id;
    retval = send(socket, (const char*)&init_packet, init_packet.size, 0);

    if (retval == SOCKET_ERROR) {
        err_display("[server] send()");
    }

    world.addPlayer(id);

    while (true) {
        retval = recv(socket, buf, PACKETSIZEMAX, 0);
        if (retval == SOCKET_ERROR || retval == 0) {
            break;
        }

        // 패킷 큐에 추가
        {
            lock_guard<mutex> lock(queue_mutex);
            packet_queue.push({id, vector<char>(buf, buf + retval)});
        }
        queue_condition.notify_one();
    }

    world.removePlayer(id);
    closesocket(socket);

    lock_guard<mutex> lock(socket_mutex);
    sockets[id] = INVALID_SOCKET;
}

// 네트워크 초기화
int NetworkInitialize() {
    for (auto& sock : sockets) {
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

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    if (bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        err_quit("bind()");
    }

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        err_quit("listen()");
    }

    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;

    while (true) {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            continue;
        }

        bool slot_found = false;

        lock_guard<mutex> lock(socket_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (sockets[i] == INVALID_SOCKET) {
                sockets[i] = client_sock;
                thread(ProcessClient, client_sock, clientaddr, i).detach();
                slot_found = true;
                break;
            }
        }

        if (!slot_found) {
            closesocket(client_sock);
        }
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}

// 메인 함수
int main() {
    world.setUp();

    thread game_logic(run_game, ref(world));
    thread packet_processor(process_packet_thread);

    game_logic.detach();
    packet_processor.detach();

    NetworkInitialize();
    return 0;
}
