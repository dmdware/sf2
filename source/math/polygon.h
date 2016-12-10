










#ifndef POLYGON_H
#define POLYGON_H

#include "../platform.h"
#include "vec3f.h"

struct Polyg	//name shortened due to naming conflict on Windows
{
public:
	std::list<Vec3f> edv;	//used for constructing the polygon on-the-fly
	Vec3f* drawoutva;		//used for drawing outline

	Polyg();
	~Polyg();
	Polyg(const Polyg& original);
	Polyg& operator=(const Polyg& original);
	void makeva();
	void freeva();
};

ecbool InsidePoly(Vec3f vIntersection, Vec3f Poly[], int verticeCount);
ecbool InterPoly(Vec3f vPoly[], Vec3f l[], int verticeCount, Vec3f* vIntersection=NULL);

#endif
