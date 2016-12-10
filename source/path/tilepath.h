












#ifndef TILEPATH_H
#define TILEPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../platform.h"

class PathJob;
class PathNode;
class Unit;
class Building;

extern uint32_t pathnum;

#define MAX_JAM_VAL		255	//max of unsigned char
//#define MAX_JAM_VAL					1	//max of unsigned char
//#define MAX_JAM_VAL_DONTDRIVE		2	//max of unsigned char

//#define UPD_JAMS_DELAY		(SIM_FRAME_RATE*60)	//minute
//#define UPD_JAMS_DELAY		(SIM_FRAME_RATE*10)	//10 seconds
#define UPD_JAMS_DELAY			(SIM_FRAME_RATE)	//1 second

//can't be greater than (or equal to?): 255 (MAX_JAM_VAL) - biggest unit size = 5
//#define	HUMANOID_JAM	4
#define	HUMANOID_JAM	1

#define MAX_U_TPATH		64

void TilePath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, signed char cdtype, int32_t supplier,
               std::list<Vec2s> *tpath, Unit* thisu, Unit* targu, Building* targb,
               int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
               int32_t maxsearch, /* bool ignorejams=false */ bool ignorejams=true, bool verify=true);

void Expand_T(PathJob* pj, PathNode* node);

void UpdJams();
void Jamify(Unit* u);

/*
hierarchical works well when PATHNODE_SIZE is TILE_SIZE/20
(smaller units), but causes clumps around stores when TILE_SIZE/8.
TILE_SIZE/8 has less total pathnodes, so is an easier load,
possibly not as good as hierarchical though.
edit: HIERPATH causes clumps around stores in both cases.
also, check the "causes clumps" comment in MoveUnit();
*/
//#define HIERPATH	//hierarchical pathfinding?

#endif