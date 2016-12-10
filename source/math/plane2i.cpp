










#include "vec3i.h"
#include "plane2i.h"
#include "physics.h"
#include "3dmath.h"
#include "../utils.h"

Plane2i::Plane2i()
{
}

Plane2i::Plane2i(int x, int y, int d)
{
	normal.x = x;
	normal.y = y;
	d = d;
}

Plane2i::~Plane2i()
{
}

int PlaneDistance(Vec2i normal, Vec2i point)
{
	int distance = 0; // This variable holds the distance from the plane to the origin

	// Use the plane equation to find the distance (Ax + By + Cz + D = 0)  We want to find D.
	// So, we come up with D = -(Ax + By + Cz)
	// Basically, the negated dot product of the normal of the plane and the point.
	distance = - (normal.x * point.x + normal.y * point.y);

	return distance;
}
