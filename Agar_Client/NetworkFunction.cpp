///////////////////////////////////////////////////////////////////////////////////////////////////////
// File name: NetworkFunction.cpp
// Created: 2045-11-09 18:09:00
// Author: 윤영민
// Description: 네트워크 함수를 정의한 소스 파일입니다.
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "NetworkFunction.h"

#include <iostream>
#include <string>

#include "../protocol.h"

//char* SERVERIP = (char*)"192.168.21.9";
char* SERVERIP = (char*)"127.0.0.1";

WSADATA wsadata;
SOCKET clientsocket;
struct sockaddr_in serveraddr;

void NetworkInitialize()
{
	int retval;

	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		err_quit("WSAStartup()");

	// 소켓 생성
	clientsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientsocket == INVALID_SOCKET)
		err_quit("socket()");

	// 서버 주소 설정
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	// 서버에 접속
	retval = connect(clientsocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

}

void NetworkFinalize()
{
	// 소켓 닫기
	closesocket(clientsocket);

	// 윈속 종료
	WSACleanup();
}


void SendData(void* buf, int size)
{
	int retval;

	// 데이터 전송
	retval = send(clientsocket, (char*)buf, size, 0);
	if (retval == SOCKET_ERROR) err_quit("send()");
}

int RecvData(char buf[]) {
    switch(recv(clientsocket, buf, sizeof(PACKET_HEADER), MSG_WAITALL)) {
        case SOCKET_ERROR:
            err_quit("[client1] recv()");
            return -1;
        case 0:
            return 0;
		default:
            PACKET_HEADER* header = (PACKET_HEADER*)buf;
            unsigned short int size = header->size;
			switch(recv(clientsocket, buf + sizeof(PACKET_HEADER), size - sizeof(PACKET_HEADER), MSG_WAITALL)) {
				case SOCKET_ERROR:
					err_quit("[client2] recv()");
					return -1;
				case 0:
					return 0;
				default:
					return size;
			}
    }
}

PACKET_HEADER* RecvPacket() {
	PACKET_HEADER header;
	switch(recv(clientsocket, (char*)&header, sizeof(PACKET_HEADER), MSG_WAITALL)) {
		case SOCKET_ERROR:
			err_quit("[client1] recv()");
			return nullptr;
		case 0:
			return nullptr;
	}

	switch(header.type) {
		case SC_INIT: {
            SC_INIT_PACKET* packet = new SC_INIT_PACKET;
            packet->type = SC_INIT;
            packet->size = header.size;
			switch(recv(clientsocket, (char*)&packet->id, sizeof(int), MSG_WAITALL)) {
                case SOCKET_ERROR:
                    err_quit("[client2] recv()");
                    return nullptr;
                case 0:
                    return nullptr;
                default:
                    return packet;
			}
		}

        case SC_WORLD: {
            SC_WORLD_PACKET* packet = new SC_WORLD_PACKET;
            packet->type = SC_WORLD;
			packet->size = header.size;
            switch(recv(clientsocket, (char*)&packet->object_num, sizeof(int), MSG_WAITALL)) {
                case SOCKET_ERROR:
                    err_quit("[client2] recv()");
                    return nullptr;
                case 0:
                    return nullptr;
                default:
                    packet->objects.resize(packet->object_num);
					switch(recv(clientsocket, (char*)packet->objects.data(), packet->object_num * sizeof(SC_OBJECT), MSG_WAITALL)) {
						case SOCKET_ERROR:
							err_quit("[client3] recv()");
							return nullptr;
						case 0:
							return nullptr;
					}
                    return packet;
            }
        }
	}
}
