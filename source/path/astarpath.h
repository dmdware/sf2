










#ifndef ASTARPATH_H
#define ASTARPATH_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class PathJob;
class PathNode;
class Unit;
class Building;

void AStarPath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, int32_t cdtype,
               std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* targu, Building* targb,
               int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
               int32_t maxsearch,
			   int32_t nminx, int32_t nminy, int32_t nmaxx, int32_t nmaxy, bool bounded, bool capend);

void Expand_A(PathJob* pj, PathNode* node);

#endif
