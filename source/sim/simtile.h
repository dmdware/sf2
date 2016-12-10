
#ifndef SIMTILE_H
#define SIMTILE_H

#include "../platform.h"
#include "../render/vertexarray.h"

#define INC_0000		0
#define INC_0001		1
#define INC_0010		2
#define INC_0011		3
#define INC_0100		4
#define INC_0101		5
#define INC_0110		6
#define INC_0111		7
#define INC_1000		8
#define INC_1001		9
#define INC_1010		10
#define INC_1011		11
#define INC_1100		12
#define INC_1101		13
#define INC_1110		14
#define INCLINES		15

extern VertexArray g_tileva[INCLINES];
extern int g_currincline;
extern ecbool g_cornerinc[INCLINES][4];
extern int g_tilesize;
extern float g_tilerisecm;

//#define g_tilesize		(10*100)	//10 meters = 1,000 centimeters
//extern int g_tilesize;
//#define TILE_RISE		(g_tilesize/3)
//#define TILE_DIAG		(sqrtf(g_tilesize*g_tilesize*2))
//#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/2)
//#define TILE_RISE		(tan(DEGTORAD(30))*TILE_DIAG/4)
//#define TILE_RISE		g_tilerisecm

#define TEX_DIFF		0
#define TEX_SPEC		1
#define TEX_NORM		2
#define TEX_TEAM		3
#define TEX_TYPES		4

extern unsigned int g_tiletexs[TEX_TYPES];

void DrawTile();
void MakeTiles();

#endif
