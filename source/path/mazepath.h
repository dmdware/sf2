










#ifndef MAZEPATH_H
#define MAZEPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class Unit;
class Building;
class PathJob;
class PathNode;

char AngleDir(int dx, int dy);
#if 01
void MazePath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, int32_t cdtype, int32_t supplier,
                 std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* targu, Building* targb,
                 int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
                 int32_t maxsearch, bool capend, bool allowpart, bool adjgoalbounds=true);
#endif
bool Expand_MP(PathJob* pj, PathNode* node, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded, uint8_t dir);

#endif
