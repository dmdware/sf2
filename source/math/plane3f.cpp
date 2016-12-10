










#include "vec3f.h"
#include "plane3f.h"
#include "physics.h"
#include "3dmath.h"
#include "../utils.h"

Plane3f::Plane3f()
{
}

Plane3f::Plane3f(float x, float y, float z, float d)
{
	normal.x = x;
	normal.y = y;
	normal.z = z;
	d = d;
}

Plane3f::~Plane3f()
{
}

ecbool Close(Plane3f a, Plane3f b)
{
	if(fabs(a.normal.x - b.normal.x) <= CLOSE_EPSILON && fabs(a.normal.y - b.normal.y) <= CLOSE_EPSILON && fabs(a.normal.z - b.normal.z) <= CLOSE_EPSILON && fabs(a.d - b.d) <= CLOSE_EPSILON)
		return ectrue;

	if(fabs(-a.normal.x - b.normal.x) <= CLOSE_EPSILON && fabs(-a.normal.y - b.normal.y) <= CLOSE_EPSILON && fabs(-a.normal.z - b.normal.z) <= CLOSE_EPSILON && fabs(-a.d - b.d) <= CLOSE_EPSILON)
		return ectrue;

	return ecfalse;
}

//#define PLANE_DEBUG

Vec3f PointOnPlane(Plane3f p)
{
	//Ax + By + Cz + D = 0
	//x = -D/A	if(A != 0)
	//y = -D/B	if(B != 0)
	//z = -D/C	if(C != 0)

#ifdef PLANE_DEBUG
	Log("point on plane ("<<p.normal.x<<","<<p.normal.y<<","<<p.normal.z<<"),"<<p.d);
	
#endif

	int greatest = -1;
	float greatestd = 0;

	if(greatest < 0 || fabs(p.normal.x) > greatestd)
	{
		greatest = 0;
		greatestd = fabs(p.normal.x);
	}
	if(greatest < 0 || fabs(p.normal.y) > greatestd)
	{
		greatest = 1;
		greatestd = fabs(p.normal.y);
	}
	if(greatest < 0 || fabs(p.normal.x) > greatestd)
	{
		greatest = 2;
		greatestd = fabs(p.normal.z);
	}

	if(fabs(p.normal.x) > EPSILON)
		//if(greatest == 0)
		return Vec3f(- p.d / p.normal.x, 0, 0);

	if(fabs(p.normal.y) > EPSILON)
		//if(greatest == 1)
		return Vec3f(0, - p.d / p.normal.y, 0);

	if(fabs(p.normal.z) > EPSILON)
		//if(greatest == 2)
		return Vec3f(0, 0, - p.d / p.normal.z);

	return Vec3f(0, 0, 0);
}

float PlaneDistance(Vec3f normal, Vec3f point)
{
	float distance = 0; // This variable holds the distance from the plane to the origin

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// So, we come up with D = -(Ax + By + Cz)
	// Basically, the negated dot product of the normal of the plane and the point.
	distance = - (normal.x * point.x + normal.y * point.y + normal.z * point.z);

	return distance;
}

ecbool PointBehindPlane(Vec3f point, Plane3f plane)
{
	float result = point.x*plane.normal.x + point.y*plane.normal.y + point.z*plane.normal.z + plane.d;

	if(result <= 0)
		return ectrue;

	return ecfalse;
}

ecbool PointOnOrBehindPlane(Vec3f point, Plane3f plane, float epsilon)
{
	float result = point.x*plane.normal.x + point.y*plane.normal.y + point.z*plane.normal.z + plane.d;

	if(result <= epsilon)
		return ectrue;

	return ecfalse;
}

ecbool PointOnOrBehindPlane(Vec3f point, Vec3f normal, float dist, float epsilon)
{
	float result = point.x*normal.x + point.y*normal.y + point.z*normal.z + dist;

	if(result <= epsilon)
		return ectrue;

	return ecfalse;
}

void RotatePlane(Plane3f& p, Vec3f about, float radians, Vec3f axis)
{
	Vec3f pop = PointOnPlane(p);
	pop = RotateAround(pop, about, radians, axis.x, axis.y, axis.z);
	p.normal = Rotate(p.normal, radians, axis.x, axis.y, axis.z);
	p.d = PlaneDistance(p.normal, pop);
}

// http://thejuniverse.org/PUBLIC/LinearAlgebra/LOLA/planes/std.html
void MakePlane(Vec3f* norm, float* d, Vec3f point, Vec3f setnorm)
{
	*norm = setnorm;
	*norm = Normalize( *norm );
	*d = -Dot(setnorm, point);
}

// Parametric line
void ParamLine(Vec3f* line, Vec3f* change)
{
	(*change) = line[1] - line[0];
	//(*change) = Normalize(*change);

}

// line intersects plane?
ecbool LineInterPlane(const Vec3f* line, const Vec3f norm, const float d, Vec3f* inter)
{
	Vec3f change = line[1] - line[0];
	//ParamLine(line, &change);

	float denom = Dot(norm, change);

	if(fabs(denom) <= EPSILON)
		return ecfalse;

	float SegScalar = (d - Dot(norm, line[0])) / denom;

	//TODO: Check if SegScalar is [0.0, 1.0]?
	if(SegScalar < 0.0f)
		return ecfalse;

	*inter = change * SegScalar + line[0];

	return ectrue;
}
