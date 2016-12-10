











#include "pathnode.h"
#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../render/transaction.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../algo/binheap.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "tilepath.h"
#include "fillbodies.h"
#include "../sim/simflow.h"
#include "partialpath.h"

//not engine
#include "../gui/layouts/chattext.h"

uint32_t pathnum = 0;


void CleanPath(std::list<TileNode*>& toclear)
{
	for(std::list<TileNode*>::iterator niter = toclear.begin(); niter != toclear.end(); niter++)
	{
		TileNode* n = *niter;
		n->opened = false;
		n->closed = false;
		n->prev = NULL;
		//n->tregs.clear();
	}

	toclear.clear();
}

bool CompareTiles(void* a, void* b)
{
	return ((TileNode*)a)->cost > ((TileNode*)b)->cost;
}

Vec2s TileNodePos(TileNode* node)
{
	if(!node)
		return Vec2s(-1,-1);

	const int32_t i = node - g_tilenode;
	const int32_t ny = i / g_mapsz.x;
	const int32_t nx = i % g_mapsz.x;
	return Vec2s(nx, ny);
}

void UpdJams()
{
	//TODO too heavy on the CPU
	//return;

	//only decrement jam every minute
	if(g_simframe % UPD_JAMS_DELAY != 0)
		return;

	for(int16_t y=0; y<g_mapsz.y; y++)
		for(int16_t x=0; x<g_mapsz.x; x++)
		{
			int16_t index = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[index];
			//for(unsigned char d=0; d<SDIRS; ++d)
			{
				if(tn->jams <= 0)
					continue;
				tn->jams --;
			}
		}
}

void Jamify(Unit* u)
{	
	int16_t tin = (u->cmpos.x/TILE_SIZE) + (u->cmpos.y/TILE_SIZE)*g_mapsz.x;
	TileNode* tn = &g_tilenode[tin];
	//for(unsigned char d=0; d<SDIRS; d++)
	tn->jams = imin(tn->jams + 5, MAX_JAM_VAL);
	UType* ut = &g_utype[u->type];
	//tn->jams = imax(tn->jams, MAX_JAM_VAL - ut->size.x);

	if(u->tpath.size())
	{
		Vec2s nextt = *u->tpath.begin();
		tin = nextt.x + nextt.y*g_mapsz.x;
		tn = &g_tilenode[tin];
		tn->jams = imin(tn->jams + 6, MAX_JAM_VAL);
	}
}

/*
TilePath + sub-pathing (hierarchical pathfinding)
won't work if you have to end up in certain tile
regions. If you automatically get access to a tile
region by being in the current tile,
it will work. So if you have a sub-tile-level
zig-zagging maze, this won't work.
If you have forests that might separate
a tile into two, this will work.
*/

