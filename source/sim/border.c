

#include "border.h"
#include "../platform.h"
#include "../gui/draw2d.h"
#include "../gui/gui.h"
#include "../render/heightmap.h"
#include "map.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "../math/isomath.h"
#include "../render/fogofwar.h"
#include "../sim/unit.h"
#include "../sim/bltype.h"
#include "../sim/building.h"

signed char* g_border = NULL;

void DrawBord(Vec3i cmfrom, Vec3i cmto, Vec2i frompixoff, Vec2i topixoff, float* color)
{				
	Vec2i screenfrom;
	Vec2i screento;
	GUI *gui;

	cmfrom.z = Hmap_geth(&g_hmap, cmfrom.x/TILE_SIZE, cmfrom.y/TILE_SIZE) * TILE_RISE;
	cmto.z = Hmap_geth(&g_hmap, cmto.x/TILE_SIZE, cmto.y/TILE_SIZE) * TILE_RISE;

	Vec2i_sub(&screenfrom, CartToIso(cmfrom), g_scroll);
	Vec2i_sub(&screento, CartToIso(cmto), g_scroll);

	Vec2i_add(&screenfrom, frompixoff, screenfrom);
	Vec2i_add(&screento, topixoff, screenfrom);

	gui = (Widget*)&g_gui;

	DrawLine(color[0], color[1], color[2], color[3], 
		(float)screenfrom.x, (float)screenfrom.y, (float)screento.x, (float)screento.y, 
		gui->crop);
}

#define BORDERPIXOFF	1

void DrawBords(Vec2i tmin, Vec2i tmax)
{
	GUI *gui;
	float* crop;
	Vec3i cmfrom, cmto;
	Vec2i frompixoff, topixoff;
	int i, i2;
	signed char owner, owner2;
	float* color;
	unsigned char tx, ty;
	Py *py;
	
	gui = (Widget*)&g_gui;
	crop = gui->crop;

	for(tx=tmin.x; tx<=tmax.x; tx++)
	{
		for(ty=tmin.y; ty<=tmax.y; ty++)
		{
			i = tx + ty * g_mapsz.x;

			owner = *(g_border+i);

			if(owner < 0)
				continue;

			if(!IsTileVis(g_localP, tx, ty))
			//if(!Explored(g_localP, tx, ty))
				continue;

			py = g_py+owner;
			color = py->color;

			ecbool drawne = ecfalse;
			ecbool drawnw = ecfalse;
			ecbool drawse = ecfalse;
			ecbool drawsw = ecfalse;

			/* north-east */
			if(ty > 0)
			{
				i2 = tx + (ty-1) * g_mapsz.x;
				owner2 = *(g_border+i2);

				if(owner != owner2)
					drawne = ectrue;
			}
			else
				drawne = ectrue;

			if(drawne)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = ty * TILE_SIZE;

				frompixoff.x = -BORDERPIXOFF;
				frompixoff.y = BORDERPIXOFF;
				topixoff.x = -BORDERPIXOFF;
				topixoff.y = BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}
		
			/* north-west */
			if(tx > 0)
			{
				i2 = (tx-1) + ty * (g_mapsz.x+0);
				owner2 = *(g_border+i2);
				
				if(owner != owner2)
					drawnw = ectrue;
			}
			else
				drawnw = ectrue;

			if(drawnw)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = tx * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = BORDERPIXOFF;
				frompixoff.y = BORDERPIXOFF;
				topixoff.x = BORDERPIXOFF;
				topixoff.y = BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}

			/* south-west */
			if(ty < g_mapsz.y-1)
			{
				i2 = tx + (ty+1) * (g_mapsz.x+0);
				owner2 = *(g_border+i2);

				if(owner != owner2)
					drawsw = ectrue;
			}
			else
				drawsw = ectrue;

			if(drawsw)
			{
				cmfrom.x = tx * TILE_SIZE;
				cmfrom.y = (ty+1) * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = BORDERPIXOFF;
				frompixoff.y = -BORDERPIXOFF;
				topixoff.x = BORDERPIXOFF;
				topixoff.y = -BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}

			/* south-east */
			if(tx < g_mapsz.x-1)
			{
				i2 = (tx+1) + ty * (g_mapsz.x+0);
				owner2 = *(g_border+i2);

				if(owner != owner2)
					drawse = ectrue;
			}
			else
				drawse = ectrue;

			if(drawse)
			{
				cmfrom.x = (tx+1) * TILE_SIZE;
				cmfrom.y = ty * TILE_SIZE;
				cmto.x = (tx+1) * TILE_SIZE;
				cmto.y = (ty+1) * TILE_SIZE;

				frompixoff.x = -BORDERPIXOFF;
				frompixoff.y = -BORDERPIXOFF;
				topixoff.x = -BORDERPIXOFF;
				topixoff.y = -BORDERPIXOFF;

				DrawBord(cmfrom, cmto, frompixoff, topixoff, color);
			}
		}
	}
}

void MarkTerr_mv(Mv* mv)
{
	Vec2i t;
	int i;

	Vec2i_divi(&t, mv->cmpos, TILE_SIZE);
	i = t.x + t.y * g_mapsz.x;
	*(g_border+i) = mv->owner;
}

void MarkTerr_bl(Bl* b)
{
	BlType* bt;
	int tminx, tminy, tmaxx, tmaxy, tx, ty, i;

	bt = g_bltype+b->type;
	tminx = b->tpos.x - bt->width.x/2;
	tminy = b->tpos.y - bt->width.y/2;
	tmaxx = tminx + bt->width.x;
	tmaxy = tminy + bt->width.y;

	for(tx=tminx; tx<tmaxx; tx++)
	{
		for(ty=tminy; ty<tmaxy; ty++)
		{
			i = tx + ty * g_mapsz.x;
			*(g_border+i) = b->owner;
		}
	}
}