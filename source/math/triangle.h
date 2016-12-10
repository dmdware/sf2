










#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vec3f.h"
#include "vec2f.h"

struct Vec3f;

struct Triangle
{
public:
	Vec3f vertex[3];
};

struct Triangle2 : public Triangle
{
public:
	Vec2f texcoord[3];
};

#endif
