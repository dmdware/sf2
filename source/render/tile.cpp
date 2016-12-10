











#include "../texture.h"
#include "tile.h"
#include "../utils.h"
#include "../sim/map.h"
#include "../render/heightmap.h"

const char* INCLINENAME[] =
{
	"0000",
	"0001",
	"0010",
	"0011",
	"0100",
	"0101",
	"0110",
	"0111",
	"1000",
	"1001",
	"1010",
	"1011",
	"1100",
	"1101",
	"1110"
};

//InType g_intype[INCLINES];
unsigned int g_ground;
unsigned int g_100sph;
unsigned int g_fog0;
unsigned int g_fog1;

void DefTl(const char* sprel)
{
#if 0
	for(int tti=0; tti<INCLINES; tti++)
	{
		InType* t = &g_intype[tti];
		//QueueTex(&t->sprite.texindex, texpath, ectrue);
		//CreateTex(t->sprite.texindex, texpath, ectrue, ecfalse);
		char specific[SFH_MAX_PATH+1];
		sprintf(specific, "%s_inc%s", sprel, INCLINENAME[tti]);
		QueueSprite(specific, &t->sprite, ecfalse, ectrue);
	}
#if 0
	t->sprite.offset[0] = spriteoffset.x;
	t->sprite.offset[1] = spriteoffset.y;
	t->sprite.offset[2] = t->sprite.offset[0] + spritesz.x;
	t->sprite.offset[3] = t->sprite.offset[1] + spritesz.y;
#endif
#endif
	if(!LoadSpriteList(sprel, &g_ground, ecfalse, ectrue, ectrue))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
}

Tile &SurfTile(int tx, int ty, Heightmap* hm)
{
	tx = imin(tx, g_mapsz.x);
	ty = imin(ty, g_mapsz.y);
	return hm->surftile[ ty * g_mapsz.x + tx ];
}
