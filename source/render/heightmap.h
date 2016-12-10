











#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../math/vec2uc.h"
#include "../texture.h"
#include "vertexarray.h"
#include "tile.h"
#include "../math/isomath.h"

#define MAPMINZ				(0)
#define MAPMAXZ				(g_mapsz.y*TILE_SIZE)

#define MAX_MAP				63
//#define MAX_MAP					25
//#define MAX_MAP					18
//#define MAX_MAP				14
//#define MAX_MAP					11
//#define MAX_MAP				127

//todo add "age" variable to PathNode's and prefer newer added over older always for equal scores

struct Vec2f;
struct Vec3f;
struct Shader;
struct Matrix;
struct Plane3f;

extern Vec2i g_mapview[2];

/*
Number of tiles, not heightpoints/corners.
Number of height points/corners is +1.
*/
extern Vec2uc g_mapsz;

struct Heightmap
{
public:
	unsigned char *hpt;
	Vec3f *origv;
	Vec2f *texco;
	Vec3f *norm;
	int *own;
	ecbool *tricfg;
	Plane3f *tridiv;
	Tile *surftile;
	Vec2i width;

	void alloc(int wx, int wy);
	void remesh();
	void draw();

	inline unsigned char getheight(int tx, int ty)
	{
		return hpt[ (ty)*(g_mapsz.x+1) + tx ];
	}

	float accheight(float x, float y);
	float accheight2(float x, float y);
	float accheight(int x, int y);
	void adjheight(int x, int y, signed char change);
	void setheight(int x, int y, unsigned char height);
	void destroy();
	Vec3f getnormal(int x, int y);
    void lowereven();
    void highereven();

    Heightmap()
    {
        hpt = NULL;
        origv = NULL;
        texco = NULL;
        norm = NULL;
        own = NULL;
        tricfg = NULL;
        tridiv = NULL;
        surftile = NULL;
    }
	~Heightmap()
	{
		destroy();
	}
};

extern Heightmap g_hmap;

void AllocGrid(int wx, int wy);
void FreeGrid();

#endif
