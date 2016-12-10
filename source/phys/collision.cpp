#include "collision.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/unit.h"
#include "../sim/mvtype.h"
#include "../render/heightmap.h"
#include "../path/collidertile.h"

int g_lastcollider = -1;
int g_collidertype;
ecbool g_ignored;

ecbool BlAdj(int i, int j)
{
	Bl* bi = &g_bl[i];
	Bl* bj = &g_bl[j];

	BlType* ti = &g_bltype[bi->type];
	BlType* tj = &g_bltype[bj->type];

	Vec2i tpi = bi->tpos;
	Vec2i tpj = bj->tpos;

	Vec2i mini;
	Vec2i minj;
	Vec2i maxi;
	Vec2i maxj;

	mini.x = tpi.x - ti->width.x/2;
	mini.y = tpi.y - ti->width.y/2;
	minj.x = tpj.x - tj->width.x/2;
	minj.y = tpj.y - tj->width.y/2;
	maxi.x = mini.x + ti->width.x;
	maxi.y = mini.y + ti->width.y;
	maxj.x = minj.x + tj->width.x;
	maxj.y = minj.y + tj->width.y;

	if(maxi.x >= minj.x && maxi.y >= minj.y && mini.x <= maxj.x && mini.y <= maxj.y)
		return ectrue;

	return ecfalse;
}

// Is conduit x,z adjacent to building i?
ecbool CoAdj(unsigned char ctype, int i, int x, int z)
{
	Bl* b = &g_bl[i];
	BlType* t = &g_bltype[b->type];

	Vec2i tp = b->tpos;

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tp.x - t->width.x/2;
	tmin.y = tp.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x;
	tmax.y = tmin.y + t->width.y;

	Vec2i cmmin = Vec2i(tmin.x*TILE_SIZE, tmin.y*TILE_SIZE);
	Vec2i cmmax = Vec2i(tmax.x*TILE_SIZE, tmax.y*TILE_SIZE);

	CdType* ct = &g_cdtype[ctype];
	Vec2i ccmp2 = ct->physoff + Vec2i(x, z)*TILE_SIZE;

	Vec2i cmmin2 = Vec2i(ccmp2.x-TILE_SIZE/2, ccmp2.y-TILE_SIZE/2);
	Vec2i cmmax2 = Vec2i(ccmp2.x+TILE_SIZE/2, ccmp2.y+TILE_SIZE/2);

	if(cmmax.x >= cmmin2.x && cmmax.y >= cmmin2.y && cmmin.x <= cmmax2.x && cmmin.y <= cmmax2.y)
		return ectrue;

	return ecfalse;
}

ecbool CollidesWithBuildings(int minx, int miny, int maxx, int maxy, int ignore)
{
	g_ignored = ecfalse;

	for(int i=0; i<BUILDINGS; i++)
	{
		Bl* b = &g_bl[i];

		if(!b->on)
			continue;

		BlType* t = &g_bltype[ b->type ];

		Vec2i tmin;
		Vec2i tmax;

		tmin.x = b->tpos.x - t->width.x/2;
		tmin.y = b->tpos.y - t->width.y/2;
		tmax.x = tmin.x + t->width.x - 1;
		tmax.y = tmin.y + t->width.y - 1;

		Vec2i cmmin2;
		Vec2i cmmax2;

		cmmin2.x = tmin.x * TILE_SIZE;
		cmmin2.y = tmin.y * TILE_SIZE;
		cmmax2.x = cmmin2.x + t->width.x*TILE_SIZE - 1;
		cmmax2.y = cmmin2.y + t->width.y*TILE_SIZE - 1;

		if(maxx >= cmmin2.x && maxy >= cmmin2.y && minx <= cmmax2.x && miny <= cmmax2.y)
		{
			if(i == ignore)
			{
				//g_ignored = ectrue;
				continue;
			}
			/*
			 if(g_debug1)
			 {
			 g_applog<<"fabs((p.x)"<<p.x<<"-(x)"<<x<<") < (hwx)"<<hwx<<"+(hwx2)"<<hwx2<<" && fabs((p.z)"<<p.z<<"- (z)"<<z<<") < (hwz)"<<hwz<<"+(hwz2)"<<hwz2<<std::endl;
			 g_applog<<fabs(p.x-x)<<" < "<<(hwx+hwx2)<<" && "<<fabs(p.z-z)<<" < "<<(hwz+hwz2)<<std::endl;
			 }*/

			g_lastcollider = i;
			g_collidertype = COLLIDER_BUILDING;
			return ectrue;
		}
	}

	return ecfalse;
}


#if 0
ecbool CollidesWithTerr(int cmminx, int cmminy, int cmmaxx, int cmmaxy)
{
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int ny=nminy; ny<=nmaxy; ny++)
		for(int nx=nminx; nx<=nmaxx; nx++)
		{
			ColliderTile* cl = ColliderAt(nx,ny);

			if(cl->flags & FLAG_ABRUPT)
			{
				g_lastcollider = -1;
				g_collidertype = COLLIDER_TERRAIN;
				return ectrue;
			}
		}

	return ecfalse;
}
#endif

ecbool CollidesWithTerr(int cmminx, int cmminy, int cmmaxx, int cmmaxy)
{
	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	for (int ny = nminy; ny <= nmaxy; ny++)
		for (int nx = nminx; nx <= nmaxx; nx++)
		{
			//corpd fix 2016
			int ni = PATHNODEINDEX(nx, ny);

			//if (g_collider.on(ni))

			PathNode* n = g_pathnode + ni;
			if(n->flags & PATHNODE_BLOCKED)
			{
				g_lastcollider = -1;
				g_collidertype = COLLIDER_TERRAIN;
				return ectrue;
			}
		}

	return ecfalse;
}

//TODO circular unit radius
ecbool CollidesWithUnits(int minx, int miny, int maxx, int maxy, ecbool isunit, Mv* thisu, Mv* ignore)
{
	for (int i = 0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if (!mv->on)
			continue;

		if (u == ignore)
			continue;

		if (u == thisu)
			continue;

		if (mv->hidden())
			continue;

		MvType* t = &g_mvtype[mv->type];

		int minx2 = mv->cmpos.x - t->size.x / 2;
		int miny2 = mv->cmpos.y - t->size.x / 2;
		int maxx2 = minx2 + t->size.x - 1;
		int maxy2 = miny2 + t->size.x - 1;

		if (maxx >= minx2 && maxy >= miny2 && minx <= maxx2 && miny <= maxy2)
		{
			g_lastcollider = i;
			g_collidertype = COLLIDER_UNIT;
			return ectrue;
		}
	}

	return ecfalse;
}

ecbool OffMap(int cmminx, int cmminy, int cmmaxx, int cmmaxy)
{
	if(cmminx < 0)
		return ectrue;

	if(cmmaxx >= g_mapsz.x*TILE_SIZE)
		return ectrue;

	if(cmminy < 0)
		return ectrue;

	if(cmmaxy >= g_mapsz.y*TILE_SIZE)
		return ectrue;

	return ecfalse;
}

