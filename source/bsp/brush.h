

#ifndef BRUSH_H
#define BRUSH_H

#include "../math/vec3f.h"
#include "../math/plane3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "brushside.h"
#include "../bsp/door.h"

#define STOREY_HEIGHT	250.0f //20.0f

struct Brush
{
public:
	int nsides;
	BrushSide* sides;
	int nsharedv;
	Vec3f* sharedv;	//shared vertices array
	int texture;	//used to determine brush attributes
	EdDoor* door;
	ecbool broken;
	
	Brush& operator=(const Brush& original);
	Brush(const Brush& original);
	Brush();
	~Brush();
	void add(BrushSide b);
	void setsides(int nsides, BrushSide* sides);
	void getsides(int* nsides, BrushSide** sides);
	void removeside(int i);
	void collapse();
	void collapse2();
	void colshv();
	void colva();
	void coloutl();
	void remaptex();
	Vec3f traceray(Vec3f line[]);
	void prunev(ecbool* invalidv);
	void moveto(Vec3f newp);
	void trysimp();
};

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height);
void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height);
void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const Vec3f vmin, const Vec3f vmax);
ecbool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes);

#endif