










#ifndef COLLIDERTILE_H
#define COLLIDERTILE_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "pathnode.h"
#include "../algo/bitset.h"

//#define FLAG_HASROAD	4
#define FLAG_HASLAND	1
#define FLAG_ABRUPT		2

#define LARGEST_UNIT_NODES		(150/PATHNODE_SIZE)
#define MAX_COLLIDER_UNITS		4

//Enable this option to use map tile-sized collider tiles,
//instead of pathnode-sized.
//#define TILESIZECOLLIDER

// byte-align structures
#if 0
#pragma pack(push, 1)
class ColliderTile
{
public:
	//bool hasroad;
	//bool hasland;
	//bool haswater;
	//bool abrupt;	//abrupt incline?
	unsigned char flags;
	int16_t building;
#ifdef TILESIZECOLLIDER
	std::list<int16_t> units;
	std::list<uint16_t> foliages;
#else
	//int16_t units[MAX_COLLIDER_UNITS];
	int16_t unit;
	uint16_t foliage;
#endif

	ColliderTile();
};
#pragma pack(pop)

extern ColliderTile *g_collider;
#endif

//extern BitSet g_collider;
extern Vec2i g_pathdim;

class Unit;
class Building;
class PathJob;

//ColliderTile* ColliderAt(int32_t nx, int32_t ny);
//Vec2i PATHNODEPOS(int32_t cmposx, int32_t cmposy);
void FreePathGrid();
void AllocPathGrid(int32_t cmwx, int32_t cmwy);
void FillColliderGrid();
bool Standable(const int16_t nx, const int16_t ny, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, 
			   const Building* igb, const Unit* igu, bool roaded);
bool Standable2(const PathJob* pj, int32_t cmposx, int32_t cmposy);
bool TileStandable(const PathJob* pj, const int32_t nx, const int32_t ny);

extern  int calls;

#define PATHNODEINDEX(nx,ny)	(ny * g_pathdim.x + nx)

#if 0
inline ColliderTile* ColliderAt(int32_t nx, int32_t ny)
{
	return &g_collider[ PATHNODEINDEX(nx, ny) ];
}
#elif 0

inline int32_t ColliderAt(int32_t nx, int32_t ny)
{
	return g_collider.on(PATHNODEINDEX(nx, ny));
}
#endif

#endif
