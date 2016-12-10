











#include "build.h"
#include "bltype.h"
#include "../render/shader.h"
#include "../platform.h"
#include "../window.h"
#include "../math/camera.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "building.h"
#include "../utils.h"
#include "mvtype.h"
#include "unit.h"
#include "../render/water.h"
#include "../phys/collision.h"
#include "../gui/richtext.h"
#include "../gui/font.h"
#include "../math/vec4f.h"
#include "../gui/icon.h"
#include "player.h"
#include "../app/appmain.h"
#include "../gui/gui.h"
#include "../gui/widget.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../render/foliage.h"
#include "umove.h"
#include "simdef.h"
#include "simflow.h"
#include "../math/fixmath.h"
#include "../path/pathnode.h"
#include "job.h"
#include "../render/drawsort.h"
#include "map.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"
#include "../gui/layouts/playgui.h"

int g_build = -1;

void UpdSBl()
{
	Py* py = &g_py[g_localP];
	Camera* c = &g_cam;

	if(g_build == BL_NONE)
		return;

	//g_vdrag[0] = Vec3f(-1,-1,-1);
	//g_vdrag[1] = Vec3f(-1,-1,-1);

#if 0
	Vec3f ray = ScreenPerspRay(g_mouse.x, g_mouse.y, g_width, g_height, c->zoompos(), c->strafe, c->up2(), c->view - c->pos, FIELD_OF_VIEW);

	Vec3f intersect;

	Vec3f line[2];
	line[0] = c->zoompos();
	line[1] = line[0] + ray * MAX_DISTANCE / 3 / g_zoom;
#else
	Vec3f ray;
	Vec3f point;
	IsoToCart(g_mouse+g_scroll, &ray, &point);
	Vec3i intersect;
    Vec3f fint;
    Vec3f line[2];
    line[0] = point - ray * (MAX_MAP * 5 * TILE_SIZE);
    line[1] = point + ray * (MAX_MAP * 2 * TILE_SIZE);
	//if(!MapInter(&g_hmap, ray, point, &intersect))
    if(!FastMapIntersect(&g_hmap, g_mapsz, line, &fint))
		return;
    intersect = Vec3i((int)fint.x, (int)fint.y, (int)fint.z);
#endif

	if(!g_mousekeys[MOUSE_LEFT])
		g_vdrag[0] = intersect;
	else
		g_vdrag[1] = intersect;

	g_canplace = ectrue;

	if(g_build < BL_TYPES)
	{
		Vec2i tpos (intersect.x/TILE_SIZE, intersect.y/TILE_SIZE);

		g_vdrag[0].x = tpos.x * TILE_SIZE;
		g_vdrag[0].y = tpos.y * TILE_SIZE;

		BlType* t = &g_bltype[g_build];

		if(t->width.x%2 == 1)
			g_vdrag[0].x += TILE_SIZE/2;
		if(t->width.y%2 == 1)
			g_vdrag[0].y += TILE_SIZE/2;

		//g_vdrag[0].y = Lowest(tpos.x, tpos.y, tpos.x, tpos.y);

		if(!CheckCanPlace(g_build, tpos, -1))
		{
			g_canplace = ecfalse;
			g_bpcol = g_collidertype;
		}
		//PlaceBl(type, tpos, ectrue, -1, -1, -1);
	}
	else if(g_build >= BL_TYPES && g_build < BL_TYPES+CD_TYPES)
	{
		int ctype = g_build - BL_TYPES;

		if(g_mousekeys[0])
			UpdCdPlans(ctype, g_localP, g_vdrag[0], g_vdrag[1]);
		else
			UpdCdPlans(ctype, g_localP, g_vdrag[0], g_vdrag[0]);
	}
}

