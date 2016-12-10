










#ifndef RECONSTRUCTPATH_H
#define RECONSTRUCTPATH_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "pathjob.h"

class PathNode;

void ReconstructPath(std::list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int32_t cmgoalx, int32_t cmgoaly);
void ReconstructPathJPS(std::list<Vec2i> &path, PathNode* bestS, Vec2i &subgoal, int32_t cmgoalx, int32_t cmgoaly);
void ReconstructPath(PathJob* pj, PathNode* endnode);
void ReconstructPath2(PathJob* pj, PathNode* endnode, PathNode* endnode2);

#endif
