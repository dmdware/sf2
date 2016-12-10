










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
#include "../render/transaction.h"

bool AnyPath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, int32_t cdtype,
                 Unit* thisu, Unit* targu, Building* targb,
                 int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
				 int32_t nminx, int32_t nminy, int32_t nmaxx, int32_t nmaxy /*, int32_t maxdist */ )
{
	UType* ut = &g_utype[utype];

#if 0
	if(thisu)
	{
		RichText rt("pathf");
		NewTransx(thisu->cmpos, &rt);
	}
#endif

	if(!targb)
	{
		switch(umode)
		{
		case UMODE_GODEMB:
		case UMODE_ATDEMB:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
			targb = &g_building[target];
			break;
		case UMODE_GOSUP:
		case UMODE_ATSUP:
			if(thisu)
				targb = &g_building[thisu->supplier];
			break;
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
			if(thisu)
				targb = &g_building[thisu->fuelstation];
			break;
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		}
	}

	if(!targu)
	{
		switch(umode)
		{
		case UMODE_GOSUP:
		case UMODE_GODEMB:
		case UMODE_GOREFUEL:
		case UMODE_REFUELING:
		case UMODE_ATDEMB:
		case UMODE_ATSUP:
		case UMODE_GOBLJOB:
		case UMODE_BLJOB:
		case UMODE_GOCSTJOB:
		case UMODE_CSTJOB:
		case UMODE_GOSHOP:
		case UMODE_SHOPPING:
		case UMODE_GOREST:
		case UMODE_RESTING:
		case UMODE_NONE:
		case UMODE_GOCDJOB:
		case UMODE_CDJOB:
		case UMODE_GODEMCD:
		case UMODE_ATDEMCD:
			break;
		case UMODE_GODRIVE:
		case UMODE_DRIVE:
			targu = &g_unit[target];
			break;
		}
	}

	//could be cleaner, not rely on thisu->cdtype
	//if(targtype == TARG_CD && thisu)
	// TO DO
	if((umode == UMODE_GODEMCD ||
		umode == UMODE_ATDEMCD ||
		umode == UMODE_GOCDJOB) &&
		thisu)
	{
		CdType* ct = &g_cdtype[cdtype];
		//...and not muck around with cmgoalmin/max
		CdTile* ctile = GetCd(cdtype, target, target2, false);
		Vec2i ccmpos = Vec2i(target, target2)*TILE_SIZE + ct->physoff;
		cmgoalminx = ccmpos.x - TILE_SIZE/2;
		cmgoalminy = ccmpos.y - TILE_SIZE/2;
		cmgoalmaxx = cmgoalminx + TILE_SIZE;
		cmgoalmaxy = cmgoalminy + TILE_SIZE;
	}

#if 0
	if((umode == UMODE_GOSUP ||
		umode == UMODE_GODEMB ||
		umode == UMODE_GOREFUEL) &&
		thisu)
	{
		Building* b;

		if(umode == UMODE_GOSUP)
			b = &g_building[thisu->supplier];
		else if(umode == UMODE_GODEMB)
			b = &g_building[thisu->target];
		else if(umode == UMODE_GOREFUEL)
			b = &g_building[thisu->fuelstation];

		BlType* bt = &g_bltype[b->type];

		int32_t btminx = b->tpos.x - bt->width.x/2;
		int32_t btminy = b->tpos.y - bt->width.y/2;
		int32_t btmaxx = btminx + bt->width.x - 1;
		int32_t btmaxy = btminy + bt->width.y - 1;

		cmgoalminx = btminx * TILE_SIZE;
		cmgoalminy = btminy * TILE_SIZE;
		cmgoalmaxx = cmgoalminx + bt->width.x*TILE_SIZE - 1;
		cmgoalmaxy = cmgoalminy + bt->width.y*TILE_SIZE - 1;
	}
#endif

	PathJob pj;
	pj.utype = utype;
	pj.umode = umode;
	pj.cmstartx = cmstartx;
	pj.cmstarty = cmstarty;
	pj.target = target;
	pj.target2 = target2;
	pj.targtype = targtype;
	//pj.path = path;
	//pj.subgoal = subgoal;
	pj.thisu = thisu ? thisu - g_unit : -1;
	pj.targu = targu ? targu - g_unit : -1;
	pj.targb = targb ? targb - g_building : -1;
	//pj.goalx = (cmgoalminx+cmgoalmaxx)/2;
	//pj.goaly = (cmgoalminy+cmgoalmaxy)/2;
	pj.goalx = cmgoalx;
	pj.goaly = cmgoaly;
	pj.goalx = pj.goalx / PATHNODE_SIZE;
	pj.goaly = pj.goaly / PATHNODE_SIZE;
	pj.goalminx = cmgoalminx;
	pj.goalminy = cmgoalminy;
	pj.goalmaxx = cmgoalmaxx;
	pj.goalmaxy = cmgoalmaxy;
	pj.roaded = ut->roaded;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.airborne = ut->airborne;
	pj.callback = Callback_UnitPath;
	pj.pjtype = PATHJOB_ANYPATH;
	//pj.maxsearch = maxsearch;
	pj.cdtype = cdtype;
	//pj.cmgoal = Vec2i(cmgoalx, cmgoaly);

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)

	//pj->process();

	ResetPathNodes();//
	//fixme //SnapToNode(&pj, );

	int32_t depth = 0;
	const int32_t maxdepth = (TILE_SIZE * 20) * (TILE_SIZE * 20) / PATHNODE_SIZE / PATHNODE_SIZE;

