










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
#include "jpspath.h"
#include "pathnode.h"
#include "../render/shader.h"
#include "../sim/player.h"
#include "pathdebug.h"
#include "../math/isomath.h"
#include "../sim/map.h"

std::vector<Vec3f> g_gridvecs;
Unit* g_pathunit = NULL;

void DrawSteps()
{
	Shader* s = &g_shader[g_curS];
	Player* py = &g_player[g_localP];

	//if(g_sel.units.size() <= 0)
	//	return;

	//int32_t ui = *g_sel.units.begin();
	//Unit* u = &g_unit[ui];

	std::vector<Vec3f> lines;

#if 0

	Vec2i npos = Vec2i( u->cmpos.x / PATHNODE_SIZE, u->cmpos.y / PATHNODE_SIZE );
	int32_t nminx = imax(0, npos.x-50);
	int32_t nminz = imax(0, npos.y-50);
	int32_t nmaxx = imin(g_pathdim.x-1, npos.x+50);
	int32_t nmaxz = imin(g_pathdim.y-1, npos.y+50);

	for(int32_t x = nminx; x <= nmaxx; x ++)
		for(int32_t y = nminz; y <= nmaxz; y++)
		{
			PathNode* n = PATHNODEAT(x, y);

			if(!n->prev)
				continue;

			Vec2i nprevpos = PATHNODEPOS(n->prev);

			Vec3f to;
			Vec3f from;

			to.x = x * PATHNODE_SIZE + PathOff(ut->size.x);
			to.z = y * PATHNODE_SIZE + PathOff(ut->size.x);
			to.y = g_hmap.accheight(to.x, to.z) + TILE_SIZE/20;

			from.x = nprevpos.x * PATHNODE_SIZE + PathOff(ut->size.x);
			from.z = nprevpos.y * PATHNODE_SIZE + PathOff(ut->size.x);
			from.y = g_hmap.accheight(from.x, from.z) + TILE_SIZE/20;

			lines.push_back(from);
			lines.push_back(to);
		}
#elif 0	//fixme
	int32_t nminx = 0;
	int32_t nminz = 0;
	int32_t nmaxx = g_pathdim.x-1;
	int32_t nmaxz = g_pathdim.y-1;

	for(int32_t x = nminx; x <= nmaxx; x ++)
		for(int32_t y = nminz; y <= nmaxz; y++)
		{
			PathNode* n = PATHNODEAT(x, y);

			if(!n->prev)
				continue;

			Vec2i nprevpos = PATHNODEPOS(n->prev);

			Vec3f to;
			Vec3f from;

			to.x = x * PATHNODE_SIZE + PATHNODE_SIZE/2;
			to.z = y * PATHNODE_SIZE + PATHNODE_SIZE/2;
			to.y = g_hmap.accheight(to.x, to.z) + TILE_SIZE/20;

			from.x = nprevpos.x * PATHNODE_SIZE + PATHNODE_SIZE/2;
			from.z = nprevpos.y * PATHNODE_SIZE + PATHNODE_SIZE/2;
			from.y = g_hmap.accheight(from.x, from.z) + TILE_SIZE/20;

			lines.push_back(from);
			lines.push_back(to);
		}
#elif 0
	int32_t si = 0;
	std::list<Widget*>::iterator siter = wt->openlist.begin();
	for(; siter != wt->openlist.end(); si++, siter++)
	{
		PathNode* tS = &*siter;
		PathNode* prevS = tS->prev;

		if(!prevS)
			continue;

		Vec3f fromvec;
		Vec3f tovec;

		fromvec.x = prevS->nx * PATHNODE_SIZE + PathOff(ut->size.x);
		fromvec.z = prevS->ny * PATHNODE_SIZE + PathOff(ut->size.x);
		fromvec.y = g_hmap.accheight(fromvec.x, fromvec.z) + TILE_SIZE/200;

		tovec.x = tS->nx * PATHNODE_SIZE + PathOff(ut->size.x);
		tovec.z = tS->ny * PATHNODE_SIZE + PathOff(ut->size.x);
		tovec.y = g_hmap.accheight(tovec.x, tovec.z) + TILE_SIZE/200;

		lines.push_back(fromvec);
		lines.push_back(tovec);
	}
#endif

	if(lines.size() <= 0)
		return;

	glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0, 1);
	//glBegin(GL_LINES);

#ifdef PLATFORM_GL14
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &lines[0]);
	glVertexPointer(3, GL_FLOAT, 0, &lines[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &lines[0]);
#endif
	
	glDrawArrays(GL_LINES, 0, lines.size());
}

