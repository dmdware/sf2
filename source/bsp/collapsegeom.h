

#ifndef COLLAPSEGEOM_H
#define COLLAPSEGEOM_H

#include "../platform.h"
#include "../math/vec3f.h"
#include "brushside.h"
#include "../math/plane3f.h"
#include "winding.h"

#define SIDE_FRONT	0
#define SIDE_BACK	1
#define SIDE_ON		2

#define ON_EPSILON	0.1f
#define MAX_BRUSH	1000000

void ClipSide(Winding *in, Plane3f *split, Winding **front, Winding **back);
void ChopBack(Winding *in, Plane3f *split, Winding** out);
void ChopFront(Winding *in, Plane3f *split, Winding** out);
void BigQuad(Winding *wind, Vec3f vmin, Vec3f vmax, BrushSide* s);
void SideCorners(Winding *wind, Vec3f vmin, Vec3f vmax, Vec3f quadaxis1, Vec3f quadaxis2, BrushSide* s);
void SideCorner(Vec3f &corner, Vec3f quadaxis, Vec3f bound);
void SideCorner3(Vec3f &corner, Plane3f* pl, Vec3f bound);
void SideCorner4(Vec3f& min1min2, Vec3f& min1max2, Vec3f& max1min2, Vec3f& max1max2, BrushSide* s, Vec3f quadaxis1, Vec3f quadaxis2);

#endif