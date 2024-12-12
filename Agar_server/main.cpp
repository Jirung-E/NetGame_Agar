#pragma warning(disable:4996)

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

// 상수 및 전역 변수 정의
const int SERVERPORT = 9000;
const int PACKETSIZEMAX = 512;
const int MAX_CLIENTS = 8;

mutex socket_mutex;
SOCKET sockets[MAX_CLIENTS];

const uint8_t TRAP_ID = 100;
const uint8_t FEED_ID = 200;

World world;



HANDLE hProcessPacket;

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
    int update_time = fps(60);
    bool send_limit_flag = false;

    while (true) {
        elapsed += clock() - timer;
        timer = clock();
        if (elapsed < update_time) {
            continue;
        }

        elapsed -= update_time;


        // 패킷 처리 이벤트 false
        ResetEvent(hProcessPacket);

        world.update(update_time);

        // 패킷 처리 이벤트 true
		SetEvent(hProcessPacket);



        if(!send_limit_flag) {
            auto players = world.getPlayers();
            auto traps = world.getTraps();
            auto feeds = world.getFeeds();

            SC_WORLD_PACKET packet;
            packet.player_num = 0;
            packet.object_num = 0;

            for(const auto& p : players) {
                auto player = p.second;

                if(player.cells.empty()) {
                    continue;
                }

                SC_PLAYER_PROFILE profile;
                profile.id = player.id;
                strcpy(profile.name, player.getName().c_str());

                packet.players.push_back(profile);
                packet.player_num++;

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

            auto data = packet.serialize();
            send_packet(INVALID_SOCKET, data.data(), packet.size);
        }
        
        send_limit_flag = !send_limit_flag;
    }
}


thread_local std::string player_name;

void ProcessPacket(int id, char* buf) {
    char packetType = buf[0];

    switch (packetType) {
    case CS_LOGIN: {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(buf);
        world.addPlayer(id);
        player_name = p->name;
        world.setPlayerName(id, player_name);
        break;
    }
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
    case CS_RESPAWN: {
        //리스폰처리
        world.addPlayer(id);
        world.setPlayerName(id, player_name);
        break;
    }
    case CS_EXIT: {
        // closesocket -> ProcessClient에서 처리
        // world에서 제거 -> ProcessClient에서 처리
        break;
    }
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

    while (true) {
        retval = recv(socket, buf, PACKETSIZEMAX, 0);
        if (retval == SOCKET_ERROR || retval == 0) {
            break;
        }

		WaitForSingleObject(hProcessPacket, INFINITE);

        // 패킷 처리 직접 수행
        ProcessPacket(id, buf);
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

    {
        char hostName[256];

        // 호스트 이름 가져오기
        if(gethostname(hostName, sizeof(hostName)) != 0) {
            std::cerr << "gethostname failed." << std::endl;
            WSACleanup();
            return 1;
        }

        // 호스트 엔트리 가져오기
        struct hostent* hostEntry = gethostbyname(hostName);
        if(hostEntry == nullptr) {
            std::cerr << "gethostbyname failed." << std::endl;
            WSACleanup();
            return 1;
        }

        // 첫 번째 IP 주소 출력
        struct in_addr addr;
        addr.s_addr = *(u_long*)hostEntry->h_addr_list[0];
        std::cout << "Local IP Address - " << inet_ntoa(addr) << ":" << SERVERPORT << std::endl;
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

	hProcessPacket = CreateEvent(NULL, FALSE, TRUE, NULL);

    thread game_logic(run_game, ref(world));

    game_logic.detach();

    NetworkInitialize();
    return 0;
}