void DrawSBl(unsigned int renderdepthtex, unsigned int renderfb)
{
	Py* py = &g_py[g_localP];

	if(g_build == BL_NONE)
		return;
	Shader* s = g_sh+g_curS;

#if 0
	UseS(SHADER_DEEPTEAMELEV);
	Shader* s = g_sh+g_curS;
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
#endif
	if(g_build < BL_TYPES)
	{
		//Log("dr");

		if(g_canplace)
			glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
		else
			glUniform4f(s->slot[SSLOT_COLOR], 1, 0, 0, 0.5f);
		//m->draw(0, g_vdrag[0], 0);

		//Sprite* sp = NULL;

		BlType* t = &g_bltype[g_build];
		Vec2i tpos = Vec2i((int)g_vdrag[0].x / TILE_SIZE, (int)g_vdrag[0].y / TILE_SIZE);

		//Vec3f vmin(b->drawpos.x - t->width.x*TILE_SIZE/2, b->drawpos.y, b->drawpos.y - t->width.y*TILE_SIZE/2);
		//Vec3f vmax(b->drawpos.x + t->width.x*TILE_SIZE/2, b->drawpos.y + (t->width.x+t->width.y)*TILE_SIZE/2, b->drawpos.y + t->width.y*TILE_SIZE/2);

		//if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.y, vmax.x, vmax.y, vmax.y))
		//	continue;
		
		Tile surf = SurfTile(tpos.x,tpos.y, &g_hmap);
		int sli = t->splist;
		SpList* sl = &g_splist[ sli ];
		int ci = SpriteRef(sl, 0, surf.incltype, 0, 0, 0,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Py* py = &g_py[g_localP];
		float* color = py->color;
		glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		Vec3i cmpos = Vec3i( tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0), tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0), surf.elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		Texture* difftex = &g_texture[ sp->difftexi ];
		Texture* depthtex = &g_texture[ sp->depthtexi ];
		Texture* teamtex = &g_texture[ sp->teamtexi ];

		int rendz;

		CartToDepth(cmpos, &rendz);

#if 0
		DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.crop);
#else	
		DrawDeep(difftex->texname, depthtex->texname, teamtex->texname, 
			renderdepthtex, renderfb,
			rendz, cmpos.z,
			(float)screenpos.x + sp->cropoff[0],
			(float)screenpos.y + sp->cropoff[1],
			(float)screenpos.x + sp->cropoff[2],
			(float)screenpos.y + sp->cropoff[3], 
			sp->crop[0], sp->crop[1], 
			sp->crop[2], sp->crop[3]);
#endif
	}
#if 0
	else if(g_build == BL_ROAD)
	{
	}
	else if(g_build == BL_POWL)
	{
	}
	else if(g_build == BL_CRPIPE)
	{
	}
#endif
}

void DrawBReason(Matrix* mvp, float width, float height, ecbool persp)
{
	Py* py = &g_py[g_localP];

	if(g_canplace || g_build == BL_NONE)
		return;

	Vec3f pos3 = Vec3f(g_vdrag[0].x, g_vdrag[0].y, g_vdrag[0].y);

	if(g_build >= BL_TYPES)
		pos3 = Vec3f(g_vdrag[1].x, g_vdrag[1].y, g_vdrag[1].y);

	RichText reason;

	Vec4f pos4 = ScreenPos(mvp, pos3, width, height, persp);

	switch(g_bpcol)
	{
	case COLLIDER_NONE:
		reason.part.push_back(RichPart(UStr("")));
		break;
	case COLLIDER_UNIT:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" A unit is in the way.")));
		break;
	case COLLIDER_BUILDING:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" Another building is in the way.")));
		break;
	case COLLIDER_TERRAIN:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" Buildings must be placed on even terrain.")));
		break;
	case COLLIDER_NOROAD:
		reason.part.push_back(RichPart(UStr("")));
		break;
	case COLLIDER_OTHER:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" Can't place here.")));
		break;
	case COLLIDER_NOLAND:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" This building must be placed on land.")));
		break;
	case COLLIDER_NOSEA:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" This structure must be placed in the sea.")));
		break;
	case COLLIDER_NOCOAST:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" This building must be placed along the coast.")));
		break;
	case COLLIDER_ROAD:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" A road is in the way.")));
		break;
	case COLLIDER_OFFMAP:
		reason.part.push_back(RichPart(RICH_ICON, ICON_EXCLAMATION));
		reason.part.push_back(RichPart(UStr(" Bl is out of bounds.")));
		break;
	}

	float color[] = {0.9f,0.7f,0.2f,0.8f};
	//DrawCenterShadText(MAINFONT32, pos4.x, pos4.y-64, &reason, color, -1);
	DrawBoxShadText(MAINFONT32, pos4.x-200, pos4.y-64-100, 400, 200, &reason, color, 0, -1);
}

