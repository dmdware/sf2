










#ifndef PLANE2I_H
#define PLANE2I_H

#include "vec2i.h"
#include "physics.h"

struct Plane2i
{
public:
	Vec2i normal;
	int d;

	ecbool operator==(const Plane2i p) const
	{
		if(normal.x == p.normal.x && normal.y == p.normal.y && d == p.d)
			return ectrue;

		return ecfalse;
	}

	Plane2i();
	Plane2i(int x, int y, int d);
	~Plane2i();
};

int PlaneDistance(Vec2i normal, Vec2i point);

#endif
