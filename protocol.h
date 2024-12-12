#pragma once

#include <vector>

#define CS_ACTION 1
#define CS_EXIT 2
#define CS_RESPAWN 3
#define CS_LOGIN 4

#define SC_WORLD 1
#define SC_INIT 2

#pragma pack(push, 1)


struct PACKET_HEADER {
	char type;
	unsigned short int size;		// 패킷 헤더 + 데이터 크기
};


struct SC_INIT_PACKET : public PACKET_HEADER {
    int id;
};


struct CS_ACTION_PACKET {
	PACKET_HEADER header;
	char flags; // 0x01: split, 0x02: spit
	double mx;   // [-1.0f ~ 1.0f]
	double my;   // [-1.0f ~ 1.0f]
};

struct CS_EXIT_PACKET {
	PACKET_HEADER header;  // CS_EXIT
};

struct CS_RESPAWN_PACKET {
	PACKET_HEADER header;  // CS_RESPAWN
};

struct CS_LOGIN_PACKET {
	PACKET_HEADER header;  // CS_LOGIN
	char name[16];
};

struct SC_OBJECT {
	// 타입은 필요 없다.
	uint8_t id;
	double x;
	double y;
	double radius;
	COLORREF color;
};

struct SC_WORLD_PACKET : public PACKET_HEADER {
	int object_num;
	std::vector<SC_OBJECT> objects;

public:
	std::vector<char> serialize() {
		std::vector<char> buffer(sizeof(PACKET_HEADER) + sizeof(int) + objects.size() * sizeof(SC_OBJECT));
		PACKET_HEADER* header = (PACKET_HEADER*)&buffer[0];
		header->type = SC_WORLD;
		header->size = buffer.size();
		*(int*)&buffer[sizeof(PACKET_HEADER)] = object_num;
		memcpy(&buffer[sizeof(PACKET_HEADER) + sizeof(int)], objects.data(), objects.size() * sizeof(SC_OBJECT));

		return buffer;
	}
};
/*
struct LOGIN_PACKET {
	uint8_t id;
	Player playerinfo;

	//기본형? 미리 만들어둠 수정필요.

};
*/
#pragma pack(pop)
