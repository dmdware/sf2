










#ifndef PATHJOB_H
#define PATHJOB_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"

class Unit;
class Building;
class PathNode;

#define PATH_DELAY		(SIM_FRAME_RATE*15)
//#define PATH_DELAY		0

#define PATHJOB_JPS					0
#define PATHJOB_QUICKPARTIAL		1
#define PATHJOB_ASTAR				2
#define PATHJOB_JPSPART				3
#define PATHJOB_ANYPATH				4
#define PATHJOB_TILE				5
#define PATHJOB_BOUNDJPS			6
#define PATHJOB_BOUNDASTAR			7
#define PATHJOB_MAZE				8

//#define PATHHEUR MAG_VEC3F
//MAN_VEC2I, MAG_VEC2I, DIA_VEC2I
#define PATHHEUR DIA_VEC2I
#define JOBHEUR DIA_VEC2I
#define FUELHEUR DIA_VEC2I
#define TRANHEUR DIA_VEC2I	//transport heuristic

//#define HIERDEBUG	//hierarchical pathfinding debug output
//#define TSDEBUG		//truck stuck debug with hierarchical pathfinding only

#ifdef TSDEBUG
extern Unit* tracku;
#endif

//#define POWCD_DEBUG
#ifdef POWCD_DEBUG
extern std::string powcdstr;
#endif

// byte-align structures
#pragma pack(push, 1)

extern uint32_t g_pathage;

class PathJob
{
public:
	//TO DO figure out which things can be shorts/ushorts/schars
	unsigned char utype;
	unsigned char umode;
	int32_t cmstartx;
	int32_t cmstarty;
	int16_t target;
	int16_t target2;
	signed char targtype;
	signed char cdtype;
	std::list<Vec2i> *path;
	std::list<Vec2s> *tpath;
	Vec2i *subgoal;
	int16_t thisu;
	int16_t targu;
	int16_t targb;
	//int32_t cmgoalx;
	//int32_t cmgoaly;
	//might be in pathnodes,
	//might be in centimeters,
	//depending on pathjob type.
	//TO DO set goalx,z to uint16_t once i test with map below MAX_MAP width tiles
	int16_t nstartx;
	int16_t nstarty;
	int32_t goalx;
	int32_t goaly;
	int32_t goalminx;
	int32_t goalminy;
	int32_t goalmaxx;
	int32_t goalmaxy;
	bool roaded;
	bool landborne;
	bool seaborne;
	bool airborne;
	int32_t maxsearch;
	unsigned char pjtype;
	PathNode* closestnode;
	int32_t closest;
	int32_t searchdepth;
#if 0
	int32_t maxsubsearch;
	int32_t maxsubstraight;
	int32_t maxsubdiag;
	int32_t maxsubdiagstraight;
	int32_t subsearchdepth;
#endif

	//search bounds nodes
	int32_t nminx;
	int32_t nminy;
	int32_t nmaxx;
	int32_t nmaxy;

	Vec2i cmgoal;
	bool capend;	//append cmgoal to path?
	bool allowpart;	//allow incomplete (closest node) path?

	void (*callback)(bool result, PathJob* pj);

	virtual bool process();
};
#pragma pack(pop)

extern std::vector<PathNode*> g_toclear;

void Callback_UnitPath(bool result, PathJob* pj);
void CleanPath(std::vector<PathNode*> &toclear);
bool Trapped(Unit* u, Unit* targu);

#endif
