










#ifndef LINE_H
#define LINE_H

#include "vec3f.h"

struct Line
{
public:
	Vec3f vertex[2];

	Line();
	Line(Vec3f start, Vec3f end);
};

#endif
