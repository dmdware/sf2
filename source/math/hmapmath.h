










#ifndef HMAPMATH_H
#define HMAPMATH_H

#include "../math/vec3f.h"
#include "../math/vec2uc.h"

struct Heightmap;

float Bilerp(Heightmap* hmap, float x, float y);
ecbool MapInter(Heightmap* hmap, Vec3f ray, Vec3f point, Vec3i* cmint);
ecbool FastMapIntersect(Heightmap* hmap, Vec2uc sz, Vec3f* line, Vec3f* intersect, ecbool flipyz=ectrue);
ecbool MapBoundsIntersect(Vec2uc sz, Vec3f* line, Vec3f* intersect, ecbool flipyz=ectrue);

#endif