void TilePath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype, signed char cdtype, int32_t supplier,
			  std::list<Vec2s> *tpath, Unit* thisu, Unit* targu, Building* targb,
			  int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
			  int32_t maxsearch, bool ignorejams, bool verify)
{
	UType* ut = &g_utype[utype];

#if 0
	if(thisu)
	{
		RichText rt("pathf");
		NewTransx(thisu->cmpos, &rt);
	}
#endif

	if(umode == UMODE_GODRIVE)
		ignorejams = true;

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

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->tpath = tpath;
	//pj->path = path;
	//pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->targu = targu ? targu - g_unit : -1;
	pj->targb = targb ? targb - g_building : -1;
	pj->goalx = cmgoalx / TILE_SIZE;
	pj->goaly = cmgoaly / TILE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath; //call TODO
	pj->pjtype = PATHJOB_TILE;
	pj->maxsearch = maxsearch;
	pj->nminx = 0;
	pj->nminy = 0;
	pj->nmaxx = g_pathdim.x-1;
	pj->nmaxy = g_pathdim.y-1;

	pj->tpath->clear();

#if 0
	int32_t nx = cmstartx / PATHNODE_SIZE;
	int32_t ny = cmstarty / PATHNODE_SIZE;
	int32_t nin = nx + ny * g_pathdim.x;
	unsigned char treg = g_tregs[nin];
#endif

	int32_t maxallowjam = 0;
	int32_t nextmax = 0;

	verify = false;
	//ignorejams = true;

	struct PathException
	{
		TileNode* tfrom;
		TileNode* tto;
		//Vec2s tfrom;
		//Vec2s tto;	//TODO use unsigned char todir;
	};

	std::list<PathException> avoidlist;

	int32_t searchdepth = 0;

	//do
	while(true)
	{
		//BinHeap openlist(CompareTiles);
		BinHeap openlist;
		std::list<TileNode*> toclear;

		int32_t tx = cmstartx / TILE_SIZE;
		int32_t ty = cmstarty / TILE_SIZE;

		int32_t tin = tx + ty * g_mapsz.x;
		TileNode* tnode = &g_tilenode[tin];
		tnode->opened = true;
		tnode->rund = 0;
		tnode->cost = 1;
		tnode->prev = NULL;
		//tnode->tregs.push_back(treg);
		//tnode->arrivedcm.x = cmstartx;
		//tnode->arrivedcm.y = cmstarty;
		openlist.add(tnode);
		toclear.push_back(tnode);

#ifdef HIERDEBUG
		pathnum ++;

		//if(pathnum == 73)
		if(thisu - g_unit == 19)
		{
			Log("the 13th unit:");
			g_speed = SPEED_PAUSE;
			Unit* u = thisu;
			//g_cam.move( u->drawpos - g_cam.m_view );
			g_zoom = MAX_ZOOM;
		}

		Log("tpath----"<<" #"<<pathnum);
#endif

		while(openlist.hasmore())
		{
			++searchdepth;

			tnode = (TileNode*)openlist.delmin();

			tnode->closed = true;
			int32_t rund = tnode->rund;

			Vec2s tpos = TileNodePos(tnode);

			//If at goal...
			int32_t cmminx = tpos.x * TILE_SIZE;
			int32_t cmminy = tpos.y * TILE_SIZE;
			int32_t cmmaxx = (tpos.x+1) * TILE_SIZE - 1;
			int32_t cmmaxy = (tpos.y+1) * TILE_SIZE - 1;

#if 0
			std::list<Vec2i> subpath;
			Vec2i subsubgoal;

			MazePath(utype, umode, tnode->arrivedcm.x, tnode->arrivedcm.y,
				target, target2, targtype, cdtype,
				&subpath, &subsubgoal, thisu, targu, targb,
				pj->cmgoal.x, pj->cmgoal.y,
				pj->goalminx, pj->goalminy, pj->goalmaxx, pj->goalmaxy,
				TILE_SIZE*TILE_SIZE/PATHNODE_SIZE/PATHNODE_SIZE*2,
				true, false);
#endif

#if 1
			if(cmminx <= pj->goalmaxx &&
				cmminy <= pj->goalmaxy &&
				cmmaxx >= pj->goalminx &&
				cmmaxy >= pj->goalminy)
#else
			if(subpath.size())
#endif
			{
				//subpath.clear();

				//Return path


				//Don't include very first tile because we're already in it.
				//But what if the tpath size is only 1 tile?
				//for(TileNode* n = tnode; n->prev; n = n->prev)
				//Edit: Now just check if we're in the last tpath node,
				//then full-path to goal. Or if we start in first tpath node,
				//then pop it and path to next.
				for(TileNode* n = tnode; n; n = n->prev)
					//for(TileNode* n = tnode; n->prev || (n && pj->tpath->size() == 0); n = n->prev)
				{
					tpos = TileNodePos(n);
					pj->tpath->push_front(tpos);
				}


				//cap end? necessary? corpd fix xp
				tpos = Vec2s(pj->goalx,pj->goaly);
				pj->tpath->push_back(tpos);

#if 11
				if(!verify)
					goto cleanup;

				Vec2i arrivedcm;
				arrivedcm.x = cmstartx;
				arrivedcm.y = cmstarty;
				
				//verify path
				for(std::list<Vec2s>::iterator n=pj->tpath->begin(); n!=pj->tpath->end(); n++)
				{
					std::list<Vec2s>::iterator n2 = n;
					n2 ++;

					//reached the end, path verified
					if(n2 == pj->tpath->end())
					{
						//final check to make sure we end up at exact cm goal

						std::list<Vec2i> subpath;
						Vec2i subsubgoal;

						PartialPath(utype, umode, arrivedcm.x, arrivedcm.y,
							target, target2, targtype, cdtype, supplier,
							&subpath, &subsubgoal, thisu, targu, targb,
							pj->cmgoal.x, pj->cmgoal.y,
							pj->goalminx, pj->goalminy, pj->goalmaxx, pj->goalmaxy,
							TILE_SIZE*TILE_SIZE/PATHNODE_SIZE/PATHNODE_SIZE*2,
							false, false, true);

						//final step doesn't work, no path
						if(!subpath.size())
						{
							pj->tpath->clear();

							goto cleanup;
						}

						//return with path
						goto cleanup;
					}

					std::list<Vec2i> subpath;
					Vec2i subsubgoal;

					PartialPath(utype, umode, arrivedcm.x, arrivedcm.y,
						target, target2, targtype, cdtype, supplier,
						&subpath, &subsubgoal, thisu, targu, targb,
						n2->x*TILE_SIZE+TILE_SIZE/2, n2->y*TILE_SIZE+TILE_SIZE/2,
						n2->x * TILE_SIZE, n2->y * TILE_SIZE, (n2->x+1)*TILE_SIZE-1, (n2->y+1)*TILE_SIZE-1,
						TILE_SIZE*TILE_SIZE/PATHNODE_SIZE/PATHNODE_SIZE*2,
						false, false, false);
				
					//if move not possible, add exception and try pathing over again
					if(!subpath.size())
					{
						PathException pe;
						//pe.tfrom = *n;
						//pe.tto = *n2;
						pe.tfrom = TileNodeAt(n->x, n->y);
						pe.tto = TileNodeAt(n2->x, n2->y);
						avoidlist.push_back(pe);
						
#if 0
						char m[123];
						sprintf(m, "add f%d thisu=%d A %d,%d->%d,%d", (int32_t)g_simframe, (int32_t)pj->thisu, (int32_t)n->x, (int32_t)n->y, (int32_t)n2->x, (int32_t)n2->y);
						//InfoMess(m,m);
						g_applog<<m<<std::endl;
						g_applog.flush();
#endif
						
						if(*n == *n2)
						{
#if 0
							int32_t k=0;
							for(TileNode* pn = tnode; pn; pn = pn->prev, k++)
								//for(TileNode* n = tnode; n->prev || (n && pj->tpath->size() == 0); n = n->prev)
							{
								tpos = TileNodePos(pn);
								//pj->tpath->push_front(tpos);
								char m[123];
								sprintf(m, "k=%d tpos=%d,%d", (int32_t)k, (int32_t)tpos.x, (int32_t)tpos.y);
								InfoMess(m,m);
							}
							
								char m[123];
								tpos = Vec2s(pj->goalx,pj->goaly);
								sprintf(m, "cap k=%d tpos=%d,%d", (int32_t)-1, (int32_t)tpos.x, (int32_t)tpos.y);
								InfoMess(m,m);
#endif
						}

						CleanPath(toclear);
						toclear.clear();

#if 1
						//no path? same tile
						if(*n == *n2)
						{
							goto cleanup;
						}
#endif

						openlist.free();

						//plant starting tnode again
						int32_t tin = tx + ty * g_mapsz.x;
						TileNode* tnode = &g_tilenode[tin];
						tnode->opened = true;
						tnode->rund = 0;
						tnode->cost = 1;
						tnode->prev = NULL;
						//tnode->tregs.push_back(treg);
						//tnode->arrivedcm.x = cmstartx;
						//tnode->arrivedcm.y = cmstarty;
						openlist.add(tnode);
						toclear.push_back(tnode);

						goto tryagain;
					}

					arrivedcm = *subpath.rbegin();
				}
#endif

				break;
			}

			if(searchdepth >= maxsearch)
				goto cleanup;

			for(unsigned char i=0; i<SDIRS; i++)
			{

				if(tpos.x + STRAIGHTOFF[i].x < 0)
					continue;

				if(tpos.x + STRAIGHTOFF[i].x >= g_mapsz.x)
					continue;

				if(tpos.y + STRAIGHTOFF[i].y < 0)
					continue;

				if(tpos.y + STRAIGHTOFF[i].y >= g_mapsz.y)
					continue;

				Vec2s tpos2(tpos.x + STRAIGHTOFF[i].x, tpos.y + STRAIGHTOFF[i].y);

				//pass[i] = Standable(pj, tpos.x + STRAIGHTOFF[i].x * TILE_SIZE/PATHNODE_SIZE, tpos.y + STRAIGHTOFF[i].y * TILE_SIZE/PATHNODE_SIZE);
				if(!TileStandable(pj, (tpos2.x * TILE_SIZE + TILE_SIZE/2)/PATHNODE_SIZE, (tpos2.y * TILE_SIZE + TILE_SIZE/2)/PATHNODE_SIZE))
					continue;

				//tin = tpos2.x + tpos2.y * g_mapsz.x;
				//TileNode* tnode2 = &g_tilenode[ tin ];
				TileNode* tnode2 = TileNodeAt(tpos2.x, tpos2.y);

#if 0
				std::list<Vec2i> subpath;
				Vec2i subsubgoal;

				MazePath(utype, umode, tnode->arrivedcm.x, tnode->arrivedcm.y,
					target, target2, targtype, cdtype,
					&subpath, &subsubgoal, thisu, targu, targb,
					tpos2.x*TILE_SIZE+TILE_SIZE/2, tpos2.y*TILE_SIZE+TILE_SIZE/2,
					tpos2.x * TILE_SIZE, tpos2.y * TILE_SIZE, (tpos2.x+1)*TILE_SIZE-1, (tpos2.y+1)*TILE_SIZE-1,
					TILE_SIZE*TILE_SIZE/PATHNODE_SIZE/PATHNODE_SIZE*2,
					true, false);

				if(!subpath.size())
					continue;
#endif

				if(verify)
				{
					bool avoid = false;

					//check for exceptions...
					for(std::list<PathException>::iterator pe=avoidlist.begin(); pe!=avoidlist.end(); pe++)
					{
						//if(pe->tfrom.x == tpos.x && pe->tfrom.y == tpos.y &&
						//	pe->tto.x == tpos2.x && pe->tto.y == tpos2.y)
						if(pe->tfrom == tnode && pe->tto == tnode2)
						{
							avoid = true;
							break;
						}
					}

					if(avoid)
					{
#if 0
						char m[123];
						sprintf(m, "thisu=%d A %d,%d->%d,%d", (int32_t)pj->thisu, (int32_t)tpos.x, (int32_t)tpos.y, (int32_t)tpos2.x, (int32_t)tpos2.y);
						//InfoMess(m,m);
						g_applog<<m<<std::endl;
						g_applog.flush();
#endif
						continue;
					}
				}

				//tnode2->arrivedcm = *subpath.rbegin();
				//subpath.clear();

				if(!ignorejams && tnode->jams && tnode2->jams)
					continue;

#if 1
#endif	//TODO rewrite to get rid of "regions" attempt

				//For each region we've accessed on this tile,
				//check if it leads to this other tile.

				//Vec2s tpos2(tpos.x + STRAIGHTOFF[i].x, tpos.y + STRAIGHTOFF[i].y);
				//int32_t tin2 = (tpos2.x) + (tpos2.y) * g_mapsz.x;

				//bool visited = false;
				//bool found = false;

				if(tnode2->closed)
				{
#if 0
					//Visited and expanded this tile.

					int32_t newd = rund + 2;

					if(newd > tnode2->rund)
						continue;

					found = true;
					//toclear.push_back(tnode2);
					tnode2->prev = tnode;
					int32_t H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) * 2;
					//if(pj->roaded)
					//H += tnode2->jams;
					//else if(pj->utype == UNIT_LABOURER)
					//	H += tnode2->jams / 16;
					tnode2->heapkey.cost = rund + 2 + H;
					tnode2->rund = newd;
					tnode2->closed = false;
					tnode2->opened = true;
					//tnode2->tregs.push_back(rit2->from);
					//openlist.heapify(tnode2);
					//tnode2->arrivedcm = *subpath.rbegin();
					openlist.add(tnode2);
#endif
				}
				//If we are already waiting for this tile to queue up...
				else if(tnode2->opened)
				{
#ifdef HIERDEBUG
					Log("\t7");
#endif

					int32_t newd = rund + 2;

					if(newd > tnode2->rund)
						continue;

#ifdef HIERDEBUG
					Log("\t9");
#endif

					//Else, not visited this region.
					//found = true;
					//toclear.push_back(tnode2);
					tnode2->prev = tnode;
					int32_t H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) * 2;
					//if(pj->roaded)
					//H += tnode2->jams;
					//else if(pj->utype == UNIT_LABOURER)
					//	H += tnode2->jams / 16;
					tnode2->cost = newd + H;
					tnode2->rund = newd;
					tnode2->closed = false;
					tnode2->opened = true;
					//tnode2->arrivedcm = *subpath.rbegin();
					//tnode2->tregs.push_back(rit2->from);
					openlist.heapify(tnode2, NULL);
				}
				//not opened or closed
				else
					//else if(tnode2->jams == 0)
				{
#ifdef HIERDEBUG
					Log("\t11");
#endif

					//Not visited this tile at all.
					//found = true;
					toclear.push_back(tnode2);
					tnode2->prev = tnode;
					int32_t H = PATHHEUR( Vec2i(tpos2.x - pj->goalx, tpos2.y - pj->goaly) ) * 2;
					//if(pj->roaded)
					//H += tnode2->jams;
					//else if(pj->utype == UNIT_LABOURER)
					//	H += tnode2->jams / 16;
					tnode2->cost = rund + 2 + H;
					tnode2->rund = rund + 2;
					tnode2->closed = false;
					tnode2->opened = true;
					//tnode2->arrivedcm = *subpath.rbegin();
					//tnode2->tregs.push_back(rit2->from);
					openlist.add(tnode2);
				}

				//Already found path from desired region, stop looking.
				//if(found)
				//	break;
				//Edit: no, there might be other regions in this tile.

			}
		}