void DrawGrid()
{
	Shader* s = &g_shader[g_curS];

#if 1
	if(g_gridvecs.size() > 0)
	{
		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glBegin(GL_LINES);

#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &g_gridvecs[0]);
		glVertexPointer(3, GL_FLOAT, 0, &g_gridvecs[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &g_gridvecs[0]);
#endif

		if(s->slot[SSLOT_TEXCOORD0] != -1)    glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, 0, &g_gridvecs[0]);
		glDrawArrays(GL_LINES, 0, g_gridvecs.size());
	}
#endif

#if 1
	glUniform4f(s->slot[SSLOT_COLOR],  0.5f, 0, 0, 1);

	Player* py = &g_player[g_localP];

	if(g_sel.units.size() > 0)
	{
		int32_t i = *g_sel.units.begin();
		Unit* u = &g_unit[i];

		bool roadVeh = false;
#if 1
		if(g_utype[u->type].roaded)
			roadVeh = true;
#endif

		UType* t = &g_utype[u->type];

		int32_t ux = u->cmpos.x / PATHNODE_SIZE;
		int32_t uz = u->cmpos.y / PATHNODE_SIZE;

#if 0
		//debug pathing
		double unminx = (u->cmpos.x - t->size.x/2) / (double)PATHNODE_SIZE;
		double unminz = (u->cmpos.y - t->size.x/2) / (double)PATHNODE_SIZE;

		static bool first = false;

		if(!first)
		{
			first = true;
			char msg[128];
			sprintf(msg, "nxd,y:%lf,%lf", unminx, unminz);
			InfoMess(msg, msg);
		}
#endif

#if 1
		for(int32_t x=imax(0, ux-50); x<imin(ux+50, g_pathdim.x); x++)
			for(int32_t y=imax(0, uz-50); y<imin(uz+50, g_pathdim.y); y++)
			{
				//ColliderTile* cell = ColliderAt(x, y);

				bool blocked = false;

				//if(roadVeh && !(cell->flags & FLAG_HASROAD))
				if(roadVeh)
				{
					CdTile* cdtile = GetCd(CD_ROAD, u->cmpos.x / TILE_SIZE, u->cmpos.y / TILE_SIZE, false);

					if(!(cdtile->on && cdtile->finished))
						blocked = true;
				}

				int32_t ni = PATHNODEINDEX(x, y);

				//if (g_collider.on(ni))
				//	blocked = true;

				PathNode* n = g_pathnode + ni;
				if(n->flags & PATHNODE_BLOCKED)
					blocked = true;

				//if(cell->flags & FLAG_ABRUPT)
				//	blocked = true;

				bool foundother = false;

#if 0	//TODO
				for(int16_t uiter = 0; uiter < 4; uiter++)
				{
					if(cell->units[uiter] < 0)
						continue;

					int16_t uindex = cell->units[uiter];

					Unit* u2 = &g_unit[uindex];

					if(u2 != u && !u2->hidden())
					{
						foundother = true;
						break;
					}
				}
#endif

				bool foundbl = false;

				//if(cell->building >= 0)
				//	foundbl = true;

				if(!foundother && !blocked && !foundbl)
					continue;

				std::vector<Vec3f> vecs;
				vecs.push_back(Vec3f(x*PATHNODE_SIZE, 1, y*PATHNODE_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHNODE_SIZE, 1, (y+1)*PATHNODE_SIZE));
				vecs.push_back(Vec3f((x+1)*PATHNODE_SIZE, 1, y*PATHNODE_SIZE));
				vecs.push_back(Vec3f(x*PATHNODE_SIZE, 1, (y+1)*PATHNODE_SIZE));

				vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/20;
				vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/20;
				vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/20;
				vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/20;

#ifdef PLATFORM_GL14
				//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
				glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
				
#ifdef PLATFORM_GLES20
				glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
				
				glDrawArrays(GL_LINES, 0, vecs.size());
			}
#endif
	}
#endif

	if(g_gridvecs.size() > 0)
		return;

	//return;

	//g_gridvecs.reserve( g_pathdim.x * g_pathdim.y );

	for(int32_t x=0; x<g_pathdim.x-1; x++)
	{
		for(int32_t y=0; y<g_pathdim.y-1; y++)
		{
			int32_t i = g_gridvecs.size();

			g_gridvecs.push_back(Vec3f(x*PATHNODE_SIZE, 0 + 1, y*PATHNODE_SIZE));
			g_gridvecs.push_back(Vec3f(x*PATHNODE_SIZE, 0 + 1, (y+1)*PATHNODE_SIZE));
			g_gridvecs[i+0].y = g_hmap.accheight(g_gridvecs[i+0].x, g_gridvecs[i+0].z) + TILE_SIZE/20;
			g_gridvecs[i+1].y = g_hmap.accheight(g_gridvecs[i+1].x, g_gridvecs[i+1].z) + TILE_SIZE/20;
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, 0);
			//glVertex3f(x*(MIN_RADIUS*2.0f), 0 + 1, g_map.g_mapsz.Z*TILE_SIZE);
		}
	}

	for(int32_t y=0; y<g_pathdim.y-1; y++)
	{
		for(int32_t x=0; x<g_pathdim.x-1; x++)
		{
			int32_t i = g_gridvecs.size();
			g_gridvecs.push_back(Vec3f(x*PATHNODE_SIZE, 0 + 1, y*PATHNODE_SIZE));
			g_gridvecs.push_back(Vec3f((x+1)*PATHNODE_SIZE, 0 + 1, y*PATHNODE_SIZE));
			g_gridvecs[i+0].y = g_hmap.accheight(g_gridvecs[i+0].x, g_gridvecs[i+0].z) + TILE_SIZE/20;
			g_gridvecs[i+1].y = g_hmap.accheight(g_gridvecs[i+1].x, g_gridvecs[i+1].z) + TILE_SIZE/20;
			//glVertex3f(0, 0 + 1, y*(MIN_RADIUS*2.0f));
			//glVertex3f(g_map.g_mapsz.X*TILE_SIZE, 0 + 1, y*(MIN_RADIUS*2.0f));
		}
	}

	//glEnd();
	//glColor4f(1, 1, 1, 1);
}

