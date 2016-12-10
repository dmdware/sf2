










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
#include "partialpath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "../render/transaction.h"
#include "../sim/simflow.h"
#include "hotqueue.h"

char AngleDir(int dx, int dy)
{
	if(dx == 0)
		dx = 1;
	//if(dy == 0)
#if 0
	Vec2s(0, 1), //S
	Vec2s(-1, 1), //SW
	Vec2s(-1, 0), //W
	Vec2s(-1, -1), //NW
	Vec2s(0, -1), //N
	Vec2s(1, -1), //NE
	Vec2s(1, 0), //E
	Vec2s(1, 1) //SE
#endif

	if(dy >= 0 && RATIO_DENOM*dy/dx <= RATIO_DENOM*2 && RATIO_DENOM*dy/dx >= RATIO_DENOM*-2)
	{
		return DIR_S;
	}
	if(dy >= 0 && dx >= 0 && RATIO_DENOM*dy/dx <= RATIO_DENOM*2 && RATIO_DENOM*dy/dx >= RATIO_DENOM*1/2)
	{
		return DIR_SE;
	}
	if(dx >= 0 && RATIO_DENOM*dy/dx <= RATIO_DENOM*1/2 && RATIO_DENOM*dy/dx >= RATIO_DENOM*-1/2)
	{
		return DIR_E;
	}
	if(dy <= 0 && dx >= 0 && RATIO_DENOM*dy/dx >= RATIO_DENOM*-1/2 && RATIO_DENOM*dy/dx <= RATIO_DENOM*-2)
	{
		return DIR_NE;
	}
	if(dy <= 0 && RATIO_DENOM*dy/dx <= RATIO_DENOM*2 && RATIO_DENOM*dy/dx >= RATIO_DENOM*-2)
	{
		return DIR_N;
	}
	if(dy <= 0 && dx <= 0 && RATIO_DENOM*dy/dx >= RATIO_DENOM*1/2 && RATIO_DENOM*dy/dx <= RATIO_DENOM*2)
	{
		return DIR_NW;
	}
	if(dx <= 0 && RATIO_DENOM*dy/dx <= 0 && RATIO_DENOM*dy/dx >= RATIO_DENOM*-1/2)
	{
		return DIR_W;
	}

	return DIR_SW;
}

