











#include "order.h"
#include "../render/shader.h"
#include "selection.h"
#include "unit.h"
#include "mvtype.h"
#include "../render/heightmap.h"
#include "simdef.h"
#include "simflow.h"
#include "../window.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/matrix.h"
#include "../math/hmapmath.h"
#include "../sound/sound.h"
#include "player.h"
#include "map.h"
#include "../net/lockstep.h"
#include "../gui/layouts/chattext.h"

std::list<OrderMarker> g_order;

void DrawOrders(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
#if 0
	UseS(SHADER_BILLBOARD);
	//glBegin(GL_QUADS);

	OrderMarker* o;
	Vec2i p;
	float r;
	float a;

	Shader* s = g_sh+g_curS;

	//matrix

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	std::list<Widget*>::iterator oitr = g_order.begin();

	while(oitr != g_order.end())
	{
		o = &*oitr;
		p = o->pos;
		r = o->radius;
		a = 1.0f - (float)(GetTicks() - o->tick)/(float)ORDER_EXPIRE;

		/*
		glColor4f(1, 1, 1, a);

		0, 0);		glVertex3f(p.x - r, p.y + 1, p.z - r);
		0, 1);		glVertex3f(p.x - r, p.y + 1, p.z + r);
		1, 1);		glVertex3f(p.x + r, p.y + 1, p.z + r);
		1, 0);		glVertex3f(p.x + r, p.y + 1, p.z - r);
		*/

		glUniform4f(s->slot[SSLOT_COLOR], 0, 1, 0, a);

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		float y1 = g_hmap.accheight(p.x + r, p.z - r);
		float y2 = g_hmap.accheight(p.x + r, p.z + r);
		float y3 = g_hmap.accheight(p.x - r, p.z + r);
		float y4 = g_hmap.accheight(p.x - r, p.z - r);
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1 + TILE_SIZE/20, p.z - r,          1, 0,
			p.x + r, y2	+ TILE_SIZE/20, p.z + r,          1, 1,
			p.x - r, y3 + TILE_SIZE/20, p.z + r,          0, 1,

			p.x - r, y3 + TILE_SIZE/20, p.z + r,          0, 1,
			p.x - r, y4 + TILE_SIZE/20, p.z - r,          0, 0,
			p.x + r, y1 + TILE_SIZE/20, p.z - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SLOT::NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if(GetTicks() - o->tick > ORDER_EXPIRE)
		{
			oitr = g_order.erase(oitr);
			continue;
		}

		oitr++;
	}

	//glEnd();
	//glColor4f(1, 1, 1, 1);
	//glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

	EndS();
#endif
}

void Order(int mousex, int mousey, int viewwidth, int viewheight)
{
	
	Vec3i mapgoal;
	Vec3f ray;
	Vec3f point;
	IsoToCart(Vec2i(mousex,mousey)+g_scroll, &ray, &point);

	if(!MapInter(&g_hmap, ray, point, &mapgoal))
		return;

#if 1
	std::list<int>::iterator uit = g_sel.mv.begin();
	int counted = 0;

	std::list<unsigned short> ordered;

	while(uit != g_sel.mv.end())
	{
		Mv* mv = &g_mv[*uit];

		if(mv->owner != g_localP)
		{
			uit++;
			continue;
		}

		MvType* mvt = &g_mvtype[mv->type];

		if(!mvt->military)
		{
			uit++;
			continue;
		}

#if 0
		if(e->hp <= 0.0f)
		{
			g_sel.erase( selitr );
			continue;
		}
#endif

#if 0
		char cmess[128];
		sprintf(cmess, "rec %d", *uit);
		RichText mess = RichText(cmess);
		AddNotif(&mess);
#endif

		ordered.push_back(*uit);
		uit++;
	}

	if(ordered.size() <= 0)
		return;

	MoveOrderPacket* mop = (MoveOrderPacket*)malloc(
		sizeof(MoveOrderPacket) + 
		sizeof(unsigned short) * (short)ordered.size() );
	mop->header.type = PACKET_MOVEORDER;
	mop->mapgoal = Vec2i(mapgoal.x, mapgoal.y);
	mop->nunits = ordered.size();

	std::list<unsigned short>::iterator oit=ordered.begin();
	for(unsigned short oin=0; oit!=ordered.end(); oit++, oin++)
		mop->mv[oin] = *oit;

	LockCmd((PacketHeader*)mop);

	free(mop);

#endif
}

