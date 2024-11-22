#pragma once

#define CS_ACTION 1
#define SC_WORLD 2
#define SC_INIT 3

#pragma pack(push, 1)


struct PACKET_HEADER {
	char type;
	unsigned short int size;		// 패킷 헤더 + 데이터 크기
};


struct SC_INIT_PACKET {
    PACKET_HEADER header;
    int id;
};


struct CS_ACTION_PACKET {
	PACKET_HEADER header;
	char flags; // 0x01: split, 0x02: spit
	double mx;   // [-1.0f ~ 1.0f]
	double my;   // [-1.0f ~ 1.0f]
};



struct SC_WORLD_PACKET {
	PACKET_HEADER header;
	int object_num;
};
// 이 뒤로 오브젝트들의 배열이 붙어야 한다.


struct SC_OBJECT {
	// 타입은 필요 없다.
	uint8_t id;
	double x;
	double y;
	double radius;
	COLORREF color;
};


#pragma pack(pop)