#if 0	//fixme

	while( g_openlist.hasmore() )
	{
#if 1
		depth++;

		if(depth > maxdepth)
			break;
#endif

		// Pops the lowest cost-cost node, moves it in the closed std::list
		PathNode* node = (PathNode*)g_openlist.delmin();

		node->closed = true;

		// If the popped node is the endNode, return it
		if( AtGoal(&pj, node) )
		{
			CleanPath(g_toclear);
			return true;
		}

		// otherwise, identify successors of the popped node
		Vec2i npos = PATHNODEPOS(node);

#if 0
		if(npos.x < nminx)
			continue;
		if(npos.y < nminy)
			continue;
		if(npos.x > nmaxx)
			continue;
		if(npos.y > nmaxy)
			continue;
#endif

#if 0	//fixme
		//for(unsigned char i=0; i<DIRS; i++)
		for(unsigned char i=0; i<SDIRS; i++)	//only straight paths
		{
			if(!Standable(&pj, npos.x + STRAIGHTOFF[i].x, npos.y + STRAIGHTOFF[i].y))
				continue;

			//we don't care about computing travel
			//edit: or do we? which will get a result faster?

			Vec2i nextnpos(npos.x + STRAIGHTOFF[i].x, npos.y + STRAIGHTOFF[i].y);
			PathNode* nextn = PATHNODEAT(nextnpos.x, nextnpos.y);

			if(nextn->opened)
				continue;

			nextn->rund = node->rund + 1 * 2;

			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			int32_t H = PATHHEUR( nextnpos - Vec2i(pj.goalx, pj.goaly) ) * 2;
			nextn->heapkey.cost = nextn->rund + H;
			//nextn->heapkey.cost = PATHHEUR( nextnpos - Vec2i(pj.goalx, pj.goaly) ) * 2;
			//nextn->prev = node;
			g_openlist.insert(nextn);
			nextn->opened = true;
		}
#endif
	}

	CleanPath(g_toclear);
	
#endif // 0

	return false;
}

void Expand_AP(PathJob* pj, PathNode* node)
{
#if 0
	Vec2i npos = PATHNODEPOS(node);

	int32_t thisdistance = PATHHEUR(Vec2i(npos.x - pj->goalx, npos.y - pj->goaly)) * 2;

	if( !pj->closestnode || thisdistance < pj->closest )
	{
		pj->closestnode = node;
		pj->closest = thisdistance;
	}

	//int32_t rund = 0;

	//if(node->prev)
	//	rund = node->prev->rund;

	int32_t rund = node->rund;

	bool stand[DIRS];

//	for(int32_t i=0; i<DIRS; i++)
//		stand[i] = Standable(pj, npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y);

	bool pass[DIRS];

	pass[DIR_NW] = stand[DIR_NW] && stand[DIR_N] && stand[DIR_W];
	pass[DIR_N] = stand[DIR_N];
	pass[DIR_NE] = stand[DIR_NE] && stand[DIR_N] && stand[DIR_E];
	pass[DIR_E] = stand[DIR_E];
	pass[DIR_SE] = stand[DIR_SE] && stand[DIR_S] && stand[DIR_E];
	pass[DIR_S] = stand[DIR_S];
	pass[DIR_SW] = stand[DIR_SW] && stand[DIR_S] && stand[DIR_W];
	pass[DIR_W] = stand[DIR_W];

	for(int32_t i=0; i<DIRS; i++)
	{
		if(!pass[i])
			continue;

		int32_t newd = rund + STEPDIST[i];

		Vec2i nextnpos(npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y);
		PathNode* nextn = PATHNODEAT(nextnpos.x, nextnpos.y);

		if(!nextn->closed && (!nextn->opened || newd < nextn->rund))
		{
			g_toclear.push_back(nextn); // Records this node to reset its properties later.
			nextn->rund = newd;
			const Vec2i noff = nextnpos - Vec2i(pj->goalx, pj->goaly);
			int32_t H = PATHHEUR( noff ) * 2;
			nextn->heapkey.cost = nextn->rund + H;
			nextn->prev = node;

			if( !nextn->opened )
			{
				g_openlist.insert(nextn);
				nextn->opened = true;
			}
			else
			{
				g_openlist.heapify(nextn);
			}
		}
	}
#endif
}
