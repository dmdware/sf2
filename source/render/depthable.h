












#ifndef DEPTHABLE_H
#define DEPTHABLE_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../platform.h"

struct Dl
{
public:
	//used to do topological depth sort with other items:
	Vec3i cmmin;
	Vec3i cmmax;
	//used for picking what to send to render (pixel min/max):
	Vec2i pixmin;
	Vec2i pixmax;
	unsigned short index;
	unsigned char dtype;
	unsigned char cdtype;
	ecbool plan;
	std::list<Dl*> behind;
	ecbool visited;
	unsigned int pathnode;	//index to pathnode, with vertical/z layers, used in draw order sorting
	int rendz;	//depth for per-pixel depth writing
};

#define DEPTH_U		0	//unit
#define DEPTH_BL	1	//building
#define DEPTH_FOL	2	//foliage
#define DEPTH_CD	3	//conduit

struct Mv;
struct Bl;
struct Foliage;
struct CdTile;

//update drawabilities
void UpDraw(Mv* mv);
void UpDraw(Bl* b);
void UpDraw(Foliage* f);
void UpDraw(CdTile* c, unsigned char ctype, int tx, int ty);

#endif