void MoveOrder(MoveOrderPacket* mop)
{
	Vec2i mapgoal = mop->mapgoal;

	std::list<unsigned short> mv;

	if(mop->nunits <= 0)
		return;

	for(unsigned short uit = 0; uit < mop->nunits; uit++)
	{
		
#if 0
		char cmess[128];
		sprintf(cmess, "exec %d", (int)mop->mv[uit] );
		RichText mess = RichText(cmess);
		AddNotif(&mess);
#endif

		mv.push_back( mop->mv[uit] );
	}

	int ownerP = g_mv[ mop->mv[0] ].owner;

#if 0
	InfoMess("asd", "ord");
#endif

	//int temp;
	//PlaceUnit(MV_TRUCK, Vec2i(mapgoal.x,mapgoal.y), 0, &temp);

#if 0
	InfoMess("asd", "found surf");
#endif

	//std::vector<int> selection = g_sel;
	//int selecttype = g_selectType;

	//g_sel.clear();

	//SelPt();

	//std::vector<int> targets = g_sel;
	//int targettype = g_selectType;

	//g_sel = selection;
	//g_selectType = selecttype;

	//char msg[128];
	//sprintf(msg, "s.size()=%d, stype=%d", (int)g_sel.size(), g_selectType);
	//Chat(msg);

	Vec2i vmin = Vec2i(0,0);
	Vec2i vmax = Vec2i((g_mapsz.x+1)*TILE_SIZE-1, (g_mapsz.y+1)*TILE_SIZE-1);
	Vec2i center(0,0);

#if 0
	char msg[256];
	sprintf(msg, "mapmaxmin:(%f,%f)->(%f,%f)", vmin.x, vmin.z, vmax.x, vmax.z);
	InfoMess("asd", msg);
#endif

	Py* py = &g_py[g_localP];

	//if(targets.size() <= 0 || (targettype != SELECT_UNIT && targettype != SELECT_BUILDING))
	{
#if 0
		int i;
#endif
		Mv* mv;
		MvType* t;
		Vec2i order = mapgoal;

#if 0
		g_order.push_back(OrderMarker(order, GetTicks(), 100));
#endif
		//order.x = Clip(order.x, 0, g_mapsz.X*TILE_SIZE);
		//order.z = Clip(order.z, 0, g_mapsz.Z*TILE_SIZE);
		Vec2i p;

		std::list<unsigned short>::iterator sit = mv.begin();
		int counted = 0;

		while(sit != mv.end())
		{
			u = &g_mv[*sit];
			MvType* mvt = &g_mvtype[mv->type];

			p = Vec2i(mv->cmpos.x, mv->cmpos.y);
			//e->target = -1;
			//e->underOrder = ectrue;

			if(p.x < vmin.x)
				vmin.x = p.x;
			if(p.y < vmin.y)
				vmin.y = p.y;
			if(p.x > vmax.x)
				vmax.x = p.x;
			if(p.y > vmax.y)
				vmax.y = p.y;

			//center = center + p;
			center = (center * counted + p) / (counted+1);

			counted ++;
			sit++;
		}

		if(counted <= 0)
			return;

		//center = center / (float)g_sel.size();
		//Vec3f half = (std::min + std::max) / 2.0f;

#if 0
		char msg[256];
		sprintf(msg, "minmax:(%f,%f)->(%f,%f),order:(%f,%f)", vmin.x, vmin.z, vmax.x, vmax.z, order.x, order.z);
		InfoMess("asd", msg);
#endif

		// All mv to one goal
		//if(fabs(center.x - order.x) < half.x && fabs(center.z - order.z) < half.z)
		if(order.x <= vmax.x && order.x >= vmin.x && order.y <= vmax.y && order.y >= vmin.y)
		{
#if 0
			InfoMess("asd", "typ 1");
#endif

			int radius = 0;
			sit = mv.begin();

			// Get the biggest unit width/radius
			while(sit != mv.end())
			{
				u = &g_mv[*sit];

				MvType* mvt = &g_mvtype[mv->type];

				mv->resetpath();
				mv->goal = Vec2i(order.x, order.y);
				mv->underorder = ectrue;
				mv->target = -1;
				mv->pathdelay = 0;
				mv->lastpath = g_simframe;
				//e->underorder = ectrue;
				t = &g_mvtype[mv->type];
				if(t->size.x > radius)
					radius = t->size.x;
				//e->pathblocked = ecfalse;
				sit++;
			}

			g_order.push_back(OrderMarker(order, GetTicks(), radius*0.5f));

#if 0
			Vec2i pixpos = CartToIso(order);
			Vec2i screenpos = pixpos - g_scroll;

			if(screenpos.x >= 0 &&
				screenpos.y >= 0 &&
				screenpos.x < g_width &&
				screenpos.y < g_height)
#else
			if(ownerP == g_localP)
#endif
				Sound_Order();
		}
		// Formation goal
		else
		{
#if 0
			InfoMess("asd", "typ 2");
#endif

			Vec2i offset;

			sit = mv.begin();

			while(sit != mv.end())
			{
				u = &g_mv[*sit];
				MvType* mvt = &g_mvtype[mv->type];

				p = Vec2i(mv->cmpos.x, mv->cmpos.y);
				offset = p - center;
				Vec2i goal = order + offset;
				mv->resetpath();
				mv->goal = Vec2i(goal.x, goal.y);
				mv->underorder = ectrue;
				mv->pathdelay = 0;
				mv->lastpath = g_simframe;
				mv->target = -1;
				t = &g_mvtype[mv->type];
				int radius = t->size.x;
				//mv->goal.x = Clip(mv->goal.x, 0 + radius, g_mapsz.X*TILE_SIZE - radius);
				//mv->goal.z = Clip(mv->goal.z, 0 + radius, g_mapsz.Z*TILE_SIZE - radius);
				//mv->pathblocked = ecfalse;
				g_order.push_back(OrderMarker(goal, GetTicks(), radius*0.5f));
				sit++;
			}

#if 0
			Vec2i pixpos = CartToIso(mapgoal);
			Vec2i screenpos = pixpos - g_scroll;
			
			if(screenpos.x >= 0 &&
				screenpos.y >= 0 &&
				screenpos.x < g_width &&
				screenpos.y < g_height )
#else
			if(ownerP == g_localP)
#endif
				Sound_Order();
		}

		//AckSnd();
	}
#if 0
	else if(targets.size() > 0 && targettype == SELECT_UNIT)
	{
		int targi = targets[0];
		CUnit* targu = &g_mv[targi];
		Vec3f p = targu->camera.Position();

		if(targu->owner == g_localP)
			return;

		MakeWar(g_localP, targu->owner);

		for(int j=0; j<g_sel.size(); j++)
		{
			int i = g_sel[j];
			CUnit* u = &g_mv[i];
			mv->goal = p;
			mv->underOrder = ectrue;
			mv->targetU = ectrue;
			mv->target = targi;
		}

		AckSnd();
	}
	else if(targets.size() > 0 && targettype == SELECT_BUILDING)
	{
		int targi = targets[0];
		Bl* targb = &g_bl[targi];
		Vec3f p = targb->pos;

		if(targb->owner == g_localP)
			return;

		MakeWar(g_localP, targb->owner);

		for(int j=0; j<g_sel.size(); j++)
		{
			int i = g_sel[j];
			CUnit* u = &g_mv[i];
			mv->goal = p;
			mv->underOrder = ectrue;
			mv->targetU = ecfalse;
			mv->target = targi;
		}

		AckSnd();
	}
#endif
}
