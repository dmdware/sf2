













#include "transport.h"
#include "building.h"
#include "unit.h"
#include "bltype.h"
#include "mvtype.h"
#include "conduit.h"
#include "resources.h"
#include "player.h"
#include "job.h"
#include "../render/transaction.h"
#include "../ai/ai.h"
#include "simdef.h"
#include "../econ/utility.h"
#include "../path/jpspath.h"
#include "../path/tilepath.h"
#include "../path/astarpath.h"
#include "../path/partialpath.h"
#include "../path/pathnode.h"
#include "../platform.h"
#include "truck.h"
#include "../sound/sound.h"
#include "../path/pathjob.h"
#include "../path/anypath.h"
#include "../path/fillbodies.h"
#include "../gui/layouts/chattext.h"
#include "simflow.h"
#include "../gui/layouts/messbox.h"

static std::list<int> truckpathd;

//#define TRANSPORT_DEBUG
//optimized pathing (checks sorted list before prepathing),
//less accurate dist (doesn't traverse the path, but uses MAG_VEC3F() or Manhattan()).
//this is now the default, disabling this define leaves it broken. will clean it up.
#define TBID2

//i would base the utility of a truck as a transporter for a given job based on a function of its distance from the supplier and the distance of
//the supplier to the demander, and the charge of the transport owner, but especially in the early building phase of the game,
//there might be disconnected pockets of the economy that might be inaccessible to each other, or even road tiles that might not be accessable
//because the road leading up to them hasn't been finished, meaning a truck might be repeatedly be given a transport job it can't fulfill.
//so i'm using the complicated, bidding and pathfinding-to-everything method i used in the prev version.

#if 0
int BestSup(Vec2i demcmpos, int rtype, int* retutil, Vec2i* retcmpos)
{
	int bestsup = -1;
	int bestutil = -1;
	Resource* r = &g_resource[rtype];

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];
		BlType* bt = &g_bltype[b->type];

		if(bt->output[rtype] <= 0)
			continue;

		Py* supp = &g_py[b->owner];

		if(supp->global[rtype] + b->stocked[rtype] <= 0)
			continue;

		//check if distance is better or if there's no best yet

		Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

		int dist = MAG_VEC3F(bcmpos - demcmpos);

		//if(dist > bestdist && bestdemb)
		//	continue;

		int margpr = b->price[rtype];

		int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

		if(util <= bestutil && bestsup >= 0)
			continue;

		bestsup = bi;
		bestutil = util;
		*retutil = util;
		*retcmpos = bcmpos;
	}

	return bestsup;
}
#endif

//warning: "arrive" must be set to starting position before calling this function, or else "dist" will be wrong.
void StepBeforeArrival(int mvtype, Vec2i* arrive, std::list<Vec2i>* path, int* dist, int cmgoalminx, int cmgoalminy, int cmgoalmaxx, int cmgoalmaxy)
{
	MvType* mvt = &g_mvtype[mvtype];

	for(std::list<Vec2i>::iterator pit=path->begin(); pit!=path->end(); pit++)
	{
#if 0
		int newuminx = pit->x - mvt->size.x/2;
		int newuminy = pit->y - mvt->size.x/2;
		int newumaxx = newuminx + mvt->size.x - 1;
		int newumaxy = newuminy + mvt->size.x - 1;

		if(newuminx <= cmgoalmaxx && newuminy <= cmgoalmaxy && newumaxx >= cmgoalminx && newumaxy >= cmgoalminy)
			return;

		*dist += MAG_VEC3F(*arrive - *pit);
		//only advance here once we know we're still out of potential collision area, so that next pathfind won't start in a solid
		*arrive = *pit;
#else
		//break up int moves into PATHNODE_SIZE steps
		Vec2i delta = *pit - *arrive;	//arrive is prev step here
		//assuming only diagonal or straight moves
		int len = imax(delta.x, delta.y);

		if(len <= 0)
			continue;

		delta = delta / len;
		Vec2i step = delta * PATHNODE_SIZE;
		int steps = len / PATHNODE_SIZE;

		for(int si=0; si<steps; si++)
		{
			Vec2i newpos = *arrive + step;

			int newuminx = newpos.x - mvt->size.x/2;
			int newuminy = newpos.y - mvt->size.x/2;
			int newumaxx = newuminx + mvt->size.x - 1;
			int newumaxy = newuminy + mvt->size.x - 1;

			if(newuminx <= cmgoalmaxx && newuminy <= cmgoalmaxy && newumaxx >= cmgoalminx && newumaxy >= cmgoalminy)
				return;

			Vec2i off = *arrive - newpos;
			*dist += MAG_VEC2I(off);
			*arrive = newpos;
		}
#endif
	}
}