cleanup:

#if 0
		static int32_t fails = 0;

		if(pj->tpath->size() == 0)
		{
			fails++;
			g_applog<<"frame "<<g_simframe<<" fails "<<fails<<std::endl;
			g_applog.flush();
		}
#endif

		CleanPath(toclear);
		delete pj;
		break;

tryagain:
		CleanPath(toclear);
	}
	//while();
}

void Expand_T(PathJob* pj, PathNode* node)
{
//#if 0	fixme	//allowed in msvc2012
#if 0	//fixme
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

#if 0
	bool stand[DIRS];

	for(unsigned char i=0; i<DIRS; i++)
		stand[i] = Standable(pj, npos.x + OFFSETS[i].x, npos.y + OFFSETS[i].y);

	bool pass[DIRS];

	pass[DIR_NW] = stand[DIR_NW] && stand[DIR_N] && stand[DIR_W];
	pass[DIR_N] = stand[DIR_N];
	pass[DIR_NE] = stand[DIR_NE] && stand[DIR_N] && stand[DIR_E];
	pass[DIR_E] = stand[DIR_E];
	pass[DIR_SE] = stand[DIR_SE] && stand[DIR_S] && stand[DIR_E];
	pass[DIR_S] = stand[DIR_S];
	pass[DIR_SW] = stand[DIR_SW] && stand[DIR_S] && stand[DIR_W];
	pass[DIR_W] = stand[DIR_W];

	for(unsigned char i=0; i<DIRS; i++)
	{
#else
	bool pass[SDIRS];

	for(unsigned char i=0; i<SDIRS; i++)
		//pass[i] = Standable(pj, npos.x + STRAIGHTOFF[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFF[i].y * TILE_SIZE/PATHNODE_SIZE);
		pass[i] = TileStandable(pj, npos.x + STRAIGHTOFF[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFF[i].y * TILE_SIZE/PATHNODE_SIZE);

	for(unsigned char i=0; i<SDIRS; i++)
	{
#endif
		if(!pass[i])
			continue;

		//int32_t newd = rund + STEPDIST[i];
		int32_t newd = rund + 2;

		Vec2i nextnpos(npos.x + STRAIGHTOFF[i].x * TILE_SIZE/PATHNODE_SIZE, npos.y + STRAIGHTOFF[i].y * TILE_SIZE/PATHNODE_SIZE);
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
				g_openlist.heapify(nextn);
		}
	}
#endif
}