ecbool BlLevel(int type, Vec2i tpos)
{
#if 1
	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tpos.x - t->width.x/2;
	tmin.y = tpos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x;
	tmax.y = tmin.y + t->width.y;

	float miny = g_hmap.getheight(tmin.x, tmin.y);
	float maxy = g_hmap.getheight(tmin.x, tmin.y);

	ecbool haswater = ecfalse;
	ecbool hasland = ecfalse;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
			float thisy = g_hmap.getheight(x, y);

			if(thisy < miny)
				miny = thisy;

			if(thisy > maxy)
				maxy = thisy;

			// Must have two adject water tiles to be water-vessel-accessible
			if(thisy < WATER_LEVEL)
			{
				// If y is along building edge and x and x+1 are water tiles
				if((y==tmin.y || y==tmax.y) && x+1 <= g_mapsz.x && x+1 <= tmax.x && g_hmap.getheight(x+1, y) < WATER_LEVEL)
					haswater = ectrue;
				// If x is along building edge and y and y+1 are water tiles
				if((x==tmin.x || x==tmax.x) && y+1 <= g_mapsz.y && y+1 <= tmax.y && g_hmap.getheight(x, y+1) < WATER_LEVEL)
					haswater = ectrue;
			}
			// Must have two adjacent land tiles to be road-accessible
			else if(thisy > WATER_LEVEL)
			{
				// If y is along building edge and x and x+1 are land tiles
				if((y==tmin.y || y==tmax.y) && x+1 <= g_mapsz.x && x+1 <= tmax.x && g_hmap.getheight(x+1, y) > WATER_LEVEL)
					hasland = ectrue;
				// If x is along building edge and y and y+1 are land tiles
				if((x==tmin.x || x==tmax.x) && y+1 <= g_mapsz.y && y+1 <= tmax.y && g_hmap.getheight(x, y+1) > WATER_LEVEL)
					hasland = ectrue;
			}
		}

	if(miny != maxy && !t->hugterr)
	{
		return ecfalse;
	}
	
	if(miny < WATER_LEVEL)
	{
#if 0
		haswater = ectrue;
#endif
		miny = WATER_LEVEL;
	}

#if 0
	if(maxy > WATER_LEVEL)
		hasland = ectrue;
#endif

	if(maxy - miny > MAX_CLIMB_INCLINE)
	{
		g_collidertype = COLLIDER_TERRAIN;
		return ecfalse;
	}

	if(t->foundation == FD_LAND)
	{
		if(haswater)
		{
			g_collidertype = COLLIDER_NOLAND;
			return ecfalse;
		}
		if(!hasland)
		{
			g_collidertype = COLLIDER_NOLAND;
			return ecfalse;
		}
	}
	else if(t->foundation == FD_SEA)
	{
		if(!haswater)
		{
			g_collidertype = COLLIDER_NOSEA;
			return ecfalse;
		}
		if(hasland)
		{
			g_collidertype = COLLIDER_NOSEA;
			return ecfalse;
		}
	}
	else if(t->foundation == FD_COAST)
	{
		if(!haswater || !hasland)
		{
			g_collidertype = COLLIDER_NOCOAST;
			return ecfalse;
		}
	}

#if 0
	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
		{
			if(g_hmap.getheight(x, y) != compare)
				return ecfalse;

			if(g_hmap.getheight(x, y) <= WATER_LEVEL)
				return ecfalse;
		}
#endif
#endif

	return ectrue;
}

ecbool Offmap(int minx, int miny, int maxx, int maxy)
{
	if(minx < 0 || miny < 0
	                || maxx >= g_mapsz.x*TILE_SIZE
	                || maxy >= g_mapsz.y*TILE_SIZE)
	{
		g_collidertype = COLLIDER_OFFMAP;
		return ectrue;
	}

	return ecfalse;
}