#if 01
void MazePath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, int32_t cdtype, int32_t supplier,
                 std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* targu, Building* targb,
                 int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
                 int32_t maxsearch, bool capend, bool allowpart, bool adjgoalbounds)
{
	UType* ut = &g_utype[utype];

#if 0
	if(thisu)
	{
		RichText rt("part pathf");
		NewTransx(thisu->cmpos, &rt);
	}
#endif

	//TODO FIXME if targb is set to final dest for transport, and not supplier, when checking for passage between truck, supl, and dem, then it will be wrong
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
			targb = &g_building[target];
			break;
		case UMODE_GOREST:
		case UMODE_RESTING:
			//targb = &g_building[target];	//corpd fix
			if(thisu)
				targb = &g_building[thisu->home];
			break;
		case UMODE_GOSUP:
		case UMODE_ATSUP:
			//if(thisu)
				targb = &g_building[supplier];	//corpd fix 2016
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
		//if(adjgoalbounds)
		{
			Vec2i ccmpos = Vec2i(target, target2)*TILE_SIZE + ct->physoff;
			cmgoalminx = ccmpos.x - TILE_SIZE/2;
			cmgoalminy = ccmpos.y - TILE_SIZE/2;
			cmgoalmaxx = cmgoalminx + TILE_SIZE;
			cmgoalmaxy = cmgoalminy + TILE_SIZE;
		}
	}

	//d fix
	if (thisu && targu)
	{
		UType* ut2 = &g_utype[targu->type];

		cmgoalminx = targu->cmpos.x - ut2->size.x / 2;
		cmgoalminy = targu->cmpos.y - ut2->size.x / 2;
		cmgoalmaxx = cmgoalminx + ut2->size.x - 1;
		cmgoalmaxy = cmgoalminy + ut2->size.x - 1;

		cmgoalminx = cmgoalminx / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalminy = cmgoalminy / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalmaxx = cmgoalmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalmaxy = cmgoalmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
	}

	//d fix
	if (/* thisu && */ targb)
	{
		BlType* bt = &g_bltype[targb->type];

		int32_t tgoalminx = targb->tpos.x - bt->width.x / 2;
		int32_t tgoalminy = targb->tpos.y - bt->width.y / 2;
		int32_t tgoalmaxx = tgoalminx + bt->width.x;
		int32_t tgoalmaxy = tgoalminy + bt->width.y;

		cmgoalminx = tgoalminx * TILE_SIZE;
		cmgoalminy = tgoalminy * TILE_SIZE;
		cmgoalmaxx = tgoalmaxx * TILE_SIZE - 1;
		cmgoalmaxy = tgoalmaxy * TILE_SIZE - 1;

		cmgoalminx = cmgoalminx / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalminy = cmgoalminy / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalmaxx = cmgoalmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		cmgoalmaxy = cmgoalmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
	}

	if (thisu && !thisu->hidden())
	{
		thisu->freecollider();
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

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->path = path;
	pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->targu = targu ? targu - g_unit : -1;
	pj->targb = targb ? targb - g_building : -1;
	//pj->goalx = (cmgoalminx+cmgoalmaxx)/2;
	//pj->goaly = (cmgoalminy+cmgoalmaxy)/2;
	pj->goalx = cmgoalx;
	pj->goaly = cmgoaly;
	pj->goalx = pj->goalx / PATHNODE_SIZE;
	pj->goaly = pj->goaly / PATHNODE_SIZE;
	pj->nstartx = cmstartx / PATHNODE_SIZE;
	pj->nstarty = cmstarty / PATHNODE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath;
	pj->pjtype = PATHJOB_MAZE;
	pj->maxsearch = maxsearch;
	pj->cdtype = cdtype;
	pj->nminx = 0;
	pj->nminy = 0;
	pj->nmaxx = g_pathdim.x-1;
	pj->nmaxy = g_pathdim.y-1;
	pj->cmgoal = Vec2i(cmgoalx, cmgoaly);
	pj->capend = capend;
	pj->allowpart = allowpart;

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)

	//Log("pj->umode=%d\r\n", (int32_t)pj->umode);

	g_toclear.reserve(MAXPATHN);

	pj->process();

	if (thisu && !thisu->hidden())
	{
	//if(thisu - g_unit == 182 && g_simframe > 118500)
		//Log("f12");

		thisu->fillcollider();
	}
	
#if 0
	static int32_t pn = 0;

	Log("=====PARTIALPATH %d====\r\n", pn);
	
	Log("g_simframe=%d\r\n", (int32_t)g_simframe);
	Log("g_netframe=%d\r\n", (int32_t)g_netframe);
	Log("pj->thisu=%d\r\n", (int32_t)pj->thisu);
	Log("pj->umode=%d\r\n", (int32_t)pj->umode);
	Log("pj->target=%d\r\n", (int32_t)pj->target);
	Log("pj->target2=%d\r\n", (int32_t)pj->target2);
	Log("pj->cdtype=%d\r\n", (int32_t)pj->cdtype);
	Log("pj->targtype=%d\r\n", (int32_t)pj->targtype);
	
	
	Log("pj->path->size()=%d\r\n", (int32_t)pj->path->size());
	
	int32_t ps = 0;
	for(std::list<Widget*>::iterator pit=pj->path->begin(); pit!=pj->path->end(); pit++, ps++)
		Log("pathstep[%d]=%d,%d\r\n", (int32_t)ps, (int32_t)pit->x, (int32_t)pit->y);
	
	Log("====================\r\n");
	
	pn ++;
#endif

	delete pj;
}

