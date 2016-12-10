﻿











#include "fillbodies.h"
#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../algo/binheap.h"
#include "anypath.h"
#include "reconstructpath.h"
#include "pathdebug.h"

TileRegs* g_tilepass = NULL;
unsigned char* g_tregs = NULL;
TileNode* g_tilenode = NULL;

void TileRegs::add(unsigned char sdir, uint16_t from, uint16_t to)
{
	for(std::list<TilePass>::iterator rit=tregs[sdir].begin(); rit!=tregs[sdir].end(); rit++)
	{
		if(rit->from == from && rit->to == to)
			return;
	}

	TileRegs::TilePass pass = {from,to};
	tregs[sdir].push_back(pass);
}

bool BodyStandable(const int32_t nx, const int32_t ny, const uint16_t cmsize)
{
	return false;

#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

#if 0
	ColliderTile* cell = ColliderAt( nx, ny );

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("stand? u"<<pj->thisu<<" n"<<nx<<","<<ny<<" dn("<<(nx-pj->goalx)<<","<<(ny-pj->goaly)<<")");

		char add[1280];
		sprintf(add, "std? u%d n%d,%d dn%d,%d \r\n", (int32_t)pj->thisu, (int32_t)nx, (int32_t)ny, (int32_t)(nx-pj->goalx), (int32_t)(ny-pj->goaly));
		powcdstr += add;
	}
#endif

	if(cell->flags & FLAG_ABRUPT)
		return false;

	const int32_t cmposx = nx * PATHNODE_SIZE;
	const int32_t cmposy = ny * PATHNODE_SIZE;

	const int32_t cmminx = cmposx - cmsize/2;
	const int32_t cmminy = cmposy - cmsize/2;
	const int32_t cmmaxx = cmminx + cmsize - 1;
	const int32_t cmmaxy = cmminy + cmsize - 1;

	const int32_t nminx = cmminx/PATHNODE_SIZE;
	const int32_t nminy = cmminy/PATHNODE_SIZE;
	const int32_t nmaxx = cmmaxx/PATHNODE_SIZE;
	const int32_t nmaxy = cmmaxy/PATHNODE_SIZE;

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

#if 0
	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
#endif

	for(int32_t y=nminy; y<=nmaxy; y++)
		for(int32_t x=nminx; x<=nmaxx; x++)
		{
			cell = ColliderAt(x, y);

			if(cell->foliage != USHRT_MAX)
				//collided = true;
				return false;

#if 0
			if(cell->building >= 0)
			{
				collided = true;
			}
#endif
		}

#if 0
	if(collided && !ignoredu && !ignoredb)
		return false;
#endif

	return true;
#endif
}

