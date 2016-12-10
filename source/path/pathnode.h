










#ifndef PATHNODE_H
#define PATHNODE_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "../math/fixmath.h"
#include "../render/heightmap.h"
#include "../algo/stackpool.h"
//#include "../algo/localmap.h"
#include "../algo/bitset.h"
#include "cheaplist.h"
#include "hotqueue.h"
#include "../algo/fibheap.h"
#include "../algo/heapkey.h"
#include "../algo/hashkey.h"

//must divide tiles evenly into integers,
//so they line up.
//#define PATHNODE_SIZE	(TILE_SIZE/8)
#define PATHNODE_SIZE	(TILE_SIZE/20)
#define PATHNODE_DIAG	(isqrt(PATHNODE_SIZE*PATHNODE_SIZE*2))

//straight dirs
#define SDIR_E	0
#define SDIR_W	1
#define SDIR_S	2
#define SDIR_N	3
#define SDIRS	4

// Offsets for straights moves
const Vec2s STRAIGHTOFF[4] =
{
	Vec2s(1, 0), //E
	Vec2s(-1, 0), //W
	Vec2s(0, 1), //S
	Vec2s(0, -1) //N
};

// Offsets for diagonal moves
const Vec2s DIAGOFF[4] =
{
	Vec2s(-1, -1), //NW
	Vec2s(1, -1), //NE
	Vec2s(-1, 1), //SW
	Vec2s(1, 1) //SE
};

//TODO check for memleak because app is taking too int32_t to exit.

#define DIR_S       0
#define DIR_SW      1
#define DIR_W       2
#define DIR_NW      3
#define DIR_N       4
#define DIR_NE      5
#define DIR_E       6
#define DIR_SE      7
#define DIRS        8

#define OFFSETSX0	0
#define OFFSETSX1	-1
#define OFFSETSX2	-1
#define OFFSETSX3	-1
#define OFFSETSX4	0
#define OFFSETSX5	1
#define OFFSETSX6	1
#define OFFSETSX7	1

#define OFFSETSY0	1
#define OFFSETSY1	1
#define OFFSETSY2	0
#define OFFSETSY3	-1
#define OFFSETSY4	-1
#define OFFSETSY5	-1
#define OFFSETSY6	0
#define OFFSETSY7	1

const Vec2s OFFSETS[DIRS] =
{
	Vec2s(0, 1), //S
	Vec2s(-1, 1), //SW
	Vec2s(-1, 0), //W
	Vec2s(-1, -1), //NW
	Vec2s(0, -1), //N
	Vec2s(1, -1), //NE
	Vec2s(1, 0), //E
	Vec2s(1, 1) //SE
};

#define DIRX(i)		((i>4)-((i<4)&&(i>0)))
#define DIRY(i)		(((i<2)||(i>6))-((i>2)&&(i<6)))
//#define DIRX(i)			OFFSETS[i].x
//#define DIRY(i)			OFFSETS[i].y

//vertical dirs
#define VDIR_UP		0
#define VDIR_MID	1
#define VDIR_DOWN	2
#define VDIRS		3

const signed char VOFFSETS[VDIRS] =
{
	1, 0, -1
};

#if 0
//big error: pathnode cost and G factors count pathnodes, not centimeters
const int32_t STEPDIST[DIRS] =
{
	(int32_t)PATHNODE_DIAG, //NW
	(int32_t)PATHNODE_SIZE, //N
	(int32_t)PATHNODE_DIAG, //NE
	(int32_t)PATHNODE_SIZE, //E
	(int32_t)PATHNODE_DIAG, //SE
	(int32_t)PATHNODE_SIZE, //S
	(int32_t)PATHNODE_DIAG, //SW
	(int32_t)PATHNODE_SIZE //W
};
#else
//now rund and stepD should be multiplied by two
//(each PATHNODE_SIZE will count as two).
//multiply by 2 to give granularity
//(needed for diagonal moves).
//e.g., int32_t H = Manhattan(Vec2i(endx-nx,endz-ny)) * 2;
//must be in the same order as OFFSETS.
//corpc fix
const unsigned char STEPDIST[DIRS] =
{
	(unsigned char)2, //S
	(unsigned char)3, //SW
	(unsigned char)2, //W
	(unsigned char)3, //NW
	(unsigned char)2, //N
	(unsigned char)3, //NE
	(unsigned char)2, //E
	(unsigned char)3 //SE
};
#endif

