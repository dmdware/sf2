












#ifndef FILLBODIES_H
#define FILLBODIES_H

#include "pathnode.h"

void FillBodies();

class TileRegs
{
public:

	class TilePass
	{
	public:
		uint16_t from;
		uint16_t to;
	};

	std::list<TilePass> tregs[SDIRS];

	void add(unsigned char sdir, uint16_t from, uint16_t to);
};

extern TileRegs* g_tilepass;
extern unsigned char* g_tregs;

// byte-align structures
#pragma pack(push, 1)

class TileNode
{
public:
	uint16_t cost;
	uint16_t age;	//dummy
	//int16_t nx;
	//int16_t ny;
	uint16_t rund;
	//unsigned char expansion;
	TileNode* prev;
	//bool tried;
	//std::list<uint16_t> tregs;
	bool opened;
	bool closed;
	//unsigned char jams[SDIRS];
	/*
	"jams" now doesn't indicate the number of jams on the tile 
	(as an indicator of severity), but the inverse of the minimum
	size unit that can't pass. So the smaller the unit that can't
	pass, the greater the severity, and the greater the "jams" value.
	255 is thus the max unit width.
	*/
	unsigned char jams;
	//Vec2i arrivedcm;	//for lower-level tile-to-tile passing/pathfinding check

	TileNode()
	{
		//tried = false;
		prev = NULL;
		opened = false;
		closed = false;
		jams = 0;
		age=0;
	};
	TileNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, TileNode* prev, int32_t rund, int32_t stepD, uint16_t treg);
	//PathNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, PathNode* prev, int32_t rund, int32_t stepD, unsigned char expan);
};

#pragma pack(pop)

extern TileNode* g_tilenode;

inline TileNode* TileNodeAt(int32_t tx, int32_t ty)
{
	return &g_tilenode[ tx + ty * g_mapsz.x ];
}

#endif