void DrawUnitSquares()
{
	Unit* u;
	UType* t;
	Vec3f p;
	Shader* s = &g_shader[g_curS];

	glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0, 0, 1);

	for(int32_t i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		t = &g_utype[u->type];
		p = Vec3f(u->cmpos.x, u->drawpos.y + TILE_SIZE/100, u->cmpos.y);

#if 1
		if(u->collided)
			glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 0, 0, 1);
		else
			glUniform4f(s->slot[SSLOT_COLOR], 0.2f, 0, 0, 1);
#endif

		/*
		 glVertex3f(p.x - r, 0 + 1, p.z - r);
		 glVertex3f(p.x - r, 0 + 1, p.z + r);
		 glVertex3f(p.x + r, 0 + 1, p.z + r);
		 glVertex3f(p.x + r, 0 + 1, p.z - r);
		 glVertex3f(p.x - r, 0 + 1, p.z - r);
		 */

		Vec3i vmin(u->cmpos.x - t->size.x/2, 0, u->cmpos.y - t->size.x/2);

		std::vector<Vec3f> vecs;
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z + t->size.x));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z + t->size.x));
		vecs.push_back(Vec3f(vmin.x + t->size.x, 0 + 1, vmin.z));
		vecs.push_back(Vec3f(vmin.x, 0 + 1, vmin.z));

		vecs[0].y = g_hmap.accheight(vecs[0].x, vecs[0].z) + TILE_SIZE/20;
		vecs[1].y = g_hmap.accheight(vecs[1].x, vecs[1].z) + TILE_SIZE/20;
		vecs[2].y = g_hmap.accheight(vecs[2].x, vecs[2].z) + TILE_SIZE/20;
		vecs[3].y = g_hmap.accheight(vecs[3].x, vecs[3].z) + TILE_SIZE/20;
		vecs[4].y = g_hmap.accheight(vecs[4].x, vecs[4].z) + TILE_SIZE/20;

#ifdef PLATFORM_GL14
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
		
		glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
	}

}

