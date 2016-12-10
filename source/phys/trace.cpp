










#include "../path/collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/mvtype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../math/3dmath.h"
#include "collision.h"
#include "../math/vec2i.h"
#include "../math/plane2i.h"
#include "../path/pathnode.h"

#define EPSILON_I		1

//experimental, so far, haven't found a way to use fixmath instead of floats for pass ratios
ecbool PassUnits(Vec2i vstart, Vec2i vend,
               int cmminx, int cmminy, int cmmaxx, int cmmaxy,
               Mv* u2)
{
	float startratio = -1.0f;
	float endratio = 1.0f;
	ecbool startsout = ecfalse;

	MvType* u2t = &g_mvtype[u2->type];

	int cmminx2 = u2->cmpos.x - u2t->size.x/2;
	int cmminy2 = u2->cmpos.y - u2t->size.x/2;
	int cmmaxx2 = cmminx2 + u2t->size.x;
	int cmmaxy2 = cmminy2 + u2t->size.x;

	Plane2i planes[4];
	planes[0] = Plane2i(-1, 0, PlaneDistance(Vec2i(-1, 0), Vec2i(cmminx2, cmminy2)));
	planes[1] = Plane2i(1, 0, PlaneDistance(Vec2i(1, 0), Vec2i(cmmaxx2, cmminy2)));
	planes[2] = Plane2i(0, -1, PlaneDistance(Vec2i(0, -1), Vec2i(cmminx2, cmminy2)));
	planes[3] = Plane2i(0, 1, PlaneDistance(Vec2i(0, 1), Vec2i(cmminx2, cmmaxy2)));

	for(int i = 0; i < 4; i++)
	{
		Plane2i* p = &planes[i];

		float offset = 0;

#if 0
		if(tj->type == TRACE_SPHERE)
			offset = tj->radius;

		float startdistance = Dot(tj->absstart, p->normal) + (p->d + offset);
		float enddistance = Dot(tj->absend, p->normal) + (p->d + offset);
#endif
		int startdistance;
		int enddistance;

		Vec2i voffset(0,0);
#if 0
		if(tj->type == TRACE_BOX)
#endif
		{

			voffset.x = (p->normal.x < 0) ? cmmaxx : cmminx;
			voffset.y = (p->normal.y < 0) ? cmmaxy : cmminy;
#if 0
			voffset.z = (p->normal.z < 0) ? tj->vmax.z : tj->vmin.z;
#endif

			startdistance = Dot(vstart + voffset, p->normal) + p->d;
			enddistance = Dot(vend + voffset, p->normal) + p->d;
		}

		if(startdistance > 0)	startsout = ectrue;

		if(startdistance > 0 && enddistance > 0)
			return ectrue;

		if(startdistance <= 0 && enddistance <= 0)
			continue;

		if(startdistance > enddistance)
		{
			float ratio1 = (startdistance - EPSILON_I) / (float)(startdistance - enddistance);

			if(ratio1 > startratio)
			{
				startratio = ratio1;
#if 0

				tw->collisionnormal = p->normal;

				if((tj->start.x != tj->end.x || tj->start.z != tj->end.z) && p->normal.y != 1 && p->normal.y >= 0.0f)
					//if((tj->start.x != tj->end.x || tj->start.z != tj->end.z))
				{
					tw->trytostep = ectrue;

					//if(debugb)
					//	InfoMess("asd", "try step");
				}

				if(tw->collisionnormal.y > 0.2f)
					tw->onground = ectrue;
#endif
			}
		}
		else
		{
			float ratio = (float)( (startdistance + EPSILON_I) / (startdistance - enddistance) );

			if(ratio < endratio)
				endratio = ratio;
		}
	}

#if 0
	tw->collided = ectrue;

	Texture* ptex = &g_texture[b->texture];

	if(ptex->ladder)
		tw->atladder = ectrue;
#endif

	if(startsout == ecfalse)
	{
#if 0
		tw->stuck = ectrue;
#endif
		return ecfalse;
	}

	if(startratio < endratio)
	{
		//if(startratio > -1 && startratio < tw->traceratio)
		if(startratio > -1)
		{
			if(startratio < 0)
				startratio = 0;

#if 0
			g_selB.push_back(b);

			tw->traceratio = startratio;
#endif
			return ecfalse;
		}
	}

	return ectrue;
}

int Trace(int mvtype, int umode,
          Vec2i vstart, Vec2i vend,
          Mv* thisu, Mv* targu, Bl* targb)
{
	MvType* mvt = &g_mvtype[mvtype];

	int cmminx = -mvt->size.x/2;
	int cmminy = -mvt->size.x/2;
	int cmmaxx = cmminx + mvt->size.x;
	int cmmaxy = cmminx + mvt->size.x;

	Vec2i absmin( imin(vstart.x + cmmaxx, vend.x + cmmaxx), imin(vstart.y + cmmaxy, vend.y + cmmaxy) );
	Vec2i absmax( imax(vstart.x + cmmaxx, vend.x + cmmaxx), imax(vstart.y + cmmaxy, vend.y + cmmaxy) );

	int nminx = absmin.x / PATHNODE_SIZE;
	int nminy = absmin.y / PATHNODE_SIZE;
	int nmaxx = absmax.x / PATHNODE_SIZE;
	int nmaxy = absmax.y / PATHNODE_SIZE;

#if 0
	for(int nx = nminx; nx <= nmaxx; nx++)
		for(int ny = nminy; ny <= nmaxy; ny++)
		{
			ColliderTile* cell = ColliderAt(nx, ny);

#if 0	//TODO
			for(short uiter = 0; uiter < 4; uiter++)
			{
				short uindex = cell->mv[uiter];

				if(uindex < 0)
					continue;

				Mv* mv = &g_mv[uindex];

				if(u == thisu)
					continue;

				if(u == targu)
					continue;

				if(!PassUnits(vstart, vend, cmminx, cmminy, cmmaxx, cmmaxy, u))
					return COLLIDER_UNIT;
			}
#endif
		}
#endif

	return COLLIDER_NONE;
}
