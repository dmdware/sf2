#ifndef COLLISION_H
#define COLLISION_H

#include "../platform.h"

#define MAX_CLIMB_INCLINE		(TILE_SIZE)

#define COLLIDER_NONE		-1
#define COLLIDER_UNIT		0
#define COLLIDER_BUILDING	1
#define COLLIDER_TERRAIN	2
#define COLLIDER_NOROAD		3
#define COLLIDER_OTHER		4
#define COLLIDER_NOLAND		5
#define COLLIDER_NOSEA		6
#define COLLIDER_NOCOAST	7
#define COLLIDER_ROAD		8
#define COLLIDER_OFFMAP		9

extern int g_lastcollider;
extern int g_collidertype;
extern ecbool g_ignored;

struct Mv;

ecbool BlAdj(int i, int j);
ecbool CoAdj(char ctype, int i, int x, int z);
ecbool CollidesWithTerr(int cmminx, int cmminy, int cmmaxx, int cmmaxy);
ecbool CollidesWithUnits(int minx, int miny, int maxx, int maxy, ecbool isunit, Mv* thisu, Mv* ignore);
ecbool OffMap(int cmminx, int cmminy, int cmmaxx, int cmmaxy);
ecbool CollidesWithBuildings(int minx, int miny, int maxx, int maxy, int ignore=-1);

#endif
