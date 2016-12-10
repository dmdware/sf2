













#include "fogofwar.h"
#include "../sim/mvtype.h"
#include "../sim/unit.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "heightmap.h"
#include "../utils.h"
#include "../math/fixmath.h"
#include "../app/appmain.h"
#include "../gui/layouts/messbox.h"

VisTile* g_vistile = NULL;


void AddVis(Mv* mv)
{
	//return;

	//if(mv->owner < 0)
	//	return;
#if 0
	if(mv->owner >= PLAYERS)
		return;
#endif

	MvType* mvt = &g_mvtype[mv->type];
	int vr = mvt->visrange;
	int cmminx = mv->cmpos.x - vr;
	int cmminy = mv->cmpos.y - vr;
	int cmmaxx = mv->cmpos.x + vr;
	int cmmaxy = mv->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<mv->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.x;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.y;
#endif

#if 0
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int ui = u - g_mv;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			//v->uvis.push_back( ui );
			v->vis[mv->owner] = 255;

			//Log("add vis "<<tx<<","<<ty);
		}
}


void UpdVis()
{
	//return;

#if 1
	for(unsigned char ty=0; ty<g_mapsz.y; ++ty)
	{
		for(unsigned char tx=0; tx<g_mapsz.x; ++tx)
		{
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

			for(unsigned char pi=0; pi<PLAYERS; ++pi)
			{
				if(v->vis[pi] == 0)
					continue;
				v->vis[pi] --;
			}
		}
	}
#endif
}

void RemVis(Mv* mv)
{
	//return;

#if 0
	MvType* mvt = &g_mvtype[mv->type];
	int vr = mvt->visrange;
	int cmminx = mv->cmpos.x - vr;
	int cmminy = mv->cmpos.y - vr;
	int cmmaxx = mv->cmpos.x + vr;
	int cmmaxy = mv->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.x;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.y;
#endif

#if 0
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int ui = u - g_mv;

			std::list<Widget*>::iterator vit=v->uvis.begin();
			while(vit!=v->uvis.end())
			{
				if(*vit == ui)
				{
					vit = v->uvis.erase(vit);
					//Log("rem vis "<<tx<<","<<ty);
					continue;
				}

				vit++;
			}
		}
#endif
}

void AddVis(Bl* b)
{
	//return;

	//Log("bt "<<b->type);
	//

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int cmposx = (cmminx+cmmaxx)/2;
	int cmposy = (cmminy+cmmaxy)/2;

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 0
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible

			/*
			Using int bi :

==7857== Invalid write of size 8
==7857==    at 0x5F73B3F: std::__detail::_List_node_base::_M_hook(std::__detail::_List_node_base*) (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.18)
==7857==    by 0x47DE74: void std::list<short, std::allocator<short> >::_M_insert<short>(std::_List_iterator<short>, short&&) (stl_list.h:1562)
==7857==    by 0x47DCDE: std::list<short, std::allocator<short> >::push_back(short&&) (stl_list.h:1021)
==7857==    by 0x47D086: AddVis(Bl*) (fogofwar.cpp:160)
==7857==    by 0x46C514: FillColliderGrid() (collidertile.cpp:281)
==7857==    by 0x4A54B9: LoadMap(char const*) (savemap.cpp:1310)
==7857==    by 0x4368F6: Click_LV_Load() (loadview.cpp:78)
==7857==    by 0x421279: Button::inev(InEv*) (button.cpp:119)
==7857==    by 0x4371F7: LoadView::subinev(InEv*) (loadview.cpp:134)
==7857==    by 0x44C16A: Win::inev(InEv*) (windoww.cpp:347)
==7857==    by 0x41B127: GUI::inev(InEv*) (gui.cpp:100)
==7857==    by 0x550256: EventLoop() (appmain.cpp:1319)
==7857==  Address 0x10000003e2d3148 is not stack'd, malloc'd or (recently) free'd
			*/

			int bi = b - g_bl;

			//Log("v "<<(tx + ty * g_mapsz.x)<<"/"<<(g_mapsz.x*g_mapsz.y)<<" ");
			//

			//Log("(g_vistile == NULL) ?= " << (g_vistile == NULL) << std::endl;
			//

			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

			//Log("((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) ?= " <<
			//((&g_vistile[ tx + ty * g_mapsz.x ]) == (&(g_vistile[ tx + ty * g_mapsz.x ]))) << std::endl;
			//

			//Log("(v == NULL) ?= " << (v == NULL) << std::endl;
			//

			//Log("v s="<<v->bvis.size());
			//

			//v->bvis.push_back( bi );
			v->vis[b->owner] = 255;

			//Log("bav "<<tx<<","<<ty);

			//Log("v s="<<v->bvis.size());
			//
		}
}

void RemVis(Bl* b)
{
#if 0
	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int cmposx = (cmminx+cmmaxx)/2;
	int cmposy = (cmminy+cmmaxy)/2;

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 1
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 1
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			int bi = b - g_bl;

			std::list<Widget*>::iterator vit=v->bvis.begin();
			while(vit!=v->bvis.end())
			{
				if(*vit == bi)
				{
					vit = v->bvis.erase(vit);
					continue;
				}

				vit++;
			}
		}