ecbool Collides(Vec2i cmmin, Vec2i cmmax, int targb)
{
	if(Offmap(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return ectrue;

	Vec2i tmin = cmmin / TILE_SIZE;
	Vec2i tmax = cmmax / TILE_SIZE;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
			if(GetCd(CD_ROAD, x, y, ecfalse)->on)
			{
				g_collidertype = COLLIDER_ROAD;
				return ectrue;
			}

		//return ecfalse;

	if(CollidesWithBuildings(cmmin.x, cmmin.y, cmmax.x, cmmax.y, targb))
		return ectrue;

	if(CollidesWithUnits(cmmin.x, cmmin.y, cmmax.x, cmmax.y, ecfalse, NULL, NULL))
		return ectrue;

	if(CollidesWithTerr(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return ectrue;

	return ecfalse;
}

ecbool BlCollides(int type, Vec2i tpos, int targb)
{
	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = tpos.x - t->width.x/2;
	tmin.y = tpos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x - 1;
	tmax.y = tmin.y + t->width.y - 1;

	Vec2i cmmin;
	Vec2i cmmax;

	cmmin.x = tmin.x * TILE_SIZE;
	cmmin.y = tmin.y * TILE_SIZE;
	cmmax.x = cmmin.x + t->width.x*TILE_SIZE - 1;
	cmmax.y = cmmin.y + t->width.y*TILE_SIZE - 1;

	if(Offmap(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return ectrue;

	for(int x=tmin.x; x<=tmax.x; x++)
		for(int y=tmin.y; y<=tmax.y; y++)
			if(GetCd(CD_ROAD, x, y, ecfalse)->on)
			{
				g_collidertype = COLLIDER_ROAD;
				return ectrue;
			}

		//return ecfalse;

	if(CollidesWithBuildings(cmmin.x, cmmin.y, cmmax.x, cmmax.y, targb))
		return ectrue;

	if(CollidesWithUnits(cmmin.x, cmmin.y, cmmax.x, cmmax.y, ecfalse, NULL, NULL))
		return ectrue;

	if(CollidesWithTerr(cmmin.x, cmmin.y, cmmax.x, cmmax.y))
		return ectrue;

	return ecfalse;
}

ecbool CheckCanPlace(int type, Vec2i tpos, int targb)
{
	if(!BlLevel(type, tpos))
		return ecfalse;

	if(BlCollides(type, tpos, targb))
		return ecfalse;
	
	return ectrue;
}

/*
 Check each building, conduit, unit if they can still
 stand with given elevation change. If not, remove them.
 */

void RecheckStand()
{
    for(int i=0; i<BUILDINGS; i++)
    {
        Bl* b = &g_bl[i];
        
        if(!b->on)
            continue;
        
        if(CheckCanPlace(b->type, b->tpos, i))
            continue;
        
        b->destroy();
        b->on = ecfalse;
    }
    
    for(int ctype=0; ctype<CD_TYPES; ctype++)
        for(int tx=0; tx<g_mapsz.x; tx++)
            for(int ty=0; ty<g_mapsz.y; ty++)
            {
                CdTile* ctile = GetCd(ctype, tx, ty, ecfalse);
                if(!ctile->on)
                    continue;
                if(CdLevel(ctype, tx, ty, tx, ty, 0, 0, 0, 1, ectrue))
                    continue;
                ctile->destroy();
                ctile->on = ecfalse;
                ConnectCdAround(ctype, tx, ty, ecfalse);
            }
    
    for(int i=0; i<MOVERS; i++)
    {
        Mv* mv = &g_mv[i];
        
        if(!mv->on)
            continue;
        
        if(!UnitCollides(u, mv->cmpos, mv->type))
            continue;
        
        mv->destroy();
        mv->on = ecfalse;
    }
    
    for(int i=0; i<CD_TYPES; i++)
        ReNetw(i);
}

ecbool PlaceBl(int type, Vec2i pos, ecbool finished, int owner, int* bid)
{
	int i = NewBl();

	if(bid)
		*bid = i;

	if(i < 0)
		return ecfalse;

	Bl* b = &g_bl[i];
	b->on = ectrue;
	b->type = type;
	b->tpos = pos;

	g_onbl.push_back((unsigned short)i);
	g_onbl.sort();

	BlType* t = &g_bltype[type];

	Vec2i tmin;
	Vec2i tmax;

	tmin.x = pos.x - t->width.x/2;
	tmin.y = pos.y - t->width.y/2;
	tmax.x = tmin.x + t->width.x;
	tmax.y = tmin.y + t->width.y;

	int cmminx = tmin.x*TILE_SIZE;
	int cmminy = tmin.y*TILE_SIZE;
	int cmmaxx = cmminx + t->width.x*TILE_SIZE - 1;
	int cmmaxy = cmminy + t->width.y*TILE_SIZE - 1;

	ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);

#if 0
	b->drawpos = Vec3f(
		pos.x*TILE_SIZE,
		pos.y*TILE_SIZE,
		0.0f);
#endif

	//if(t->foundation == FD_SEA)
	//	b->drawpos.y = WATER_LEVEL;

#if 1
	if(t->width.x % 2 == 1)
		b->drawpos.x += TILE_SIZE/2;

	if(t->width.y % 2 == 1)
		b->drawpos.y += TILE_SIZE/2;

	//b->drawpos.z = Bilerp(&g_hmap, b->drawpos.x, b->drawpos.y);

	b->owner = owner;

	b->finished = finished;

	Zero(b->conmat);
	Zero(b->stocked);
	Zero(b->price);
	b->propprice = 0;
	b->forsale = ecfalse;
	b->demolish = ecfalse;	//TODO one state var unsigned char instead of all these bools finished, forsale, demolish, construction, whatev. or bit field.
	for(int ui=0; ui<MV_TYPES; ui++)
		b->manufprc[ui] = 0;
	for(int ri=0; ri<RESOURCES; ++ri)
		b->price[ri] = t->price[ri];

	b->occupier.clear();
	b->worker.clear();
	b->conwage = DEFL_CSWAGE;
	b->opwage = t->opwage;
	b->cydelay = SIM_FRAME_RATE * 60;	//TODO variable length cy delay for farms etc.
	b->cymet = 0;
	b->lastcy = g_simframe;
	b->prodlevel = RATIO_DENOM;
	b->capsup.clear();

	for(signed char ri=0; ri<RESOURCES; ri++)
		b->transporter[ri] = -1;

	b->hp = 1;

	Zero(b->varcost);
	b->fixcost = 0;

	b->demolition = ecfalse;
	
	b->cyclehist.push_back(CycleHist());

	while(b->cyclehist.size() > 100)
		b->cyclehist.erase(b->cyclehist.begin());

#if 0
	int cmminx = tmin.x*TILE_SIZE;
	int cmminy = tmin.y*TILE_SIZE;
	int cmmaxx = cmminx + t->width.x*TILE_SIZE;
	int cmmaxy = cmminy + t->width.y*TILE_SIZE;

	ClearFol(cmminx, cmminy, cmmaxx, cmmaxy);
#endif
	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		PruneCd(ctype);
		ReNetw(ctype);
	}
#if 0
	ClearPowerlines(cmminx, cmminy, cmmaxx, cmmaxy);
	ClearPipelines(cmminx, cmminy, cmmaxx, cmmaxy);
	RePow();
	RePipe();
	ReRoadNetw();
#endif

#endif

	b->fillcollider();

	g_drawlist.push_back(Dl());
	Dl* d = &*g_drawlist.rbegin();
	b->depth = d;
	d->dtype = DEPTH_BL;
	d->index = i;
	UpDraw(b);

	AddVis(b);
	Explore(b);

	//Log("placebl simf=%d netf=%d type=%d x,y=%d,%d", (int)g_simframe, (int)g_cmdframe, (int)type, (int)pos.x, (int)pos.y);

	if(g_appmode == APPMODE_PLAY)
	{
		b->allocres();
		b->inoperation = ecfalse;

		if(!b->finished && owner == g_localP)
		{
			Py* py = &g_py[g_localP];

			ClearSel(&g_sel);
			g_sel.bl.push_back(i);

			Widget *gui = (Widget*)&g_gui;
			CstrView* cv = (CstrView*)gui->get("cs view");
			cv->regen(&g_sel);
			gui->show("cs view");
		}

		NewJob(UMODE_GOCSTJOB, i, -1, CD_NONE);
	}
	else
	{
		b->inoperation = ectrue;
	}

	return ectrue;
}

//find to place building about certain tile
//place not multiple of 4
ecbool PlaceBAb4(int btype, Vec2i tabout, Vec2i* tplace)
{
	//Log("PlaceBAround "<<player);
	//

	BlType* t = &g_bltype[btype];
	int shell = 0;

	//char msg[128];
	//sprintf(msg, "place b a %f,%f,%f", vAround.x/16, vAround.y/16, vAround.y/16);
	//Chat(msg);

	do
	{
		std::vector<Vec2i> canplace;
		Vec2i ttry;
		int tilex, tiley;
		int left, right, top, bottom;
		left = tabout.x - shell;
		top = tabout.y - shell;
		right = tabout.x + shell;
		bottom = tabout.y + shell;

		canplace.reserve( (right-left)*2/TILE_SIZE + (bottom-top)*2/TILE_SIZE );

		tiley = top;
		for(tilex=left; tilex<=right; tilex++)
		{
			if(tiley % 4 == 0)
				continue;
			if(tilex % 3 == 0)
				continue;

			ttry = Vec2i(tilex, tiley);
			
			int cmstartx = ttry.x*TILE_SIZE - t->width.x*TILE_SIZE/2;
			int cmendx = cmstartx + t->width.x*TILE_SIZE - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y*TILE_SIZE/2;
			int cmendy = cmstarty + t->width.y*TILE_SIZE - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = right;
		for(tiley=top; tiley<=bottom; tiley++)
		{
			if(tiley % 4 == 0)
				continue;
			if(tilex % 3 == 0)
				continue;

			ttry = Vec2i(tilex, tiley);
			
			int cmstartx = ttry.x*TILE_SIZE - t->width.x*TILE_SIZE/2;
			int cmendx = cmstartx + t->width.x*TILE_SIZE - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y*TILE_SIZE/2;
			int cmendy = cmstarty + t->width.y*TILE_SIZE - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tiley = bottom;
		for(tilex=right; tilex>=left; tilex--)
		{
			if(tiley % 4 == 0)
				continue;
			if(tilex % 3 == 0)
				continue;

			ttry = Vec2i(tilex, tiley);
			
			int cmstartx = ttry.x*TILE_SIZE - t->width.x*TILE_SIZE/2;
			int cmendx = cmstartx + t->width.x*TILE_SIZE - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y*TILE_SIZE/2;
			int cmendy = cmstarty + t->width.y*TILE_SIZE - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = left;
		for(tiley=bottom; tiley>=top; tiley--)
		{
			if(tiley % 4 == 0)
				continue;
			if(tilex % 3 == 0)
				continue;

			ttry = Vec2i(tilex, tiley);
			
			int cmstartx = ttry.x*TILE_SIZE - t->width.x*TILE_SIZE/2;
			int cmendx = cmstartx + t->width.x*TILE_SIZE - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y*TILE_SIZE/2;
			int cmendy = cmstarty + t->width.y*TILE_SIZE - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		if(canplace.size() > 0)
		{
			//Chat("placing");
			//Log("placeb t="<<btype<<" "<<vTile.x<<","<<vTile.y<<","<<vTile.y<<"("<<(vTile.x/16)<<","<<(vTile.y/16)<<","<<(vTile.y/16)<<")");
			//
			//*tpos = canplace[ rand()%canplace.size() ];
			*tplace = canplace[ 0 ];

			return ectrue;
		}

		//char msg[128];
		//sprintf(msg, "shell %d", shell);
		//Chat(msg);

		shell++;
	} while(shell < g_mapsz.x || shell < g_mapsz.y);

	return ecfalse;
}

//find to place building about certain tile
ecbool PlaceBAb(int btype, Vec2i tabout, Vec2i* tplace)
{
	//Log("PlaceBAround "<<player);
	//

	BlType* t = &g_bltype[btype];
	int shell = 1;

	//char msg[128];
	//sprintf(msg, "place b a %f,%f,%f", vAround.x/16, vAround.y/16, vAround.y/16);
	//Chat(msg);

	do
	{
		std::vector<Vec2i> canplace;
		Vec2i ttry;
		int tilex, tiley;
		int left, right, top, bottom;
		left = tabout.x - shell;
		top = tabout.y - shell;
		right = tabout.x + shell;
		bottom = tabout.y + shell;

		canplace.reserve( (right-left)*2/TILE_SIZE + (bottom-top)*2/TILE_SIZE );

		tiley = top;
		for(tilex=left; tilex<right; tilex++)
		{
			ttry = Vec2i(tilex, tiley);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendy = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = right;
		for(tiley=top; tiley<bottom; tiley++)
		{
			ttry = Vec2i(tilex, tiley);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendy = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tiley = bottom;
		for(tilex=right; tilex>left; tilex--)
		{
			ttry = Vec2i(tilex, tiley);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendy = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		tilex = left;
		for(tiley=bottom; tiley>top; tiley--)
		{
			ttry = Vec2i(tilex, tiley);

			int cmstartx = ttry.x*TILE_SIZE - t->width.x/2;
			int cmendx = cmstartx + t->width.x - 1;
			int cmstarty = ttry.y*TILE_SIZE - t->width.y/2;
			int cmendy = cmstarty + t->width.y - 1;

			if(t->width.x%2 == 1)
			{
				cmstartx += TILE_SIZE/2;
				cmendx += TILE_SIZE/2;
			}
			if(t->width.y%2 == 1)
			{
				cmstarty += TILE_SIZE/2;
				cmendy += TILE_SIZE/2;
			}

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//char msg[128];
			//sprintf(msg, "check %d,%d,%d,%d", startx, startz, endx, endz);
			//Chat(msg);

			if(!CheckCanPlace(btype, ttry, -1))
				continue;
			canplace.push_back(ttry);
		}

		if(canplace.size() > 0)
		{
			//Chat("placing");
			//Log("placeb t="<<btype<<" "<<vTile.x<<","<<vTile.y<<","<<vTile.y<<"("<<(vTile.x/16)<<","<<(vTile.y/16)<<","<<(vTile.y/16)<<")");
			//
			//*tpos = canplace[ rand()%canplace.size() ];
			*tplace = canplace[ 0 ];

			return ectrue;
		}

		//char msg[128];
		//sprintf(msg, "shell %d", shell);
		//Chat(msg);

		shell++;
	} while(shell < g_mapsz.x || shell < g_mapsz.y);

	return ecfalse;
}

//find to place unit about certain position
ecbool PlaceUAb(int mvtype, Vec2i cmabout, Vec2i* cmplace)
{
	MvType* t = &g_mvtype[mvtype];
	int shell = 1;
	//important: mv must be occupying a single free pathnode,
	//or have a space of free pathnodes about them
	int size = imax(t->size.x, PATHNODE_SIZE)*2*2;
	//int size = PATHNODE_SIZE * 2;

	int pathoff = PathOff(t->size.x);

	do
	{
		std::vector<Vec2i> canplace;
		Vec2i cmtry;
		int cmx, cmz;
		int left, right, top, bottom;
		left = cmabout.x - shell*size;
		top = cmabout.y - shell*size;
		right = cmabout.x + shell*size;
		bottom = cmabout.y + shell*size;

		//canplace.reserve( (right-left)*2/size + (bottom-top)*2/size );

		cmz = top;
		for(cmx=left; cmx<right; cmx+=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//align to boundary. only works for odd multiple of PATHNODE_SIZE.
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;
			
			cmtry.x = ((cmtry.x / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;
			cmtry.y = ((cmtry.y / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendy = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			//if(!CheckCanPlace(btype, cmtry))
			if(UnitCollides(NULL, cmtry, mvtype))
				continue;
			canplace.push_back(cmtry);
		}

		cmx = right;
		for(cmz=top; cmz<bottom; cmz+=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;
			
			cmtry.x = ((cmtry.x / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;
			cmtry.y = ((cmtry.y / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendy = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, mvtype))
				continue;
			canplace.push_back(cmtry);
		}

		cmz = bottom;
		for(cmx=right; cmx>left; cmx-=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;
			
			cmtry.x = ((cmtry.x / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;
			cmtry.y = ((cmtry.y / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendy = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, mvtype))
				continue;
			canplace.push_back(cmtry);
		}

		cmx = left;
		for(cmz=bottom; cmz>top; cmz-=size)
		{
			cmtry = Vec2i(cmx, cmz);
			//cmtry = cmtry / PATHNODE_SIZE;
			//cmtry = cmtry * PATHNODE_SIZE;
			
			cmtry.x = ((cmtry.x / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;
			cmtry.y = ((cmtry.y / PATHNODE_SIZE) * PATHNODE_SIZE) + pathoff;

			int cmstartx = cmtry.x - size/2;
			int cmendx = cmstartx + size - 1;
			int cmstarty = cmtry.y - size/2;
			int cmendy = cmstarty + size - 1;

			if(cmstartx < 0)
				continue;
			else if(cmendx >= g_mapsz.x * TILE_SIZE)
				continue;
			if(cmstarty < 0)
				continue;
			else if(cmendy >= g_mapsz.y * TILE_SIZE)
				continue;

			if(UnitCollides(NULL, cmtry, mvtype))
				continue;
			canplace.push_back(cmtry);
		}

		if(canplace.size() > 0)
		{
			*cmplace = canplace[ 0 ];

			return ectrue;
		}

		shell++;
	} while(shell < g_mapsz.x || shell < g_mapsz.y);

	return ecfalse;
}