//#define DIRDIST(i)			STEPDIST[i]
#define DIRDIST(i)		(2+(i&1))
//#define DIRDIST(i)		(2+(i%2))

// byte-align structures
#pragma pack(push, 1)

#define PATHNODE_OPENED		1
#define PATHNODE_CLOSED		2
#define PATHNODE_OPENED2	4	//backwards
#define PATHNODE_CLOSED2	8	//backwards
#define PATHNODE_BLOCKED	16

#define PATHNODE_OPENED_BIT		0
#define PATHNODE_CLOSED_BIT		1
#define PATHNODE_OPENED2_BIT	2
#define PATHNODE_CLOSED2_BIT	3
#define PATHNODE_BLOCKED_BIT	4

#define PATHNODE_FLAGBITS	5	//number of bits for all values

#define PATHNODE_FORWARD	0	//PATHNODE_OPENED<<0
#define PATHNODE_BACKWARD	2	//PATHNODE_OPENED<<2

//#define GPATHFLAGS		//store flag bits outside of PathNode's?

class PathNode
{
public:
	HeapKey heapkey;
	HashKey hashkey;
	//uint32_t cost;
	//int16_t nx;
	//int16_t ny;
	uint16_t rund;
	//uint32_t rund;
	//unsigned char expansion;
	PathNode* prev;	//bits todo
	//bool tried;
	//bool opened;
	//bool closed;
	//bool opened2;	//backwards
	//bool closed2;	//backwards
#ifndef GPATHFLAGS
	uint8_t flags;
#endif
#ifdef HASHPOOL
	int32_t index;
#endif
	PathNode()
	{
		//tried = false;
		prev = NULL;
		//opened = false;
		//closed = false;
#ifndef GPATHFLAGS
		flags = 0;
#endif
	};
	//PathNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, PathNode* prev, int32_t rund, int32_t stepD);
	//PathNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, PathNode* prev, int32_t rund, int32_t stepD, unsigned char expan);
};

#pragma pack(pop)

//return ((PathNode*)a)->cost > ((PathNode*)b)->cost;

class BinHeap;
class PathJob;

#ifdef HASHPOOL
extern StackPool g_pathmem;
//extern HashMap g_pathmap;
extern std::unordered_map<int32_t, PathNode*> g_pathmap;
#else
extern PathNode* g_pathnode;
#endif

extern StackPool g_stackpool;

extern Vec2i g_pathdim;
#ifdef CHEAPLIST
extern CheapList g_openlist[2];
#elif defined(HOTQUEUE)
extern HotQueue g_openlist[2];
#elif defined(FIBHEAP)
extern FibHeap g_openlist[2];
#else
extern BinHeap g_openlist[2];
#endif

#ifdef GPATHFLAGS
extern BitSet g_pathflags;
#endif

class Building;
class Unit;

#define PATHNODEAT(nx,ny)		(&g_pathnode[ PATHNODEINDEX(nx,ny) ])
#define PATHNODEPOS(node)		(Vec2i((node - g_pathnode) % g_pathdim.x, (node - g_pathnode) / g_pathdim.x))
//#define PATHNODEPOS	PathNodePos

Vec2i PathNodePos(PathNode* node);

//int32_t PATHNODEINDEX(int32_t nx, int32_t ny);
bool AtGoal(PathJob* pj, PathNode* node);
void SnapToNode(PathJob* pj, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded);
void SnapToNode2(PathJob* pj, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded,
				 Unit* thisu);
void ResetPathNodes();
int32_t PathOff(int32_t cmwidth);

#endif
