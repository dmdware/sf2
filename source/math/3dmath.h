










#ifndef MATH3D_H
#define MATH3D_H

#include "../platform.h"
#include "vec2f.h"
#include "vec3f.h"
#include "vec2i.h"
#include "vec3i.h"
#include "fixmath.h"
#include "../utils.h"
#include "../algo/ecbool.h"

#define DEGTORAD(a)		(M_PI * a / 180.0f)
#define RADTODEG(a)		(180.0f * a / M_PI)

#ifndef PI_ON_180
#	define PI_ON_180	(M_PI/180.0f)
#endif

#define CAMERA_SPEED	121

struct Plane3f;
struct Vec3f;
struct Camera;
struct Matrix;
struct Triangle;
struct Vec4f;
struct Vec2i;
struct Vec3i;
struct Vec2f;

inline float FastSqrtInvAroundOne(float x)
{
	return (15.0f / 8.0f) + (-5.0f / 4.0f) * x + (3.0f / 8.0f) * x * x;
}
 
Vec3f Normalize(Vec3f* v)
{
	float len = v->x * v->x + v->y * v->y + v->z * v->z; /* ln sq */
	len = FastSqrtInvAroundOne(len); /* ln inv */
	v->x *= len;
	v->y *= len;
	v->z *= len;
}
/* custom calc trig fun's deterministic todo */

#define MAG_VEC3F(vec)		((float)sqrtf( ((vec).x * (vec).x) + ((vec).y * (vec).y) + ((vec).z * (vec).z) ))
#define MAG_VEC2F(vec)		((float)sqrtf( ((vec).x * (vec).x) + ((vec).y * (vec).y) ))
#define DOT_VEC3F(vec)		(((vec).x * (vec).x) + ((vec).y * (vec).y) + ((vec).z * (vec).z))
#define DOT_VEC2F(vec)		(((vec).x * (vec).x) + ((vec).y * (vec).y))
#define MAG_VEC2I(vec)		(isqrt( (vec).x*(vec).x + (vec).y*(vec).y ))	/*MUST BE DETERMINISTIC!!!!*/
#define DOT_VEC2I(vec)		((vec).x*(vec).x + (vec).y*(vec).y)
#define MAN_VEC2I(vec)		(iabs((vec).x) + iabs((vec).y))	/*Manhattan distance*/

inline int Diag(int x, int y)
{
	int ax = iabs(x);
	int ay = iabs(y);
	return ((((ax+ay)<<1) + (3 - 4) * imin(ax, ay))>>1);
}

/* Diagonal distance */
/* #define DIA_VEC2I(vec)		((((iabs(vec.x)+iabs(vec.y))<<1) + (3 - 4) * imin(iabs(vec.x), iabs(vec.y)))>>1) */
#define DIA_VEC2I(vec)			Diag(vec.x,vec.y)
/* #define DIA_VEC2I(vec)		((((abs(vec.x)+abs(vec.y))<<1) + (3 - 4) * std::min(abs(vec.x), abs(vec.y)))>>1) */



#endif
