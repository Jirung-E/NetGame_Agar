#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

#include "Common.h"
#include "Game/World.h"
#include "../protocol.h"
#include "Util.h"

using namespace std;
using namespace chrono;


const int SERVERPORT = 9000;
const int PACKETSIZEMAX = 512;

const int MAX_CLIENTS = 8;
atomic_bool slot[MAX_CLIENTS];

World world;


void run_game(World& world) {
    auto timer = clock();
    auto elapsed = 0;
    int update_time = fps(60);

    // 고정프레임 업데이트
    while(true) {
        elapsed += clock() - timer;
        timer = clock();
        if(elapsed < update_time) {
            continue;
        }

        elapsed -= update_time;

        world.update();
    }
}


void ProcessClient(SOCKET socket, struct sockaddr_in clientaddr, int id) {
    int retval;
    char buf[PACKETSIZEMAX];

    // 접속한 클라이언트 정보 출력
    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
    printf("%s:%d\n", addr, ntohs(clientaddr.sin_port));

    world.addPlayer(id);
    cout << "Players: \n";
    for(const auto& p : world.getPlayers()) {
        printf(" - id: %2d\n", p.first);
    }

    // 클라이언트와 데이터 통신
    while(true) {
        // 데이터 받기
        retval = recv(socket, buf, PACKETSIZEMAX, 0);
        if(retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if(retval == 0) {		// 연결 종료
            break;
        }

        struct Packet { // protocol.h 의 CS_ACTION_PACKET 으로 들어오는 정보일 예정
            LONG x;
            LONG y;
        }* p;
        p = reinterpret_cast<Packet*>(buf);
        cout << p->x << ", " << p->y << endl;

        // 데이터 보내기
        retval = send(socket, buf, retval, 0);
        if(retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        //player data update
        world.setPlayerDestination(id, p->x, p->y);

    }

    world.removePlayer(id);
    cout << "Players: \n";
    for(const auto& p : world.getPlayers()) {
        printf(" - id: %2d\n", p.first);
    }

    // 통신 소켓 닫기
    closesocket(socket);

    slot[id] = false;
}

int NetworkInitialize() {
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
        while (wait) {
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                // 슬롯이 비어있으면 다운로드 시작
                if (slot[i] == false) {
                    slot[i] = true;
                    std::thread new_client_thread{ProcessClient, client_sock, clientaddr, i };
                    new_client_thread.detach();

                    wait = false;
                    break;
                }
            }
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