bool Expand_MP(PathJob* pj, PathNode* node, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded, uint8_t dir)
{
#ifndef HASHPOOL
	//Vec2i npos = PATHNODEPOS(node);
	int32_t ni = node - g_pathnode;
	Vec2s npos;
	npos.x = ni % g_pathdim.x;
	npos.y = ni / g_pathdim.x;
	//todo pass gpathdimx as parameter
#else
	Vec2i npos;
	npos.x = node->index % g_pathdim.x;
	npos.y = node->index / g_pathdim.x;
#endif

	//todo divide up collider tiles and pathnodes and arrange them in tiles so a tile fits in cache and they're continuous, not having a huge separation in memory between adjacent nodes

	//FILE* fp = fopen("last.txt", "w");
	//fprintf(fp, "expqp %d,%d", npos.x, npos.y);
	//fflush(fp);
	//fclose(fp);
	Vec2i noff;
	noff.x = npos.x - pj->goalx;
	noff.y = npos.y - pj->goaly;
	int32_t thisdistance = PATHHEUR(noff) << 1;

	if( ( thisdistance < pj->closest || !pj->closestnode ) && 
		dir == PATHNODE_FORWARD)
	{
		pj->closestnode = node;
		pj->closest = thisdistance;
	}

	//int32_t rund = 0;

	//if(node->prev)
	//	rund = node->prev->rund;

	int32_t rund = node->rund;

#ifdef CHEAPLIST
		CheapList * openlist = &g_openlist[dir>>1];
		//CheapList * const openlist2 = &g_openlist[opdir>>1];
#elif defined(HOTQUEUE)
		HotQueue * openlist = &g_openlist[dir>>1];
		//HotQueue * const openlist2 = &g_openlist[opdir>>1];
#elif defined(FIBHEAP)
		FibHeap * openlist = &g_openlist[dir>>1];
		//FibHeap * const openlist2 = &g_openlist[opdir>>1];
#else
		BinHeap * openlist = &g_openlist[dir>>1];
		//BinHeap * const openlist2 = &g_openlist[opdir>>1];
#endif

#if 1
	//uint8_t stand = 0;

	//for(uint8_t i=0; i<DIRS; ++i)
	//	stand |= ( ((uint8_t)Standable(npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i );
	//	stand |= ( ((uint8_t)Standable(npos.x + DIRX(i), npos.y + DIRY(i), nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i );
	
	/*
	http://stackoverflow.com/questions/18097922/return-value-of-operator-in-c

	The comparison (equality and relational) operators (==, !=, <, >, <=, >=) all return 0 for false and 1 for true ? and no other values.

	The logical operators &&, || and ! are less fussy about their operands; they treat 0 as false and any non-zero value as true. However, they also return only 0 for false and 1 for true.
	*/
	
	uint8_t pass = 0;
	uint8_t checked = 0;

	for(uint8_t i=0; i<DIRS; ++i)
	//for(uint8_t i=0; i<(DIRS<<1); ++i)
	{
		//0,1,2,3,4,5,6,7
		//0,2,4,
		//uint8_t i2 = ((i<<1)+((i-1)>>2))%DIRS;
		
		//int32_t newd = rund + STEPDIST[i];
		int32_t newd = rund + DIRDIST(i);

		//todo use ni and offset
		//Vec2i nextnpos(npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y);
		Vec2s nextnpos;
		nextnpos.x = npos.x + DIRX(i);
		nextnpos.y = npos.y + DIRY(i);
		
		const uint8_t opened = (PATHNODE_OPENED<<dir);
		const uint8_t closed = (PATHNODE_CLOSED<<dir);
		const uint8_t opdir = (PATHNODE_BACKWARD-dir);
		const uint8_t opened2 = (PATHNODE_OPENED<<opdir);
		const uint8_t closed2 = (PATHNODE_CLOSED<<opdir);

#ifndef HASHPOOL
		PathNode* nextn = PATHNODEAT(nextnpos.x, nextnpos.y);
#else
		const int32_t nextni = PATHNODEINDEX(nextnpos.x,nextnpos.y);
		//PathNode* nextn = (PathNode*)g_pathmap.get(nextni);
		PathNode* nextn;
		std::list<Widget*>::iterator nextit = g_pathmap.find(nextni);

		//todo is checking for any closed or opened flags being set for this pathnode faster than checking if it's there?

		//if(!nextn)
		if(nextit == g_pathmap.end())
		{
			nextn = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
#ifndef GPATHFLAGS
			nextn->flags = 0;
#endif
			//g_pathflags.clear(PATHNODE_FLAGBITS * node->index + 0);
			//g_pathflags.clear(PATHNODE_FLAGBITS * node->index + 1);
			//g_pathflags.clear(PATHNODE_FLAGBITS * node->index + 2);
			//g_pathflags.clear(PATHNODE_FLAGBITS * node->index + 3);
			nextn->index = nextni;
			//g_pathmap.add(nextni, nextn);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(nextni, nextn));
		}
		else
			nextn = (*nextit).second;
#endif
		
#ifdef GPATHFLAGS
		const int32_t nextni = PATHNODEINDEX(nextnpos.x, nextnpos.y);
#if 0
		const uint8_t flags = 
			(g_pathflags.on(PATHNODE_FLAGBITS * node->index + 0) << 0) |
			(g_pathflags.on(PATHNODE_FLAGBITS * node->index + 1) << 1) |
			(g_pathflags.on(PATHNODE_FLAGBITS * node->index + 2) << 2) |
			(g_pathflags.on(PATHNODE_FLAGBITS * node->index + 3) << 3);
#else
		const uint8_t flags = 
			(g_pathflags.on(PATHNODE_FLAGBITS * nextni + 0) << 0) |
			(g_pathflags.on(PATHNODE_FLAGBITS * nextni + 1) << 1) |
			(g_pathflags.on(PATHNODE_FLAGBITS * nextni + 2) << 2) |
			(g_pathflags.on(PATHNODE_FLAGBITS * nextni + 3) << 3);
#endif
#endif

#ifdef HIERDEBUG
			
		Log("\t yes pass n "<<nextnpos.x<<","<<nextnpos.y);
#endif

#if 0	//don't check here
		//but todo check if there's a 1-pathnode passage that this will still give a path
		if(nextn->flags & (opened2 | closed2))
		{
			//path found

		}
#endif
		
#ifndef GPATHFLAGS
		if(nextn->flags & (opened2 /*| closed2 */))
#else
		if(flags & (opened2 /* | closed2 */))
#endif
		{
#if 1		
			//if(!(pass&(1<<i)))
			if(!(checked&(1<<i)))
			{
				//if(block&(1<<i))
				//	continue;
				//if(!(pass&(1<<i)))
				//	continue;

				checked |= (1<<i);

				//check pass
				pass |= Standable(nextnpos.x, nextnpos.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i;
				//block &= ~(pass&(1<<i));

				if(i%2 == 1)	//diagonal?
				{
					uint8_t i2 = (i+(DIRS-1))&(DIRS-1);
					uint8_t i3 = (i+1)&(DIRS-1);
		
					if(!(checked&(1<<i2)))
					{
						checked |= (1<<i2);
						//pass = pass && Standable(npos.x + OFFSETS[i2].x, npos.y + OFFSETS[i2].y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
						pass |= Standable(npos.x + DIRX(i2), npos.y + DIRY(i2), nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i2;
						//block &= ~(pass&(1<<i2));
					}

					if(!(checked&(1<<i3)))
					{
						checked |= (1<<i3);
						//pass = pass && Standable(npos.x + OFFSETS[i2].x, npos.y + OFFSETS[i2].y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
						pass |= Standable(npos.x + DIRX(i3), npos.y + DIRY(i3), nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i3;
						//block &= ~(pass&(1<<i3));
					}

				//updiag:
					//block |= ( (block&(1<<i)) || (block&(1<<i2)) || (block&(1<<i3)) ) << i;
					pass &= (( (pass&(1<<i2)) && (pass&(1<<i3)) ) << i) | (~(1<<i));
				}

				//if(!(pass&(1<<1)))
				//	continue;

				//if(block&(1<<i))
				//	continue;
			}
		
			if(!(pass&(1<<i)))
				continue;
#endif

			//found path

			PathNode* endnode;	//forward
			PathNode* endnode2;	//backward

			if(dir == PATHNODE_FORWARD)
			{
				endnode = node;
				endnode2 = nextn;
			}
			else
			{
				endnode = nextn;
				endnode2 = node;
			}

			ReconstructPath2(pj, endnode, endnode2);
			return true;
		}
		//if(!nextn->closed && (!nextn->opened || newd < nextn->rund))
#ifndef GPATHFLAGS
		else if(!(nextn->flags&closed) && (!(nextn->flags&opened) || newd < nextn->rund))
#else
		else if(!(flags&closed) && (!(flags&opened) || newd < nextn->rund))
#endif
		{
			//nextn->rund = newd;

			//todo macros VEC2_SUB etc
			Vec2i noff[2];
			noff[0].x = nextnpos.x - pj->goalx;
			noff[0].y = nextnpos.y - pj->goaly;
			noff[1].x = nextnpos.x - pj->nstartx;
			noff[1].y = nextnpos.y - pj->nstarty;
				
			//todo <<1
			const int32_t H = PATHHEUR( noff[dir>>1] ) << 1;
			const int32_t newcost = newd + H;

			//const int32_t ni = node - g_pathnode;

			HeapKey newhk;
			newhk.cost = newcost;
#ifdef HEAPKEYAGE	//tood test if works
			newhk.age = g_pathage;
#endif

			//D2=3 and D=2 for diagonal distance heuristic so sometimes it gives twice higher H even though newd is 1 lower
			//if((nextn->flags&opened) && nextn->heapkey.cost <= newcost)
#ifndef GPATHFLAGS
			if((nextn->flags&opened) /* && !HEAPCOMPARE( HEAPKEY(nextn), &newhk ) */ )
#else
			//if((g_pathflags.on(PATHNODE_FLAGBITS * node->index + PATHNODE_OPENED + dir)) && !HEAPCOMPARE( HEAPKEY(nextn), &newhk ) )
			if((g_pathflags.on(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED + dir)) && !HEAPCOMPARE( HEAPKEY(nextn), &newhk ) )
#endif
			{
				//char m[123];
				//sprintf(m, "new> nextn->rund=%d nextn->heapkey.cost=%d newd=%d H=%d",
				//	(int)nextn->rund, (int)nextn->heapkey.cost, (int)newd, (int)H);
				//InfoMess(m,m);
				continue;
			}

			//check pass
#if 1
			//if(!(pass&(1<<i)))
			if(!(checked&(1<<i)))
			{
				//if(block&(1<<i))
				//	continue;
				//if(!(pass&(1<<i)))
				//	continue;

				checked |= (1<<i);

				//check pass
				pass |= Standable(nextnpos.x, nextnpos.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i;
				//block &= ~(pass&(1<<i));

				if(i%2 == 1)	//diagonal?
				{
					uint8_t i2 = (i+(DIRS-1))&(DIRS-1);
					uint8_t i3 = (i+1)&(DIRS-1);
		
	#if 0
					//if((checked&(1<<i2)) && !(pass&())
					if(checked&(1<<i2))
					{
						if(!(pass&(1<<i2)))
						{
							//continue;
							//goto updiag;
							pass &= ~(1<<i);
							continue;
						}
					}
					//else if(!(pass&(1<<i2)))
					else
	#endif
					if(!(checked&(1<<i2)))
					{
						checked |= (1<<i2);
						//pass = pass && Standable(npos.x + OFFSETS[i2].x, npos.y + OFFSETS[i2].y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
						pass |= Standable(npos.x + DIRX(i2), npos.y + DIRY(i2), nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i2;
						//block &= ~(pass&(1<<i2));
					}

	#if 0
					//if(block&(1<<i3))
					if(checked&(1<<i3))
					{ 
						if(!(pass&(1<<i3)))
						{
							//continue;
							//goto updiag;
							pass &= ~(1<<i);
							continue;
						}
					}
					//else if(!(pass&(1<<i3)))
					else
	#endif
					if(!(checked&(1<<i3)))
					{
						checked |= (1<<i3);
						//pass = pass && Standable(npos.x + OFFSETS[i2].x, npos.y + OFFSETS[i2].y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
						pass |= Standable(npos.x + DIRX(i3), npos.y + DIRY(i3), nsizex, nsizey, sizex, pathoff, igb, igu, roaded) << i3;
						//block &= ~(pass&(1<<i3));
					}

				//updiag:
					//block |= ( (block&(1<<i)) || (block&(1<<i2)) || (block&(1<<i3)) ) << i;
					pass &= (( (pass&(1<<i2)) && (pass&(1<<i3)) ) << i) | (~(1<<i));
				}

				//if(!(pass&(1<<1)))
				//	continue;

				//if(block&(1<<i))
				//	continue;
			}
		
			if(!(pass&(1<<i)))
				continue;
			//use 4 if's
#endif

			nextn->rund = newd;
			//nextn->heapkey.cost = newcost; 
			//nextn->heapkey.age = g_pathage--;
			HeapKey oldhk = nextn->heapkey;
			nextn->heapkey = newhk;
			nextn->prev = node;

			//--g_pathage;

			//if( !nextn->opened )
#ifndef GPATHFLAGS
			if( !(nextn->flags&opened) )
#else
			if( !(flags&opened) )
#endif
			{
				openlist->add(nextn);
				//nextn->opened = true;
#ifndef GPATHFLAGS
				nextn->flags |= opened;
#else
				//g_pathflags.set(PATHNODE_FLAGBITS * node->index + PATHNODE_OPENED + dir);
				g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED + dir);
#endif
				g_toclear.push_back(nextn); // Records this node to reset its properties later.
			}
			else
				openlist->heapify(nextn, &oldhk);
		}
	}
#endif

	return false;
}
#endif