void FillBodies()
{
	return;
	//Log("fillb");

	//TODO use bitfield?
	//Need to determine how many bits needed for max possible regions in node.
	//tile regions

	for(int32_t i=0; i<g_pathdim.x*g_pathdim.y; i++)
		g_tregs[i] = UCHAR_MAX;

	for(int32_t i=0; i<g_mapsz.x*g_mapsz.y; i++)
	{
		g_tilepass[i].tregs[0].clear();
		g_tilepass[i].tregs[1].clear();
		g_tilepass[i].tregs[2].clear();
		g_tilepass[i].tregs[3].clear();
	}

#define IN_PATHNODES	(TILE_SIZE / PATHNODE_SIZE)

	//Mark flood-fill regions for each tile.
	//This must be done on a tile-by-tile basis
	//(the tile-to-tile region passability is determined next).
	for(unsigned char ty=0; ty<g_mapsz.y; ty++)
		for(unsigned char tx=0; tx<g_mapsz.x; tx++)
		{
			unsigned char curbody = 0;

			//For each pathnode in this tile,
			//check if it is standable for land or water.
			//Ignore buildings.
			//There must be LESS than UCHAR_MAX pathnodes in a tile.
			//UCHAR_MAX means impassable or not checked.
			//If there's 20x20 pathnodes in a tile I should use ushorts.
			for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
				for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
				{
					//node index
					int32_t nin = nx + ny * g_pathdim.x;

					if(!BodyStandable(nx, ny, PATHNODE_SIZE))
						continue;

					g_tregs[nin] = curbody;
					curbody++;
				}

			bool change;

			//Merge passable regions.
			do
			{
				change = false;

				for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
					for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
					{
						//node index
						int32_t nin = nx + ny * g_pathdim.x;

						if(g_tregs[nin] == UCHAR_MAX)
							continue;

						//is there a pathnode in this tile at [nx+1]?
						if(nx < (tx+1)*IN_PATHNODES - 1)
						{
							int32_t nin2 = (nx+1) + ny * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [nx-1]?
						if(nx > tx*IN_PATHNODES)
						{
							int32_t nin2 = (nx-1) + ny * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [ny+1]?
						if(ny < (ty+1)*IN_PATHNODES - 1)
						{
							int32_t nin2 = nx + (ny+1) * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}

						//is there a pathnode in this tile at [ny-1]?
						if(ny > ty*IN_PATHNODES)
						{
							int32_t nin2 = nx + (ny-1) * g_pathdim.x;

							if(g_tregs[nin2] != UCHAR_MAX && g_tregs[nin2] > g_tregs[nin])
							{
								g_tregs[nin2] = g_tregs[nin];
								change = true;
							}
						}
					}
			}while(change);
		}

	//Now determine passability between regions
	//in neighbouring tiles.
	//TODO
	for(unsigned char ty=0; ty<g_mapsz.y; ty++)
		for(unsigned char tx=0; tx<g_mapsz.x; tx++)
		{
			//Go along each side.
			//По каждой стороне проходим.
#if 0
			for(unsigned char sdir=0; sdir<SDIRS; sdir++)
			{
				Vec2s off = STRAIGHTOFF[sdir];
				//pathnode coords
				unsigned char n[2];
				//component
				for(unsigned char c=0; c<2; c++)
				{
					if(off[c] != 0)
						continue;

					unsigned char c2 = (c+1)%2;

					//for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
					//for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
					{
						uint16_t nx = tx*IN_PATHNODES;

						//node index
						int32_t nin = nx + ny * g_pathdim.x;
						int32_t nin2 = (nx-1) + ny * g_pathdim.x;

						if(g_tregs[nin] == UCHAR_MAX)
							continue;

						if(g_tregs[nin2] == UCHAR_MAX)
							continue;

						//tilepass index
						int32_t tpin = tx + ty * g_mapsz.x;
						g_tilepass[tpin].add(sidr, g_tregs[nin2]);
					}
				}
			}
#else
			//Go along left side.
			if(tx > 0)
				for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
				//for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
				{
					uint16_t nx = tx*IN_PATHNODES;
					//uint16_t ny = (ty-off.y)*IN_PATHNODES;

					//node index
					int32_t nin = nx + ny * g_pathdim.x;
					int32_t nin2 = (nx-1) + ny * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int32_t tpin = tx + ty * g_mapsz.x;
					g_tilepass[tpin].add(SDIR_W, g_tregs[nin], g_tregs[nin2]);

					//Log("add1");
				}

			//Go along right side.
			if(tx < g_mapsz.x-1)
				for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
				//for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
				{
					uint16_t nx = (tx+1)*IN_PATHNODES-1;
					//uint16_t ny = (ty-off.y)*IN_PATHNODES;

					//node index
					int32_t nin = nx + ny * g_pathdim.x;
					int32_t nin2 = (nx+1) + ny * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int32_t tpin = tx + ty * g_mapsz.x;
					g_tilepass[tpin].add(SDIR_E, g_tregs[nin], g_tregs[nin2]);

					//Log("add2");
				}

			//Go along top side.
			if(ty > 0)
				//for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
				for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
				{
					//uint16_t nx = tx*IN_PATHNODES;
					uint16_t ny = ty*IN_PATHNODES;

					//node index
					int32_t nin = nx + ny * g_pathdim.x;
					int32_t nin2 = nx + (ny-1) * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int32_t tpin = tx + ty * g_mapsz.x;
					g_tilepass[tpin].add(SDIR_N, g_tregs[nin], g_tregs[nin2]);

					//Log("add3");
				}

			//Go along bottom side.
			if(ty < g_mapsz.y-1)
				//for(uint16_t ny=ty*IN_PATHNODES; ny<(ty+1)*IN_PATHNODES; ny++)
				for(uint16_t nx=tx*IN_PATHNODES; nx<(tx+1)*IN_PATHNODES; nx++)
				{
					//uint16_t nx = (tx+1)*IN_PATHNODES-1;
					uint16_t ny = (ty+1)*IN_PATHNODES-1;

					//node index
					int32_t nin = nx + ny * g_pathdim.x;
					int32_t nin2 = nx + (ny+1) * g_pathdim.x;

					if(g_tregs[nin] == UCHAR_MAX)
						continue;

					if(g_tregs[nin2] == UCHAR_MAX)
						continue;

					//tilepass index
					int32_t tpin = tx + ty * g_mapsz.x;
					g_tilepass[tpin].add(SDIR_S, g_tregs[nin], g_tregs[nin2]);

					//Log("add4");
				}
#endif
		}

#undef IN_PATHNODES

	//This must be done everytime forests are changed
	//or something.
}
