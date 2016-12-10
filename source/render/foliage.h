











#ifndef FOLIAGE_H
#define FOLIAGE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../math/vec2s.h"
#include "../math/vec3i.h"
#include "../math/matrix.h"
#include "vertexarray.h"
#include "sprite.h"
#include "depthable.h"

struct FlType
{
public:
	char name[64];
	Vec2s size;
	//unsigned int sprite;
	unsigned int splist;
};


#define FL_SPRUCE1			0
#define FL_SPRUCE2			1
#define FL_SPRUCE3			2
#define FL_EUHB1			3
#define FL_TYPES			3

extern FlType g_fltype[FL_TYPES];

// byte-align structures
#pragma pack(push, 1)
struct Foliage
{
public:
	Dl* depth;
	ecbool on;
	unsigned char type;
	Vec2i cmpos;
	Vec3f drawpos;
	float yaw;
	unsigned char lastdraw;	//used for preventing repeats

	Foliage();
	void fillcollider();
	void freecollider();
	void destroy();
};
#pragma pack(pop)

//#define FOLIAGES	128
//#define FOLIAGES	1024
//#define FOLIAGES	2048
#define FOLIAGES	6000
//#define FOLIAGES	10000
//#define FOLIAGES	30000
//#define FOLIAGES	60000	//ushort limit
//#define FOLIAGES	240000


//__declspec(dllexport) asd();

extern Foliage g_fl[FOLIAGES];

void DefF(int type, const char* sprel, /* Vec3f scale, Vec3f translate,*/ Vec2s size);
ecbool PlaceFol(int type, Vec3i cmpos);
void DrawFol(Foliage* f, float rendz, unsigned int renderdepthtex, unsigned int renderfb);
void ClearFol(int cmminx, int cmminy, int cmmaxx, int cmmaxy);
void FreeFol();
void FillForest(unsigned int r);
#endif