#endif
}

void Explore(Mv* mv)
{
	//return;

	//if(mv->owner < 0)
	//	return;
#if 0
	if(mv->owner >= PLAYERS)	//TODO figure why this case happens and fix it
	{
#if 0
		char m[126];
		sprintf(m, "uown=%d", mv->owner);
		RichText rm = m;
		Mess(&rm);
#endif
		return;
	}
#endif

	MvType* mvt = &g_mvtype[mv->type];
	int vr = mvt->visrange;
	int cmminx = mv->cmpos.x - vr;
	int cmminy = mv->cmpos.y - vr;
	int cmmaxx = mv->cmpos.x + vr;
	int cmmaxy = mv->cmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);

	///Log("cymmp "<<mv->cmpos.y<<","<<vr);
	//Log("cymm "<<cmminy<<","<<cmmaxy);
	//Log("tymm "<<tminy<<","<<tmaxy);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.x;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - mv->cmpos.y;
#endif

#if 0
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int ui = u - g_mv;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[mv->owner] = ectrue;

			//Log("add vis "<<tx<<","<<ty);
		}
}

void Explore(Bl* b)
{
	//return;

	if(b->owner < 0)
		return;

	BlType* bt = &g_bltype[b->type];
	Vec2i bcmpos;
	//Needs to be offset by half a tile if centered on tile center (odd width)
	bcmpos.x = b->tpos.x * TILE_SIZE + ((bt->width.x % 2 == 1) ? TILE_SIZE/2 : 0);
	bcmpos.y = b->tpos.y * TILE_SIZE + ((bt->width.y % 2 == 1) ? TILE_SIZE/2 : 0);
	int vr = bt->visrange;
	int cmminx = bcmpos.x - vr;
	int cmminy = bcmpos.y - vr;
	int cmmaxx = bcmpos.x + vr;
	int cmmaxy = bcmpos.y + vr;
	int tminx = imax(0, cmminx / TILE_SIZE);
	int tminy = imax(0, cmminy / TILE_SIZE);
	int tmaxx = imin(g_mapsz.x-1, cmmaxx / TILE_SIZE);
	int tmaxy = imin(g_mapsz.y-1, cmmaxy / TILE_SIZE);
	int cmposx = (cmminx+cmmaxx)/2;
	int cmposy = (cmminy+cmmaxy)/2;

	//Log("bav "<<bcmpos.x<<","<<bcmpos.y);

	for(int tx=tminx; tx<=tmaxx; tx++)
		for(int ty=tminy; ty<=tmaxy; ty++)
		{
			//Distance to tile depends on which corner of the
			//tile we're measuring from.
#if 0
			int dcmx1 = labs(tx * TILE_SIZE) - mv->cmpos.x;
			int dcmx2 = labs((tx+1) * TILE_SIZE - 1) - mv->cmpos.x;
			int dcmy1 = labs(ty * TILE_SIZE) - mv->cmpos.y;
			int dcmy2 = labs((ty+1) * TILE_SIZE - 1) - mv->cmpos.y;

			int dcmx = imin(dcmx1, dcmx2);
			int dcmy = imin(dcmy1, dcmy2);
#else
			int dcmx = tx * TILE_SIZE + TILE_SIZE/2 - cmposx;
			int dcmy = ty * TILE_SIZE + TILE_SIZE/2 - cmposy;
#endif

#if 0
			int d = isqrt(dcmx*dcmx + dcmy*dcmy);

			if(d > vr)
				continue;
#else
			int d = dcmx*dcmx + dcmy*dcmy;

			if(d > vr*vr)
				continue;
#endif

			//visible
			int bi = b - g_bl;
			VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
			v->explored[b->owner] = ectrue;

			//Log("bav "<<tx<<","<<ty);
		}
}

ecbool IsTileVis(short py, short tx, short ty)
{
	//return ectrue;	//this isn't an RTS... yet

	if(g_appmode == APPMODE_EDITOR)
		return ectrue;

	//if(g_netmode != NETM_SINGLE)
	//	return ectrue;	//for now

	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];

#if 0
	for(std::list<Widget*>::iterator vit=v->bvis.begin(); vit!=v->bvis.end(); vit++)
	{
		Bl* b = &g_bl[ *vit ];

		if(b->owner == py)
			return ectrue;
	}

	for(std::list<Widget*>::iterator vit=v->uvis.begin(); vit!=v->uvis.end(); vit++)
	{
		Mv* mv = &g_mv[ *vit ];

		//InfoMess("v","v");
		if(mv->owner == py)
			return ectrue;
	}
#else
	return v->vis[py] > 0;
#endif

	return ecfalse;
}

ecbool Explored(short py, short tx, short ty)
{
	//2015/10/27 now says every area is explored until it becomes a full RTS
	//return ectrue;	//this isn't an rts yet
	VisTile* v = &g_vistile[ tx + ty * g_mapsz.x ];
	return v->explored[py];
}
