










#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "../platform.h"
#include "3dmath.h"
#include "plane3f.h"

struct Frustum
{
public:
	void construct(const Plane3f left, const Plane3f right, const Plane3f top, const Plane3f bottom, const Plane3f front, const Plane3f back);
	void construct(const float* proj, const float* modl);
	ecbool pointin(float x, float y, float z);
	ecbool spherein(float x, float y, float z, float radius);
	ecbool cubein(float x, float y, float z, float size);
	ecbool boxin(float x, float y, float z, float x2, float y2, float z2);
	ecbool boxin2(float x, float y, float z, float x2, float y2, float z2);

	float Frustum[6][4];	// This holds the A B C and D values for each side of our frustum.
};

extern Frustum g_frustum;

#endif
