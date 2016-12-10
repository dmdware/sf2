










#include "line.h"


Line::Line()
{
}

Line::Line(Vec3f start, Vec3f end)
{
	vertex[0] = start;
	vertex[1] = end;
}
