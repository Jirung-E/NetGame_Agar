#pragma once

#define CS_ACTION 1


#pragma pack(push, 1)
struct CS_ACTION_PACKET {
	char type;  // CS_ACTION
	char flags; // 0x01: split, 0x02: spit
	float mx;   // [-1.0f ~ 1.0f]
	float my;   // [-1.0f ~ 1.0f]
};
#pragma pack(push, 1)

struct SC_WORLD_PACKET {

};