void DrawPaths()
{
#if 1
	Player* py = &g_player[g_localP];
	Shader* s = &g_shader[g_curS];

	int32_t i = 0;

	if(g_sel.units.size() <= 0)
		return;

	i = *g_sel.units.begin();
#else
	for(int32_t i=0; i<UNITS; i++)
#endif
	{
		glUniform4f(s->slot[SSLOT_COLOR], 0, 1, 0, 1);

		Unit* u = &g_unit[i];

#if 0
		if(!u->on)
			continue;
#endif

		std::vector<Vec3f> vecs;

		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 0, 1);
		vecs.push_back(Vec3f(u->cmpos.x, g_hmap.accheight(u->cmpos.x, u->cmpos.y) + TILE_SIZE/20, u->cmpos.y));
		vecs.push_back(Vec3f(u->goal.x, g_hmap.accheight(u->goal.x, u->goal.y) + TILE_SIZE/20, u->goal.y));
		
#ifdef PLATFORM_GL14
		glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
		
#ifdef PLATFORM_GLES20
		glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
		
		//glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		
		vecs.clear();

		glUniform4f(s->slot[SSLOT_COLOR], 0, 1, 0, 1);

		Vec3f p;
		p.x = u->cmpos.x;
		p.z = u->cmpos.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/20;

		for(std::list<Vec2i>::iterator piter = u->path.begin(); piter != u->path.end(); piter++)
		{
			p.x = piter->x;
			p.z = piter->y;
			p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/20;
			//glVertex3f(p->x, p->y + 5, p->z);
			vecs.push_back(p);
			//vecs.push_back(p+Vec3f(0,10,0));
		}

#if 0
		p.x = u->goal.x;
		p.z = u->goal.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
#endif

		vecs.clear();	//temp

		if(vecs.size() > 1)
		{
#ifdef PLATFORM_GL14
			//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			//glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
			
#ifdef PLATFORM_GLES20
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
			
			//glDrawArrays(GL_LINE_STRIP, 0, vecs.size());

			for (std::vector<Vec3f>::iterator piter = vecs.begin(); piter != vecs.end(); piter++)
			{
				Vec3i pi;
				pi.x = piter->x;
				pi.y = piter->y;
				pi.z = piter->z;
				Vec2i pixpos = CartToIso(pi) - g_scroll;

				DrawImage(g_texture[0].texname, pixpos.x - 15, pixpos.y - 15, pixpos.x + 15, pixpos.y + 15, 0, 0, 1, 1, g_gui.m_crop);
			}
		}
		else
		{
#if 0
			//vecs.push_back(u->camera.Position() + Vec3f(0,5,0));
			//vecs.push_back(u->goal + Vec3f(0,5,0));

			glUniform4f(s->slot[SSLOT_COLOR], 0.8f, 1, 0.8f, 1);

			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
#endif
		}

		vecs.clear();

		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 1, 0.5f, 1);

		for(std::list<Vec2s>::iterator titer = u->tpath.begin(); titer != u->tpath.end(); titer++)
		{
			p.x = titer->x * TILE_SIZE + TILE_SIZE/2;
			p.z = titer->y * TILE_SIZE + TILE_SIZE/2;
			p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/20;
			//glVertex3f(p->x, p->y + 5, p->z);
			vecs.push_back(p);
			//vecs.push_back(p+Vec3f(0,10,0));
		}

#if 0
		p.x = u->goal.x;
		p.z = u->goal.y;
		p.y = g_hmap.accheight(p.x, p.z) + TILE_SIZE/100;
#endif

		if(vecs.size() > 1)
		{
#ifdef PLATFORM_GL14
			//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
			
#ifdef PLATFORM_GLES20
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
			
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#if 1
void DrawVelocities()
{
	Unit* u;
	Vec3f p;
	UType* t;
	Shader* s = &g_shader[g_curS];

	glUniform4f(s->slot[SSLOT_COLOR], 1, 0, 1, 1);

	for(int32_t i=0; i<UNITS; i++)
	{
		u = &g_unit[i];

		if(!u->on)
			continue;

		t = &g_utype[u->type];

		std::vector<Vec3f> vecs;

		//TODO
		//vecs.push_back(u->drawpos + Vec3f(0, TILE_SIZE/20, 0));
		//Vec3f prevpos = Vec3f(u->prevpos.x, g_hmap.accheight(u->prevpos.x, u->prevpos.y), u->prevpos.y);
		//vecs.push_back(u->drawpos + (u->drawpos - prevpos) * (10*t->cmspeed) + Vec3f(0, TILE_SIZE/20, 0));

		if(vecs.size() > 0)
		{
#ifdef PLATFORM_GL14
			//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
			//glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
			glVertexPointer(3, GL_FLOAT, 0, &vecs[0]);
#endif
			
#ifdef PLATFORM_GLES20
			glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vecs[0]);
#endif
			
			glDrawArrays(GL_LINE_STRIP, 0, vecs.size());
		}
	}
}

#endif


void LogPathDebug()
{
	if(g_pathunit)
	{
		g_pathunit = NULL;
		return;
	}

	Player* py = &g_player[g_localP];

	if(g_sel.units.size() <= 0)
		return;

	int32_t i = *g_sel.units.begin();

	g_pathunit = &g_unit[i];
}
