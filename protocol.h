#pragma once

#define CS_ACTION 1
#define SC_WORLD 2

#pragma pack(push, 1)


struct CS_ACTION_PACKET {
	char type;  // CS_ACTION
	char flags; // 0x01: split, 0x02: spit
	float mx;   // [-1.0f ~ 1.0f]
	float my;   // [-1.0f ~ 1.0f]
};



struct SC_WORLD_PACKET {
	char type;  // SC_WORLD
	int object_num;
};
// 이 뒤로 오브젝트들의 배열이 붙어야 한다.


struct SC_OBJECT {
	// 타입은 필요 없다.
	float x;
	float y;
	float radius;
	COLORREF color;
};


#pragma pack(pop)