// comparison, not case sensitive.
ecbool CompareJobs(const TransportJob& a, const TransportJob& b)
{
	return a.jobutil > b.jobutil;
}

// bid for transportation of resources to X
void TBid(int target, int target2, int targtype, int umode, int cdtype, int res, int amt)
{
#ifdef TRUCK_DEBUG
	Log("-------------------");
	Log("TBid("<<target<<", "<<target2<<", "<<targtype<<", "<<res<<", "<<amt<<");");
	
#endif

#ifdef TRANSPORT_DEBUG
	if(res != RES_FARMPRODUCTS)
		return;
#endif

	
#if 0
	char m[123];
	sprintf(m, "tb %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif

	Vec2i dempos;
	int demcmminx;	//"demander centimeter minimum x coordinate"
	int demcmminy;
	int demcmmaxx;
	int demcmmaxy;
	int demplayer = -1;
	MvType* mvt = &g_mvtype[MV_TRUCK];
	Bl* demb = NULL;

	//if(amt >0 && res == RES_CEMENT)
		//InfoMess("req c", "hreq c2");

	std::list<short>* roadnet = NULL;
	std::list<short> roadnet2;

	if(targtype == TARG_BL)
	{
		//char msg[128];
		//sprintf(msg, "bid %s", g_bltype[g_bl[target].type].name);
		//Chat(msg);
		demb = &g_bl[target];
		roadnet = &demb->roadnetw;
		demplayer = demb->owner;
		dempos = demb->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		BlType* bt = &g_bltype[demb->type];
		int demtminx = demb->tpos.x - bt->width.x/2;
		int demtminy = demb->tpos.y - bt->width.y/2;
#if 1
		demcmminx = demtminx * TILE_SIZE;
		demcmminy = demtminy * TILE_SIZE;
		//"pixel perfect" centimeter coordinates.
		//we subtract 1 because we start count at 0. this is important.
		//you can use <= for collision checks this way instead of <,
		//which wouldn't detect objects that are overlapping by 1 cm.
		//so an object of width 10 has the range [0,9].
		//0 + 10 - 1 = 9
		//the next object will occupy [10,19].
		demcmmaxx = demcmminx + bt->width.x * TILE_SIZE - 1;
		demcmmaxy = demcmminy + bt->width.y * TILE_SIZE - 1;
#else
		demcmminx = dempos.x;
		demcmminy = dempos.y;
		demcmmaxx = dempos.x;
		demcmmaxy = dempos.y;
#endif

#ifdef TRUCK_DEBUG
		Log("demander b = "<<g_bltype[g_bl[target].type].name);
		
#endif
	}
	else if(targtype == TARG_CD)
	{
		//Chat("bid road");
		CdTile* ctile = GetCd(cdtype, target, target2, ecfalse);
		demplayer = ctile->owner;
		CdType* ctype = &g_cdtype[cdtype];
		dempos = Vec2i(target,target2)*TILE_SIZE + ctype->physoff;
		demcmminx = dempos.x - TILE_SIZE/2;
		demcmminy = dempos.y - TILE_SIZE/2;
		demcmmaxx = demcmminx + TILE_SIZE - 1;
		demcmmaxy = demcmminy + TILE_SIZE - 1;
		
		//roadnet2.push_back(ctile->netw);
		roadnet = &roadnet2;

		Vec2i roadpos[4];
		roadpos[0] = Vec2i(demcmminx,demcmminy);
		roadpos[1] = Vec2i(demcmmaxx,demcmminy);
		roadpos[2] = Vec2i(demcmminx,demcmmaxy);
		roadpos[3] = Vec2i(demcmmaxx,demcmmaxy);

		for(unsigned char i=0; i<4; ++i)
		{
			CdTile* rtile = GetCd(CD_ROAD, roadpos[i].x/TILE_SIZE, roadpos[i].y/TILE_SIZE, ecfalse);
			if(rtile->finished)
				roadnet2.push_back(rtile->netw);
		}
	}

	int dempi = demplayer;
	Py* demp = &g_py[dempi];

	int imsti = (int)( dempi / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
	Py* imst = &g_py[imsti];

	int exsti;
	Py* exst;

	// no available sources/paths?

#if 0
	ecbool found = ecfalse;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[res] <= 0)
			continue;

		Py* bp = &g_py[b->owner];

		if(b->stocked[res] + bp->global[res] <= 0)
			continue;

		found = ectrue;
	}

	if(!found)
		return;
#elif 0
	int suputil;
	Vec2i suppos;
	bestsup = BestSup(dempos, res, &suputil, &suppos);

	if(bestsup < 0)
		return;
#endif

	int bestsup = -1;
	int bestutil = -1;
	int bestunit = -1;
	Vec2i bestsuppos;

	std::list<TransportJob> jobs;

	//try each truck
	for(int ui=0; ui<MOVERS; ui++)
	{
		Mv* mv = &g_mv[ui];

		if(!mv->on)
			continue;

		if(mv->type != MV_TRUCK)
			continue;

		if(mv->mode != UMODE_NONE)
			continue;

		if(mv->jobframes < TBID_DELAY)
			continue;

		CdTile* rtile = GetCd(CD_ROAD, mv->cmpos.x/TILE_SIZE, mv->cmpos.y/TILE_SIZE, ecfalse);

		ecbool samenet = ecfalse;

		for(std::list<short>::iterator rnet=roadnet->begin(); rnet!=roadnet->end(); rnet++)
		{
			if(*rnet == rtile->netw)
			{
				samenet = ectrue;
				break;
			}
		}

		if(!samenet)
			continue;

		truckpathd.push_back(ui);

		//if(amt >0 && res == RES_CEMENT)
		//	InfoMess("req c", "hreq c3");

		if(Trapped(u, NULL))
		{
			Jamify(u);

			continue;
		}

		Py* up = &g_py[mv->owner];

		//for each truck, try each supplier too
		int subbestsup = -1;	//sub best supplier: used to evalute the best supplier given this truck (and the demander of course), so as to assign the best supplier and truck combo
		int subbestutil = -1;	//used to figure out the supplier with best util
		Vec2i subbestsuppos;
		Resource* r = &g_resource[res];

		//...with each supplier
		for(int bi=0; bi<BUILDINGS; bi++)
		{
			Bl* potsupb = &g_bl[bi];	//potential supplier building

			if(!potsupb->on)//how could I have missed this?
				continue;

			if(!potsupb->finished)
				continue;

			BlType* bt = &g_bltype[potsupb->type];

			if(bt->output[res] <= 0)
				continue;

			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("req c", "hreq c1");

			Py* supp = &g_py[potsupb->owner];

			if(supp->global[res] + potsupb->stocked[res] <= 0)
				continue;

			//check if distance is better or if there's no best yet

			Vec2i supcmpos = potsupb->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;


			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("req c", "hreq c");

#if 0
			//warning: possibility of int overflow on large maps, definitly on 255x255 tiles, using MAG_VEC3F()
			int dist = MAG_VEC3F(supcmpos - dempos);

			//if(dist > bestdist && bestdemb)
			//	continue;

			int margpr = potsupb->price[res];

			int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

			if(util <= subbestutil && subbestsup >= 0)
				continue;
#endif

			int supcmminx = (potsupb->tpos.x - bt->width.x/2) * TILE_SIZE;
			int supcmminy = (potsupb->tpos.y - bt->width.y/2) * TILE_SIZE;	//TO DO: change all the z's to y's and vice versa, for Id coordinate system, which makes more sense for top-down RTS/strategy games
			int supcmmaxx = supcmminx + bt->width.x * TILE_SIZE - 1;
			int supcmmaxy = supcmminy + bt->width.y * TILE_SIZE - 1;

#ifndef TBID2
			//now check if a path is available from the transporter to this supplier - a very expensive op
			std::list<Vec2i> path;
			Vec2i subgoal;

#if 0
			JPSPath(mv->type, UMODE_GOSUP, mv->cmpos.x, mv->cmpos.y, bi, -1, UMODE_GOSUP, &path, &subgoal, u, NULL, potsupb, supcmpos.x, supcmpos.y, supcmminx, supcmminy, supcmmaxx, supcmmaxy);
#elif 0
			AStarPath(
				        mv->type, UMODE_GOSUP,
				        mv->cmpos.x, mv->cmpos.y, target, target2, TARG_BL, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y, SHRT_MAX-1);
#elif 1
			JPSPath(
				        mv->type, UMODE_GOSUP,
				        mv->cmpos.x, mv->cmpos.y, -1, -1, TARG_BL, cdtype, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y,
						0, 0, g_pathdim.x-1, g_pathdim.y-1);
#else

			JPSPath(
				        MV_LABOURER, UMODE_GOSUP,
				        supcmpos.x-100, supcmpos.y-100, target, target2, TARG_BL, &path, &subgoal,
				        u, NULL, potsupb,
				        supcmpos.x, supcmpos.y,
				        supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y);
#endif
			if(path.size() == 0)
			{
#ifdef TRANSPORT_DEBUG
				{
					char msg[1280];
					sprintf(msg, "no path 1 sup%d ", (int)(potsupb-g_bl));
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				continue;	//no path
			}

			//continue;

			int trucktosup = 0;
			Vec2i arrive = mv->cmpos;

			//calculate distance from truck to supplier and determine arrival spot (the path given leads to the center of the supplier, but we stop at some point before).
			StepBeforeArrival(mv->type, &arrive, &path, &trucktosup, supcmminx, supcmminy, supcmmaxx, supcmmaxy);

			//now, given the position of the transporter at the supplier, find a path from there to the demander - another expensive op
			path.clear();

			JPSPath(mv->type, umode,
				arrive.x, arrive.y, target, target2, targtype, cdtype,
				&path, &subgoal,
				u, NULL, demb,
				dempos.x, dempos.y,
				demcmminx, demcmminy, demcmmaxx, demcmmaxy,
				0, 0, g_pathdim.x-1, g_pathdim.y-1);
			//JPSPath(mv->type, umode, mv->cmpos.x, mv->cmpos.y, target, target2, targtype, &path, &subgoal, u, NULL, demb, dempos.x, dempos.y, demcmminx, demcmminy, demcmmaxx, demcmmaxy);

			//calculate distance from supplier arrival position to supplier and determine arrival spot (the path given leads to the center of the supplier, but we stop at some point before).
			int suptodem = 0;
			StepBeforeArrival(mv->type, &arrive, &path, &suptodem, demcmminx, demcmminy, demcmmaxx, demcmmaxy);

			if(path.size() == 0)
			{
#ifdef TRANSPORT_DEBUG
				{
					char msg[128];
					sprintf(msg, "no path 2 dembi%d", demb ? (int)(demb-g_bl) : -1);
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				continue;	//no path
			}

#if 1
			//warning: possibility of int overflow on large maps, definitly on 255x255 tiles, using MAG_VEC3F()
			int dist = trucktosup + suptodem;

			int margpr = potsupb->price[res];

			int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

			if(util <= subbestutil && subbestsup >= 0)
				continue;
#endif

			subbestsup = bi;
			subbestutil = util;
			subbestsuppos = supcmpos;

#else
			const Vec2i supoff = mv->cmpos - supcmpos;
			const Vec2i demoff = supcmpos - dempos;
			int trucktosup = TRANHEUR(supoff);
			int suptodem = TRANHEUR(demoff);

			int dist = trucktosup + suptodem;

			//int margpr = potsupb->price[res];

			//int util = r->physical ? PhUtil(margpr, dist) : GlUtil(margpr);

			//protectionism prices

			int suppi = potsupb->owner;

			int exsti2 = (int)( suppi / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
			Py* exst2 = &g_py[exsti2];

			int initprice = potsupb->price[res];
			int effectprice = initprice;

			//trade between countries?
			if(exsti2 != imsti)
			{
				if(exst2->protectionism)
				{
					int extariffprice = initprice * exst2->extariffratio / RATIO_DENOM;
					effectprice += extariffprice;
				}

				if(imst->protectionism)
				{
					int imtariffprice = initprice * imst->imtariffratio / RATIO_DENOM;
					effectprice += imtariffprice;
				}
			}

			int util = r->physical ? PhUtil(effectprice, dist) : GlUtil(effectprice);

#if 0
			if(util <= subbestutil && subbestsup >= 0)
				continue;

			//don't prepath until the end, when we check off a sorted list
			if(!AnyPath(
				mv->type, UMODE_GOSUP,
				mv->cmpos.x, mv->cmpos.y, -1, -1, TARG_BL, cdtype,
				u, NULL, potsupb,
				supcmpos.x, supcmpos.y,
				supcmpos.x, supcmpos.y, supcmpos.x, supcmpos.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
				continue;

			if(!AnyPath(mv->type, umode,
				mv->cmpos.x, mv->cmpos.y, target, target2, targtype, cdtype,
				u, NULL, demb,
				dempos.x, dempos.y,
				demcmminx, demcmminy, demcmmaxx, demcmmaxy,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
				continue;
#endif

#if 0
			if(amt >0 && res == RES_CEMENT)
			{
				char msg[128];
				sprintf(msg, "c hu %d", util);
				InfoMess("req c", msg);
			}
#endif

			
#if 0
	char m[123];
	sprintf(m, "tb2 %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif

			jobs.push_back(TransportJob());
			TransportJob* j = &*jobs.rbegin();
			j->suputil = util;
			j->jobutil = util;
			j->supbi = bi;
			j->supcmpos = supcmpos;
			j->truckui = ui;
			j->potsupb = potsupb;
			//TO DO: add transportjob and later go through
			//list in sorted order, pathing to each.

			//TO DO: use binheap instead of std::list,
			//for this, for JobOpp, for other uses?

#if 0
			subbestsup = bi;
			subbestutil = util;
			subbestsuppos = supcmpos;
#endif

#endif
		}

#if 0
		//no best supplier? maybe there's no path between this truck and any supplier.
		if(subbestsup < 0)
			continue;

		if(subbestutil < bestutil)
			continue;

		bestutil = subbestutil;
		bestsup = subbestsup;
		bestunit = ui;
		bestsuppos = subbestsuppos;
#endif
	}

#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "success TBid?");
		RichText debugrt;
		debugrt.part.push_back(msg);
		AddNotif(&debugrt);
	}
#endif

#if 0
	// no transporters?
	if(bestunit < 0)
		return;

	if(bestsup < 0)
		return;
#endif

#ifdef TBID2

	if(jobs.size() <= 0)
		return;

#if 0
	if(amt >0 && res == RES_CEMENT)
	{
		char msg[128];
		sprintf(msg, "j %d", (int)jobs.size());
		InfoMess("req c", msg);
	}
#endif

	jobs.sort(CompareJobs);

#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "success TBid!");
		RichText debugrt;
		debugrt.part.push_back(msg);
		AddNotif(&debugrt);
	}
#endif


	for(std::list<TransportJob>::iterator jit=jobs.begin(); jit!=jobs.end(); jit++)
	{
		TransportJob* j = &*jit;

		if(j->jobutil <= 0)
		{
			
#if 0
	char m[123];
	sprintf(m, "tb2 ju %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif

			return;
		}

		//numpaths++;

		Mv* mv = &g_mv[j->truckui];
		//TODO

		//first pathable job

#if 1
#if 0
		if(!AnyPath(mv->type, j->jobtype, mv->cmpos.x, mv->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype,
			u, j->targu, j->targb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			nminx, nminy, nmaxx, nmaxy))
			continue;
#elif 0
		if(!AnyPath(
			mv->type, UMODE_GOSUP,
			mv->cmpos.x, mv->cmpos.y, -1, -1, TARG_BL, cdtype,
			u, NULL, j->potsupb,
			j->supcmpos.x, j->supcmpos.y,
			j->supcmpos.x, j->supcmpos.y, j->supcmpos.x, j->supcmpos.y,
			0, 0, g_pathdim.x-1, g_pathdim.y-1))
		{

			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("bl","bl");

			continue;
		}

		if(!AnyPath(mv->type, umode,
			mv->cmpos.x, mv->cmpos.y, target, target2, targtype, cdtype,
			u, NULL, demb,
			dempos.x, dempos.y,
			demcmminx, demcmminy, demcmmaxx, demcmmaxy,
			0, 0, g_pathdim.x-1, g_pathdim.y-1))
		{

			//if(amt >0 && res == RES_CEMENT)
			//	InfoMess("bl","bl2");

			continue;
		}
#else

#ifdef HIERPATH	//tile corners not admissable	//edit: ???
		//high level check first
		std::list<Vec2s> tpath;

		TilePath(
			mv->type, UMODE_GOSUP,
			mv->cmpos.x, mv->cmpos.y,
			target, target2, TARG_BL, cdtype, j->supbi,
			&tpath,
			u, NULL, j->potsupb,
			j->supcmpos.x, j->supcmpos.y,
			j->supcmpos.x, j->supcmpos.y, j->supcmpos.x, j->supcmpos.y,
			g_mapsz.x*g_mapsz.y);
			
		if(tpath.size() <= 0)
			continue;
		
		tpath.clear();

		TilePath(
			mv->type, umode,
			mv->cmpos.x, mv->cmpos.y,
			target, target2, targtype, cdtype, j->supbi,
			&tpath,
			u, NULL, demb,
			dempos.x, dempos.y,
			demcmminx, demcmminy, demcmmaxx, demcmmaxy,
			g_mapsz.x*g_mapsz.y);

		if(tpath.size() <= 0)
			continue;
#endif

#ifndef HIERPATH
		std::list<Vec2i> path;
		Vec2i subgoal;
		MvType* mvt = &g_mvtype[mv->type];

		PartialPath(
			mv->type, umode,
			mv->cmpos.x, mv->cmpos.y,
			target, target2, targtype, cdtype, j->supbi,
			&path, &subgoal,
			u, NULL, demb,
			dempos.x, dempos.y,
			demcmminx, demcmminy, demcmmaxx, demcmmaxy,
			MAXPATHN /* * mvt->cmspeed */,
			ectrue, ecfalse);

		if (path.size() <= 0)
		{
#if 0
	char m[123];
	sprintf(m, "tb2 np %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif
			//Mess(&RichText("no trip path"));
			continue;
		}
		
		path.clear();
		
		PartialPath(
			mv->type, UMODE_GOSUP,
			mv->cmpos.x, mv->cmpos.y,
			//target, target2, TARG_BL, cdtype,
			target, target2, targtype, cdtype, j->supbi,	//corpd fix 2016
			&path, &subgoal,
			u, NULL, j->potsupb,
			j->supcmpos.x, j->supcmpos.y,
			j->supcmpos.x, j->supcmpos.y, j->supcmpos.x, j->supcmpos.y,
			MAXPATHN /* * mvt->cmspeed */,
			ectrue, ecfalse);
			
		if (path.size() <= 0)
		{
#if 0
	char m[123];
	sprintf(m, "tb2 np2 %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif
			//Mess(&RichText("no trip path"));
			continue;
		}
#endif

#endif
#else
		Vec2i subgoal;
		std::list<Vec2i> path;

#ifdef RANDOM8DEBUG
		if(j->targu - g_mv == 12)
			thatunit = u - g_mv;
#endif

		JPSPath(mv->type, j->jobtype, mv->cmpos.x, mv->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype,
			&path, &subgoal,
			u, j->targu, j->targb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			nminx, nminy, nmaxx, nmaxy);

#ifdef RANDOM8DEBUG
		thatunit = -1;
#endif

		if(path.size() == 0)
		{
			//InfoMess("np","np");
			continue;
		}
#endif

			
#if 0
	char m[123];
	sprintf(m, "tb3 %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif

		mv->mode = UMODE_GOSUP;
		mv->goal = j->supcmpos;
		mv->cargoreq = amt;
		mv->cargotype = res;
		mv->cargoamt = 0;
		mv->target = target;
		mv->target2 = target2;
		mv->targtype = targtype;
		mv->cdtype = cdtype;
		mv->supplier = j->supbi;
		mv->subgoal = subgoal;
		mv->path = path;

		std::list<TrCycleHist>::reverse_iterator chit = mv->cyclehist.rbegin();
		TrCycleHist* lastch = &*chit;
		chit->jobsgiven++;

		//if(amt >0 && res == RES_CEMENT)
		//	InfoMess("req c", "req c");

		if(targtype == TARG_BL)
		{
			Bl* b = &g_bl[target];
			b->transporter[res] = j->truckui;
		}
		else if(targtype == TARG_CD)
		{
			CdTile* ctile = GetCd(cdtype, target, target2, ecfalse);
			ctile->transporter[res] = j->truckui;
		}

		NewJob(UMODE_GODRIVE, j->truckui, -1, CD_NONE);
		
		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
			PlayClip(g_trsnd[TRSND_NEWJOB]);

		//InfoMess("fj", "fj");


		//Log("job paths success "<<numpaths);

		return;
	}

#else	//ifndef TBID2

	Mv* mv = &g_mv[bestunit];

#if 0
	if(res == RES_URANIUM)
	{
		char msg[1280];
		sprintf(msg, "ur del \n bestunit=%d \n bef mv->mode=%d \n target=%d \n target2=%d \n cdtype=%d \n bestsup=%d \d amt=%d \d res=%d mv->cargotype=%d",
			bestunit,
			(int)mv->mode,
			target,
			target2,
			cdtype,
			bestsup,
			amt,
			res,
			mv->cargotype);
		InfoMess(msg, msg);
	}
#endif

	mv->mode = UMODE_GOSUP;
	mv->goal = bestsuppos;
	mv->cargoreq = amt;
	mv->cargotype = res;
	mv->cargoamt = 0;
	mv->target = target;
	mv->target2 = target2;
	mv->targtype = targtype;
	mv->cdtype = cdtype;
	mv->supplier = bestsup;

	if(targtype == TARG_BL)
	{
		Bl* b = &g_bl[target];
		b->transporter[res] = bestunit;
	}
	else if(targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(cdtype, target, target2, ecfalse);
		ctile->transporter[res] = bestunit;
	}

	NewJob(UMODE_GODRIVE, bestunit, -1, CD_NONE);
	PlayClip(g_trsnd[TRSND_NEWJOB]);

#if 0
	if(res == RES_URANIUM)
	{
		char msg[1280];
		sprintf(msg, "ur del 2 \n bestunit=%d \n bef mv->mode=%d \n target=%d \n target2=%d \n cdtype=%d \n bestsup=%d \d amt=%d \d res=%d mv->cargotype=%d",
			bestunit,
			(int)mv->mode,
			target,
			target2,
			cdtype,
			bestsup,
			amt,
			res,
			mv->cargotype);
		InfoMess(msg, msg);
	}
#endif

#endif
}

void MgTrips()
{
	//if(g_simframe % (SIM_FRAME_RATE*10) != 0)
	if(g_simframe % (SIM_FRAME_RATE*60) != 0)
		return;

	//2016/04/05 path'd trucks cleared now before bidding
	truckpathd.clear();

	//if(g_simframe % U_AI_DELAY != 0)
	//	return;
	
	//g_freetrucks.clear();
	//return;

	//LastNum("pre cheap fuel");

#if 0
	char m[123];
	sprintf(m, "mt %llu", g_simframe);
	RichText rm = m;
	AddChat(&rm);
#endif

	StartTimer(TIMER_MANAGETRIPS);

	//LastNum("pre bl trip");
#ifdef TRANSPORT_DEBUG
	{
		char msg[128];
		sprintf(msg, "pre bl trip");
		RichText debugrt;
		debugrt.part.push_back(msg);
		AddNotif(&debugrt);
	}
#endif

#if 1
	for(int i=0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;

		if(mv->type != MV_TRUCK)
			continue;

		if(mv->mode == UMODE_NONE)
			continue;

		//this unit must be stuck
		if(mv->jobframes > CYCLE_FRAMES*5)
		{
			ResetMode(u);
			mv->jobframes = -mv->jobframes;	//let somebody else get this job
		}
	}
#endif

	for(int i=0; i<BUILDINGS; i++)
	{
		Bl* b = &g_bl[i];

		if(!b->on)
			continue;

		BlType* t = &g_bltype[b->type];
		//Py* p = &g_py[b->owner];

		// bid for transportation of resources to finished bl
		if(b->finished)
		{
			//if(b->type == FACTORY)
			{
				//	char msg[128];
				//	sprintf(msg, "bid Bf 1 %s", t->name);
				//	Chat(msg);
			}
			//if(b->excessoutput())
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.1");
			//if(b->prodquota <= 0.0f)
			//	continue;
			//if(b->type == FACTORY)
			//	Chat("bid 1.2");
			for(int ri=0; ri<RESOURCES; ri++)
			{

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 1");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				if(t->input[ri] <= 0)
					continue;

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 2");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				Resource* r = &g_resource[ri];

				if(r->capacity)
					continue;

				if(r->conduit != CD_ROAD)
					continue;

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 3");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				if(!r->physical)
					continue;

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 4");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				if(ri == RES_LABOUR)
					continue;


#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 5");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				//	Chat("bid Bf 2");

				if(b->transporter[ri] >= 0)
				{
					//if(ri == RES_FARMPRODUCTS)
					//	AddChat(&RichText("ffff"));

					continue;
				}

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 6");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				//	Chat("bid Bf 3");

				int netreq = b->netreq(ri);

				if(netreq <= 0)
				{
					//if(ri == RES_FARMPRODUCTS)
					//	AddChat(&RichText("n"));

					continue;
				}

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS && b->type == BL_STORE)
				{
					char msg[128];
					sprintf(msg, "farmrpdo 7");
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				//if(b->type == FACTORY)
				{
					//	char msg[128];
					//	sprintf(msg, "bid Bf 5 %s", t->name);
					//	Chat(msg);
				}

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS)
				{
					char msg[128];
					sprintf(msg, "Tbid fini i%d, ri%d, netreq%d", i, ri, netreq);
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				TBid(i, -1, TARG_BL, UMODE_GODEMB, -1, ri, netreq);
			}
		}
		// bid for transportation of resources to building construction project
		else
		{
			for(int ri=0; ri<RESOURCES; ri++)
			{
				if(t->conmat[ri] <= 0)
					continue;

				Resource* r = &g_resource[ri];
				//Chat("1");

				if(r->capacity)
					continue;

				//Chat("2");

				if(!r->physical)
					continue;

				if(r->conduit != CD_ROAD)
					continue;

				if(ri == RES_LABOUR)
					continue;

				//if(ri == CEMENT)
				//	Chat("3");

				if(b->transporter[ri] >= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("4");

				//int netreq = b->netreq(ri);
				int netreq = t->conmat[ri] - b->conmat[ri];

				//if(ri == CEMENT)
				//	Chat("5");

				if(netreq <= 0)
					continue;

				//if(ri == CEMENT)
				//	Chat("6");

#ifdef TRANSPORT_DEBUG
				if(ri == RES_FARMPRODUCTS)
				{
					char msg[128];
					sprintf(msg, "Tbid cst i%d, ri%d, netreq%d", i, ri, netreq);
					RichText debugrt;
					debugrt.part.push_back(msg);
					AddNotif(&debugrt);
				}
#endif

				//if(netreq >0 && ri == RES_CEMENT)
				//	InfoMess("req c", "req c");

				TBid(i, -1, TARG_BL, UMODE_GODEMB, -1, ri, netreq);
			}
		}
	}

	for(int x=0; x<g_mapsz.x; x++)
		for(int y=0; y<g_mapsz.y; y++)
			for(unsigned char cti=0; cti<CD_TYPES; cti++)
			{
				CdTile* ctile = GetCd(cti, x, y, ecfalse);
				CdType* ctype = &g_cdtype[cti];

				if(!ctile->on)
					continue;
				if(ctile->finished)
					continue;

#ifdef TRUCK_DEBUG
				Log("road bid 0");
				
#endif
				for(int ri=0; ri<RESOURCES; ri++)
				{
					if(ctype->conmat[ri] <= 0)
						continue;

					Resource* r = &g_resource[ri];
#ifdef TRUCK_DEBUG
					Log("road bid 1");
					
#endif
					if(r->capacity)
						continue;

					if(r->conduit != CD_ROAD)
						continue;
#ifdef TRUCK_DEBUG
					Log("road bid 2");
					
#endif
					if(ri == RES_LABOUR)
						continue;
#ifdef TRUCK_DEBUG
					Log("road bid 3");
					
#endif
					if(ctile->transporter[ri] >= 0)
						continue;
#ifdef TRUCK_DEBUG
					Log("road bid 4 ri="<<g_resource[ri].name);
					
#endif
					int netreq = ctile->netreq(ri, cti);
					if(netreq <= 0)
						continue;
#ifdef TRUCK_DEBUG
					Log("road bid 5");
					
#endif

#ifdef TRANSPORT_DEBUG
					//if(ri == RES_FARMPRODUCTS)
					{
						char msg[128];
						sprintf(msg, "Tbid cdt xy%d,%d cti%d, ri%d, netreq%d", x, y, cti, ri, netreq);
						RichText debugrt;
						debugrt.part.push_back(msg);
						AddNotif(&debugrt);
					}
#endif

					//if(netreq >0 && ri == RES_CEMENT)
					//	InfoMess("req c", "req c");

					//Chat("bid r");
					TBid(x, y, TARG_CD, UMODE_GODEMCD, cti, ri, netreq);
				}
			}

	//LastNum("pre pick up bids");

	// pick which bid to take up

	//ecbool newjobs = ecfalse;

	//if(newjobs)
	//	NewJob();

	for(std::list<int>::iterator it=truckpathd.begin(); it!=truckpathd.end(); it++)
	{
		Mv* mv = &g_mv[*it];
		mv->jobframes = 0;
	}

	truckpathd.clear();


	StopTimer(TIMER_MANAGETRIPS);
}

#undef TRANSPORT_DEBUG
