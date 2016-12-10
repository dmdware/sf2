










/*

Convex hulls or "brushes" are how first person shooters that derive from
Quake/Doom define geometry for use in collision detection and response.
Using a set of planes, 6 for all sides of a cube, we can define any convex
shape. This simplifies collision detection against axis-aligned bounding boxes
and spheres because all we have to do is check each plane/side of the brush
against the farthest and nearest point of the AA box or sphere, and check if
both points are on both sides of the plane or inside. If there's a point on
the inside side of each plane there's an overlap with the sphere or AA box
and its not hard to get the exact distance we have to move to just be touching
the surface using the dot product.

To be drawn, brushes must be broken down into triangles. To do this I loop
through each plane/side of the brush "i". And for each side "i" I get another,
different side "j". I get the line intersect between them. This is the code
I used.

http://devmaster.net/forums/topic/8676-2-plane-intersect/

Then we need another side (each is a different side) "k" that I then get the
point intersect of the line with, and another side "l" that I get another point
intersect with. I use a for-loop to go through all the sides and for "l" I
started counting at "k+1" so we don't get any repeats (this becomes important
later when building a polygon for the brush side). The two point intersections
form a side edge for a polygon for the brush side. I store it in an array of STL
lists of lines. Each brush side has a list of lines. I store the line for side "i"
because that is the brush side that the side edge belongs to and is along.

Then I loop the side edges for each brush side, making a "polygon" - basically an
outline, with a point for each vertex. I use an epsilon value to check if two points
are within a certain distance, and use the side edge's other vertex as the next point
to check for proximity, starting over from the first side edge and making sure to
exclude checking the last connecting edge.

Then I check the polygon to be clockwise order (because that is how I cull my
polygons) by checking the normal of a triangle formed by the first three vertices
of the polygon and checking if its closer to the plane normal or if the opposite
normal is closer. If the opposite is closer I reverse the list of vertices.

Oh before I make the polygon I discard any side edges with at least one point that
is not inside or on any one plane of the brush. This is necessary to cull away
bounding planes that are outside the brush, resulting from moving the other planes.
Later I remove these planes that have less than 3 side edges, the minimum to form
a triangle.

Next I allocate (v-2) triangles where "v" is the number of vertices in the side's
polygon. I construct the triangles in a fan pattern.

There's probably some improvements that can be made like storing shared edges and
not having to reconnect them by checking distance, which I will probably learn as
I follow in the footsteps of q3map and other Quake/Doom games' source code.

[edit2] By "nearest point to the plane" I mean nearest to the "inside" side of the
plane, according to the normal. For an AA box we just check the signedness of each
axis of the normal and use the min or max on each axis to get the innermost or
outermost point of the 8 points of the AA box.

[edit3] And actually, the farthest point has to be the one from before the AA box
moved and the "nearest" point has to be from the moved position.

*/

#include "../platform.h"
#include "brush.h"
#include "plane3f.h"
#include "3dmath.h"
#include "line.h"
#include "polygon.h"
#include "../utils.h"
#include "3dmath.h"

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));	//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-radius, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(radius, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -radius), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, radius), Vec3f(0, 0, 1));	//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));	//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-hwx, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(hwx, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -hwz), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, hwz), Vec3f(0, 0, 1));	//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const Vec3f vmin, const Vec3f vmax)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, vmax.y, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, vmin.y, 0), Vec3f(0, -1, 0));	//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(vmin.x, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(vmax.x, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, vmin.z), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, vmax.z), Vec3f(0, 0, 1));	//back
}

ecbool HullsIntersect(Vec3f* hull1norms, float* hull1dist, int hull1planes, Vec3f* hull2norms, float* hull2dist, int hull2planes)
{
	return ecfalse;
}

// line intersects convex hull?
ecbool LineInterHull(const Vec3f* line, Plane3f* planes, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
	{
		Vec3f inter;
		if(LineInterPlane(line, planes[i].normal, -planes[i].d, &inter))
		{
			ecbool allin = ectrue;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(inter, planes[j]))
				{
					allin = ecfalse;
					break;
				}
			}
			if(allin)
			{
				return ectrue;
			}
		}
	}

	return ecfalse;
}

// line intersects convex hull?
ecbool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
	{
		Vec3f inter;
		if(LineInterPlane(line, norms[i], -ds[i], &inter))
		{
			ecbool allin = ectrue;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(inter, norms[j], ds[j]))
				{
					allin = ecfalse;
					break;
				}
			}
			if(allin)
			{
				return ectrue;
			}
		}
	}

	return ecfalse;
}

// line intersects convex hull?
ecbool LineInterHull(const Vec3f* line, Plane3f* planes, const int numplanes, Vec3f* intersect)
{
	for(int i=0; i<numplanes; i++)
	{
		Vec3f planeinter;
		if(LineInterPlane(line, planes[i].normal, -planes[i].d, &planeinter))
		{
			ecbool allin = ectrue;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(planeinter, planes[j]))
				{
					allin = ecfalse;
					break;
				}
			}
			if(allin)
			{
				*intersect = planeinter;
				return ectrue;
			}
		}
	}

	return ecfalse;
}

// temporary hack
ecbool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes, Vec3f* intersect)
{
	Plane3f* planes = new Plane3f[numplanes];

	for(int i=0; i < numplanes; i++)
	{
		planes[i] = Plane3f(norms[i].x, norms[i].y, norms[i].z, ds[i]);
	}

	ecbool ret = LineInterHull(line, planes, numplanes, intersect);

	delete [] planes;

	return ret;
}
