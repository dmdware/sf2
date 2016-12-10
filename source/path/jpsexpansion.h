










#ifndef JPSEXPANSION_H
#define JPSEXPANSION_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../math/vec2i.h"
#include "../platform.h"

class Unit;
class Building;
class PathNode;
class PathJob;

void Expand_JPS(PathJob* pj, PathNode* node);

#endif
