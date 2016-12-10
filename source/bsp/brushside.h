


#ifndef BRUSHSIDE_H
#define BRUSHSIDE_H

#include "../math/vec3f.h"
#include "../math/plane3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../render/model2.h"
#include "../render/vertexarray.h"

//tex coord equation - not used
struct TexCEq
{
	float rot;	//degrees
	float scale[2];	//default 0.1, world to tex coordinates
	float offset[2];	//in world distances
};

struct CutBrushSide;

struct BrushSide
{
public:
	Plane3f plane;
	VertexArray drawva;
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;
	unsigned int ownerm;	//team colour map
	int ntris;
	Triangle2* tris;
	Plane3f tceq[2];	//tex coord uv equations
	Polyg outline;
	int* vindices;	//indices into parent brush's shared vertex array; only stores unique vertices as defined by polygon outline
	Vec3f centroid;
	std::list<Vec3f> sideverts;
	
	BrushSide(const BrushSide& original);
	BrushSide& operator=(const BrushSide &original);
	BrushSide();
	BrushSide(Vec3f normal, Vec3f point);
	~BrushSide();
	void usedifftex();
	void usespectex();
	void usenormtex();
	void useteamtex();
	void makeva();
	void vafromcut(CutBrushSide* cutside);
	void usetex();
	void gentexeq();	//fit texture to face
	void fittex();
	void remaptex();
};

Vec3f PlaneCrossAxis(Vec3f normal);

#endif