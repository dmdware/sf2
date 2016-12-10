










#ifndef PLANE3F_H
#define PLANE3F_H

#include "vec3f.h"
#include "physics.h"

struct Plane3f
{
public:
	Vec3f normal;
	float d;

	ecbool operator==(const Plane3f p) const
	{
		//if(fabs(normal.x - p.normal.x) <= EPSILON && fabs(normal.y - p.normal.y) <= EPSILON && fabs(normal.z - p.normal.z) <= EPSILON && fabs(d - p.d) <= EPSILON)
		//	return ectrue;

		if(normal.x == p.normal.x && normal.y == p.normal.y && normal.z == p.normal.z && d == p.d)
			return ectrue;

		return ecfalse;
	}

	Plane3f();
	Plane3f(float x, float y, float z, float d);
	~Plane3f();
};

ecbool Close(Plane3f a, Plane3f b);
Vec3f PointOnPlane(Plane3f p);
float PlaneDistance(Vec3f normal, Vec3f point);
ecbool PointBehindPlane(Vec3f point, Plane3f plane);
ecbool PointOnOrBehindPlane(Vec3f point, Plane3f plane, float epsilon=CLOSE_EPSILON);
ecbool PointOnOrBehindPlane(Vec3f point, Vec3f normal, float dist, float epsilon=CLOSE_EPSILON);
void RotatePlane(Plane3f& p, Vec3f about, float radians, Vec3f axis);
void MakePlane(Vec3f* norm, float* d, Vec3f point, Vec3f setnorm);
void ParamLine(Vec3f* line, Vec3f* change);
ecbool LineInterPlane(const Vec3f* line, const Vec3f norm, const float d, Vec3f* inter);

#endif
