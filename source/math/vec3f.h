










#ifndef VEC3F_H
#define VEC3F_H

#include "../platform.h"

struct Vec3f
{
	float x, y, z;
};

typedef struct Vec3f Vec3f;

void Vec3f_transform(Vec3f *v, const float *m);
void Vec3f_transform3(Vec3f *v, const float *m);

#endif
