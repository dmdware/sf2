











#ifndef TILE_H
#define TILE_H

#include "sprite.h"
#include "../phys/collidable.h"
#include "../math/vec2i.h"

//incline
struct InType
{
public:
	//unsigned int sprite;
};

#define IN_0000		0
#define IN_0001		1
#define IN_0010		2
#define IN_0011		3
#define IN_0100		4
#define IN_0101		5
#define IN_0110		6
#define IN_0111		7
#define IN_1000		8
#define IN_1001		9
#define IN_1010		10
#define IN_1011		11
#define IN_1100		12
#define IN_1101		13
#define IN_1110		14
#define INCLINES	15

extern const char* INCLINENAME[INCLINES];

//extern InType g_intype[INCLINES];
extern unsigned int g_ground;
extern unsigned int g_100sph;
extern unsigned int g_fog0;
extern unsigned int g_fog1;

struct Tile
{
public:
	unsigned char incltype;
	unsigned char elev;
};

struct Heightmap;

void DefTl(const char* sprel);
Tile &SurfTile(int tx, int ty, Heightmap* hm);

#endif
