













#include "job.h"
#include "building.h"
#include "conduit.h"
#include "mvtype.h"
#include "simdef.h"
#include "simflow.h"
#include "../econ/utility.h"
#include "../path/jpspath.h"
#include "../path/partialpath.h"
#include "../path/tilepath.h"
#include "../path/anypath.h"
#include "../path/pathnode.h"
#include "../path/pathjob.h"
#include "../algo/binheap.h"
#include "../path/collidertile.h"
#include "../path/fillbodies.h"
#include "../gui/layouts/chattext.h"
#include "../gui/layouts/messbox.h"

//#define PREPATHJOB		//check if path to job exists before considering job
//#define PATHTOJOB	//must we pathfind to job to see if there is a path to it before we consider the job?
#define PATHTOJOB2	//optimized

// comparison, not case sensitive.
ecbool CompareJobs(const JobOpp& a, const JobOpp& b)
{
#if 0
  unsigned int i=0;
  while ( (i<first.length()) && (i<second.length()) )
  {
    if (tolower(first[i])<tolower(second[i])) return ectrue;
    else if (tolower(first[i])>tolower(second[i])) return ecfalse;
    ++i;
  }
  return ( first.length() < second.length() );
#else
	return a.jobutil > b.jobutil;
#endif
}

ecbool FindJob(Mv* mv)
{
	if(mv->jobframes < LOOKJOB_DELAY_MAX)
		return ecfalse;

	//AddNotif(&RichText("look job"));

	StartTimer(TIMER_FINDJOB);

	mv->jobframes = 0;

	if(Trapped(u, NULL))
	{
		
	//AddNotif(&RichText("look job trapped"));
		//RichText rt = RichText("findjob trapped");
		//AddNotif(&rt);

		Jamify(u);

		StopTimer(TIMER_FINDJOB);
		return ecfalse;
	}

	//ecbool pathed = ecfalse;

	int bestjobtype = UMODE_NONE;
	int besttarget = -1;
	int besttarget2 = -1;
	//float bestDistWage = -1;
	//float distWage;
	int bestutil = -1;
	//ecbool fullquota;
	int bestctype = CD_NONE;
	Vec2i bestgoal;

	//Vec3f pos = camera.Position();
	//CResource* res;
	MvType* mvt = &g_mvtype[mv->type];

	//LastNum("before truck job");
	std::list<JobOpp> jobs;

	StartTimer(TIMER_JOBLIST);

	//Truck jobs
	for(int i=0; i<MOVERS; i++)
	{
		Mv* u2 = &g_mv[i];

		if(!u2->on)
			continue;

		if(u2->hp <= 0)
			continue;

		if(u2->type != MV_TRUCK)
			continue;

#if 0
		//only allow firm or state belonging to same state
		if(u2->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		//Chat("tj0");

		//if(mv->mode != AWAITINGDRIVER)
		//	continue;

#ifdef RANDOM8DEBUG
		if(i == 12)
			thatunit = u - g_mv;
#endif


		if(u2->mode != UMODE_GOSUP &&
			u2->mode != UMODE_GODEMB &&
			u2->mode != UMODE_GODEMCD &&
			u2->mode != UMODE_GOREFUEL)
		{


#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("not one of truck modes");
#endif

			continue;
		}

		//Chat("tj1");

		if(u2->driver >= 0 && &g_mv[u2->driver] != u)
		{

#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("driver>=0 && driver!=u");
#endif

			//AddNotif(&RichText("Has driver"));

			continue;
		}

		//Chat("tj2");

		Py* py = &g_py[u2->owner];


		//Chat("tj3");

		if(py->global[RES_DOLLARS] < u2->opwage)
		{
			Bankrupt(u2->owner, "truck expenses");
			continue;
		}

		//Chat("tj4");

		//int cmdist = MAG_VEC3F(mv->cmpos - u2->cmpos);
		const Vec2i cmoff = mv->cmpos - u2->cmpos;
		int cmdist = JOBHEUR(cmoff);

		if(cmdist > MAXPATHCM)
		{
			//AddNotif(&RichText(">jobdist"));
			continue;
		}

		//int jobutil = JobUtil(u2->opwage, cmdist, DRIVE_WORK_DELAY);
		int jobutil = JobUtil(py->truckwage, cmdist, DRIVE_WORK_DELAY);

#ifndef PATHTOJOB2
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef RANDOM8DEBUG
		if(i == 12)
			Log("better util");
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

#ifdef RANDOM8DEBUG
		if(i == 12)
			thatunit = u - g_mv;
#endif

#if 1
		JPSPath(mv->type, UMODE_GODRIVE, mv->cmpos.x, mv->cmpos.y,
			i, -1, TARG_U, CD_NONE,
			&path, &subgoal,
			u, u2, NULL,
			u2->cmpos.x, u2->cmpos.y,
			u2->cmpos.x, u2->cmpos.y, u2->cmpos.x, u2->cmpos.y);
#else
		PartialPath(mv->type, UMODE_GODRIVE, mv->cmpos.x, mv->cmpos.y,
			i, -1, TARG_U, CD_NONE,
			&path, &subgoal,
			u, u2, NULL,
			u2->cmpos.x, u2->cmpos.y,
			u2->cmpos.x, u2->cmpos.y, u2->cmpos.x, u2->cmpos.y,
			TILE_SIZE*2/PATHNODE_SIZE);
#endif

#ifdef RANDOM8DEBUG
		thatunit = -1;
#endif

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		bestutil = jobutil;
		besttarget = i;
		bestjobtype = UMODE_GODRIVE;
		bestgoal = u2->cmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GODRIVE;
		j->goal = u2->cmpos;
		j->targtype = TARG_U;
		j->targu = u2;
		j->targb = NULL;
		j->owner = u2->owner;

		//AddNotif(&RichText("have truck job"));
#endif
	}

#ifdef RANDOM8DEBUG
	thatunit = -1;
#endif

	// Construction jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Bl* b = &g_bl[i];

		if(!b->on)
			continue;

		if(b->finished)
			continue;
		
#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		BlType* bt = &g_bltype[b->type];

		if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
			continue;

		if(b->worker.size() >= CS_WORKERS)
			continue;

		Py* py = &g_py[b->owner];

		if(py->global[RES_DOLLARS] < b->conwage)
		{
			char reason[32];
			sprintf(reason, "%s construction", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//int cmdist = MAG_VEC3F(mv->cmpos - bcmpos);
		const Vec2i cmoff = mv->cmpos - bcmpos;
		int cmdist = JOBHEUR(cmoff);

		if(cmdist > MAXPATHCM)
			continue;

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

#if 0
		char msg[128];
		sprintf(msg, "%u job util %d", g_simframe, jobutil);
		RichText rt(msg);
		AddNotif(&rt);
#endif

#ifndef PATHTOJOB2
		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

		JPSPath(mv->type, UMODE_GOCSTJOB, mv->cmpos.x, mv->cmpos.y,
			i, -1, TARG_BL, CD_NONE,
			&path, &subgoal,
			u, NULL, b,
			bcmpos.x, bcmpos.y,
			bcmpos.x, bcmpos.y, bcmpos.x, bcmpos.y);

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		besttarget = i;
		bestjobtype = UMODE_GOCSTJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GOCSTJOB;
		j->goal = bcmpos;
		j->targtype = TARG_BL;
		j->targu = NULL;
		j->targb = b;
		j->owner = b->owner;
#endif
	}

	// Normal/building jobs
	for(int i=0; i<BUILDINGS; i++)
	{
		Bl* b = &g_bl[i];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		//if(!b->inoperation)
		//	continue;

		BlType* bt = &g_bltype[b->type];

		if(b->metout())
			continue;

		if(b->excin(RES_LABOUR))
			continue;

		if(b->opwage <= 0)
			continue;

		if(b->worker.size() >= iceil(bt->input[RES_LABOUR] * b->prodlevel, RATIO_DENOM * WORK_RATE))
			continue;

		Py* py = &g_py[b->owner];

		if(py->global[RES_DOLLARS] < b->opwage)
		{
			char reason[32];
			sprintf(reason, "%s expenses", bt->name);
			Bankrupt(b->owner, reason);
			continue;
		}

		Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//int cmdist = MAG_VEC3F(mv->cmpos - bcmpos);
		const Vec2i cmoff = mv->cmpos - bcmpos;
		int cmdist = JOBHEUR(cmoff);

		if(cmdist > MAXPATHCM)
			continue;

		int jobutil = JobUtil(b->conwage, cmdist, WORK_DELAY);

#ifndef PATHTOJOB2
		//if(distWage < bestDistWage)
		if(jobutil <= bestutil)
			continue;
#endif

#ifdef PATHTOJOB
		Vec2i subgoal;
		std::list<Vec2i> path;

		JPSPath(mv->type, UMODE_GOBLJOB, mv->cmpos.x, mv->cmpos.y,
			i, -1, TARG_BL, CD_NONE,
			&path, &subgoal,
			u, NULL, b,
			bcmpos.x, bcmpos.y,
			bcmpos.x, bcmpos.y, bcmpos.x, bcmpos.y);

		if(path.size() <= 0)
			continue;
#endif

#ifndef PATHTOJOB2
		besttarget = i;
		bestjobtype = UMODE_GOBLJOB;
		//bestDistWage = distWage;
		bestutil = jobutil;
		bestgoal = bcmpos;
#else
		jobs.push_back(JobOpp());
		JobOpp* j = &*jobs.rbegin();
		j->jobutil = jobutil;
		j->target = i;
		j->jobtype = UMODE_GOBLJOB;
		j->goal = bcmpos;
		j->targtype = TARG_BL;
		j->targu = NULL;
		j->targb = b;
		j->owner = b->owner;
#endif
	}

	//LastNum("after truck job 2");

	//Infrastructure/conduits construction jobs
	for(int ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int x=0; x<g_mapsz.x; x++)
		{
			for(int y=0; y<g_mapsz.y; y++)
			{
				CdTile* ctile = GetCd(ctype, x, y, ecfalse);

				if(!ctile->on)
					continue;

				if(ctile->finished)
					continue;
				
#if 0
				//only allow firm or state belonging to same state
				if(ctile->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
					continue;
#endif

				if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
					continue;
				
				if(CdWorkers(ctype, x, y) >= CS_WORKERS)
					continue;

				Py* py = &g_py[ctile->owner];

				if(py->global[RES_DOLLARS] < ctile->conwage)
				{
					Bankrupt(ctile->owner, "infrastructure construction");
					continue;
				}

				Vec2i ccmpos = Vec2i(x,y) * TILE_SIZE + ct->physoff;
				//int cmdist = MAG_VEC3F(mv->cmpos - ccmpos);
				const Vec2i cmoff = mv->cmpos - ccmpos;
				int cmdist = JOBHEUR(cmoff);

				if(cmdist > MAXPATHCM)
					continue;

				int jobutil = JobUtil(ctile->conwage, cmdist, WORK_DELAY);

#ifndef PATHTOJOB2
				if(jobutil <= bestutil)
					continue;
#endif

#ifdef PATHTOJOB
				Vec2i subgoal;
				std::list<Vec2i> path;

				JPSPath(mv->type, UMODE_GOCDJOB, mv->cmpos.x, mv->cmpos.y,
					x, y, TARG_CD, ctype,
					&path, &subgoal,
					u, NULL, NULL,
					ccmpos.x, ccmpos.y,
					ccmpos.x, ccmpos.y, ccmpos.x, ccmpos.y);

				if(path.size() <= 0)
					continue;
#endif

#ifndef PATHTOJOB2
				besttarget = x;
				besttarget2 = z;
				bestjobtype = UMODE_GOCDJOB;
				bestctype = ctype;
				bestutil = jobutil;
				bestgoal = ccmpos;
#else
				jobs.push_back(JobOpp());
				JobOpp* j = &*jobs.rbegin();
				j->jobutil = jobutil;
				j->target = x;
				j->target2 = y;
				j->ctype = ctype;
				j->jobtype = UMODE_GOCDJOB;
				j->goal = ccmpos;
				j->targtype = TARG_CD;
				j->targu = NULL;
				j->targb = NULL;
				j->owner = ctile->owner;

				//RichText msg("cd job");
				//AddNotif(&msg);
#endif
			}
		}
	}

	//RichText jt("have jobs");

	//if(jobs.size() > 0)
	//	AddNotif(&RichText("have jobs"));

	StopTimer(TIMER_JOBLIST);
	StartTimer(TIMER_JOBSORT);

	//mv->jobframes = 0;

#ifndef PATHTOJOB2
	if(bestutil <= 0 || bestjobtype == UMODE_NONE)
	{
		ResetGoal(u);
		StopTimer(TIMER_FINDJOB);
		return ecfalse;
	}
#else
	jobs.sort(CompareJobs);
#endif

	StopTimer(TIMER_JOBSORT);

#ifndef PATHTOJOB2
	ResetMode(u);

	mv->mode = bestjobtype;
	mv->goal = bestgoal;
	mv->target = besttarget;
	mv->target2 = besttarget2;
	mv->cdtype = bestctype;

	StopTimer(TIMER_FINDJOB);
#else

	StartTimer(TIMER_JOBPATH);

	int nminx = (mv->cmpos.x-MAXPATHCM/2)/PATHNODE_SIZE;
	int nminy = (mv->cmpos.y-MAXPATHCM/2)/PATHNODE_SIZE;
	int nmaxx = (mv->cmpos.x+MAXPATHCM/2)/PATHNODE_SIZE;
	int nmaxy = (mv->cmpos.y+MAXPATHCM/2)/PATHNODE_SIZE;

	//corpd fix xp
	nminx = imax(0,nminx);
	nminy = imax(0,nminy);
	nmaxx = imin(g_mapsz.x*TILE_SIZE-1,nmaxx);
	nmaxy = imin(g_mapsz.y*TILE_SIZE-1,nmaxy);

	//int numpaths = 0;

	for(std::list<JobOpp>::iterator jit=jobs.begin(); jit!=jobs.end(); jit++)
	{
		JobOpp* j = &*jit;

		if(j->jobutil <= 0)
		{
			//AddNotif(&RichText("jobutil 0"));
			goto fail;
		}

#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check");
			AddNotif(&m);
		}
#endif

		//numpaths++;

		//first pathable job
#if 1	//doesn't work because checks corner pathnode of tile for abruptness only, which invalidates whole tile at map edge
		//high level check first
		//edit: doesn't cause any problems?
		if(Trapped( u, j->targu ))
		{
			Jamify(u);

#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check trap fail");
			AddNotif(&m);
		}
#endif

			//AddNotif(&RichText("trapped"));
			continue;
		}
#endif

#ifdef HIERPATH
		std::list<Vec2s> tpath;

		TilePath(mv->type, j->jobtype, mv->cmpos.x, mv->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype, mv->supplier,
			&tpath,
			u, j->targu, j->targb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			g_mapsz.x*g_mapsz.y);

		if(tpath.size() <= 0)
		{
			
#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check tpath fail");
			AddNotif(&m);
		}
#endif
		
			//AddNotif(&RichText("no tpath"));
			continue;
		}

#else
		std::list<Vec2i> path;
		Vec2i subgoal;
		MvType* mvt = &g_mvtype[mv->type];

		PartialPath(mv->type, j->jobtype, mv->cmpos.x, mv->cmpos.y,
			j->target, j->target2, j->targtype, j->ctype, -1,
			&path, &subgoal,
			u, j->targu, j->targb,
			j->goal.x, j->goal.y,
			j->goal.x, j->goal.y, j->goal.x, j->goal.y,
			MAXPATHN /* * mvt->cmspeed */, 
			ectrue, ecfalse);

		if (path.size() <= 0)
		{
			//if (j->jobtype == UMODE_DRIVE)
			{
				//Log("drive job fail\r\n");
			}

			continue;
		}

		if (j->jobtype == UMODE_DRIVE)
		{
			//Log("drive job win\r\n");
		}
#endif

		/*
		for some reason i get to a dead end situation where all the trucks can't be jobbed due to this part
		using hierpath. so just ignore jams for going to drive trucks, since only 1 unit needs to get to it,
		unlike bl, which usually (stores) have a whole crowd of people lined up. and really, the whole
		reason for jams checking was because of worker clumps around stores.
		edit: maybe it's better for a truck to abandon its job if it's trapped and let another one take it up.
		*/
#ifndef HIERPATH	//this is the fix! but trucks can find paths despite being said trapped (otherwise), why?
		//edit: i see, maybe it's the "random" movement when no cm-path is found to next tile
		if(j->jobtype == UMODE_GODRIVE &&
			Trapped( &g_mv[j->target], u ))
		{
			//if(j->target == 15)
			//	InfoMess("15t", "15t");

			Mv* tu = &g_mv[j->target];
			Jamify(tu);

			ResetMode(tu);

			
#if 0
		if(j->jobtype == UMODE_GODRIVE)
		{
			RichText m = RichText("truck job check trap fail 2");
			AddNotif(&m);
		}
#endif

			continue;
		}
#endif

		ResetMode(u);

		
		//AddNotif(&RichText("found job"));

		mv->mode = j->jobtype;
		mv->goal = j->goal;
		mv->target = j->target;
		mv->target2 = j->target2;
		mv->cdtype = j->ctype;
		mv->exputil = j->jobutil;
#ifdef HIERPATH
		mv->tpath = tpath;
#else
		mv->path = path;
		mv->subgoal = subgoal;
#endif
		mv->owner = j->owner;
		
#if 0
	if(mv->owner >= PLAYERS)	//TODO figure why this case happens and fix it
	{
		char m[126];
		sprintf(m, "juown=%d", mv->owner);
		RichText rm = m;
		Mess(&rm);
		//return;
	}
#endif

		StopTimer(TIMER_JOBPATH);
		StopTimer(TIMER_FINDJOB);

		return ectrue;
	}

fail:
	StopTimer(TIMER_JOBPATH);
	StopTimer(TIMER_FINDJOB);
	ResetGoal(u);

	return ecfalse;

#endif

#if 0

	//ResetGoal();
	//LastNum("after truck job 5");

	if(bestjobtype == NONE)
	{
		//LastNum("after truck job 5a");
		//char msg[128];
		//sprintf(msg, "none j %d", UnitID(this));
		//Chat(msg);
		if(pathed)
			jobframes = 0;
		ResetMode();
		return ecfalse;
	}
	else
	{
		//LastNum("after truck job 5b");
		ResetGoal();
	}
#endif
	return ectrue;
}

//see if this new job is better than the one any labourer is currently going to
void NewJob(int jobtype, int target, int target2, int cdtype)
{
	//return;

	//std::list<Vec2i> prevpath;
	//int prevstep;

	Vec2i newgoal;
	int newpay = 0;
	signed char targtype = -1;
	Mv* targu = NULL;
	Bl* targb = NULL;
	int owner = -1;

	//string jobname = "unknown";

	//float newreqlab = 0;
	//float newworkdelay = 0;

	if(jobtype == UMODE_GOBLJOB)
	{
		targtype = TARG_BL;
		newgoal = g_bl[target].tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		Bl* b = &g_bl[target];
		targb = b;
		Py* p = &g_py[b->owner];
		//newpay = p->wage[b->type];
		newpay = b->opwage;
		//jobname = "normal job";
		//newreqlab = b->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
		owner = b->owner;
	}
	else if(jobtype == UMODE_GOCSTJOB)
	{
		targtype = TARG_BL;
		newgoal = g_bl[target].tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		Bl* b = &g_bl[target];
		targb = b;
		Py* p = &g_py[b->owner];
		//newpay = p->conwage;
		newpay = b->conwage;
		//jobname = "construction job";
		//newreqlab = b->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
		owner = b->owner;
	}
	else if(jobtype == UMODE_GOCDJOB)
	{
		targtype = TARG_CD;
		CdType* ct = &g_cdtype[cdtype];
		newgoal = Vec2i(target, target2) * TILE_SIZE + ct->physoff;
		CdTile* ctile = GetCd(cdtype, target, target2, ecfalse);
		//CPlayer* p = &g_py[road->owner];
		//newpay = p->conwage;
		newpay = ctile->conwage;
		//jobname = "road job";
		//newreqlab = road->netreq(LABOUR);
		//newworkdelay = WORK_DELAY;
		owner = (int)ctile->owner;
		
#if 0
		if(ctile->owner >= PLAYERS)	//TODO figure why this case happens and fix it
		{
			char m[126];
			sprintf(m, "njuown=%d jobtype=%d cdt=%d xy=%d,%d", ctile->owner, jobtype, cdtype, target, target2);
			RichText rm = m;
			Mess(&rm);
			//return;
		} 
#endif
	}
	else if(jobtype == UMODE_GODRIVE)
	{
		targtype = TARG_U;
		Mv* mv = &g_mv[target];
		targu = u;
		newgoal = mv->cmpos;
		Py* p = &g_py[mv->owner];
		newpay = p->truckwage;
		//jobname = "drive job";
		owner = mv->owner;

		//float trucklen = mv->pathlength();
		//CUnitType* truckt = &g_unitType[mv->type];
		//float truckseconds = trucklen / truckt->speed / (float)FRAME_RATE;
		//newworkdelay = DRIVE_WORK_DELAY;
		//newreqlab = LABOURER_LABOUR;	//max(1, truckseconds / newworkdelay * 1000.0f);
	}

#if 0
	CdTile* ctile = GetCd(CD_ROAD, 3, 12, ecfalse);
	
		if(ctile->owner >= PLAYERS)	//TODO figure why this case happens and fix it
		{
			char m[126];
			sprintf(m, "312 njuown=%d jobtype=%d cdt=%d xy=%d,%d", ctile->owner, jobtype, cdtype, target, target2);
			RichText rm = m;
			Mess(&rm);
			//return;
		} 
#endif

	for(int i=0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;

		if(mv->type != MV_LABOURER)
			continue;

		int prevjobtype = mv->mode;

		if(prevjobtype == jobtype && mv->target == target)
		{
			if(jobtype == UMODE_GOBLJOB || jobtype == UMODE_GOCSTJOB || jobtype == UMODE_GODRIVE)
			{
				//mv->mode = UMODE_NONE;
				//ResetGoal(u);
				continue;
			}
			else if(jobtype == UMODE_GOCDJOB)
			{
				if(mv->target2 == target2 && mv->cdtype == cdtype)
				{
					//mv->mode = UMODE_NONE;
					//ResetGoal(u);
					continue;
				}
			}
		}

		if(prevjobtype == UMODE_GOBLJOB ||
			prevjobtype == UMODE_GOCSTJOB ||
			prevjobtype == UMODE_GOCDJOB ||
			prevjobtype == UMODE_GODRIVE)
		{
			//mv->NewJob();
			//prevgoal = mv->goal;
			//prevsubgoal = mv->subgoal;

			//int newpathlen = MAG_VEC3F(newgoal - mv->cmpos);
			//int currpathlen = MAG_VEC3F(mv->goal - mv->cmpos);
			Vec2i newoff = newgoal - mv->cmpos;
			Vec2i curoff = mv->goal - mv->cmpos;
			int newpathlen = JOBHEUR(newoff);
			int currpathlen = JOBHEUR(curoff);
			int currpay = 0;
			//int currreqlab = 0;
			//int currworkdelay = 0;

			if(newpathlen > MAXPATHCM)
				continue;

			if(prevjobtype == UMODE_GOCSTJOB)
			{
				Bl* b = &g_bl[mv->target];
				//CPlayer* p = &g_py[b->owner];
				currpay = b->conwage;
				//currreqlab = min(mv->labour, b->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GOBLJOB)
			{
				Bl* b = &g_bl[mv->target];
				//CPlayer* p = &g_py[b->owner];
				//currpay = p->wage[b->type];
				currpay = b->opwage;
				//currreqlab = min(mv->labour, b->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GOCDJOB)
			{
				CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
				//CPlayer* p = &g_py[road->owner];
				//currpay = p->conwage;
				currpay = ctile->conwage;
				//currreqlab = min(mv->labour, road->netreq(LABOUR));
				//currworkdelay = WORK_DELAY;
			}
			else if(prevjobtype == UMODE_GODRIVE)
			{
				Mv* truck = &g_mv[mv->target];
				Py* p = &g_py[truck->owner];
				currpay = p->truckwage;
				//float trucklen = truck->pathlength();
				//CUnitType* truckt = &g_unitType[truck->type];
				//float truckseconds = trucklen / truckt->speed / (float)FRAME_RATE;
				//currworkdelay = DRIVE_WORK_DELAY;
				//currreqlab = mv->labour;	//min(mv->labour, max(1, truckseconds / currworkdelay * 1000.0f));
			}

			int currutil = JobUtil(currpay, currpathlen, WORK_DELAY);
			int newutil = JobUtil(newpay, newpathlen, WORK_DELAY);

			if(newutil < currutil)
				continue;

#if 0
			int jobtype, int target, int target2, int cdtype)
{
	//return;

	//std::list<Vec2i> prevpath;
	//int prevstep;

	Vec2i newgoal;
#endif

	/*
	Important: the pathfinding method here, including its search limit and whether
	it finds a path or doesn't in all cases, must match what is in FindJob and NewJob and 
	MoveUnit, because the unit might get stuck in loop as a new job is found for it, but 
	it's unable to adjust its path accordingly.
	*/
#ifdef HIERPATH
			std::list<Vec2s> tpath;

			TilePath(mv->type, jobtype, mv->cmpos.x, mv->cmpos.y,
				target, target2, targtype, cdtype, mv->supplier,
				&tpath,
				u, targu, targb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				//g_mapsz.x*g_mapsz.y
				MAX_U_TPATH);

			if(tpath.size() <= 0)
				continue;
#elif 0
			//TO DO: make us specify goal bounds outside of pathing functions,
			//so they don't have to do it for us.
			if(!AnyPath(mv->type, jobtype, mv->cmpos.x, mv->cmpos.y,
				target, target2, targtype, cdtype,
				u, targu, targb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				newgoal.x/PATHNODE_SIZE, newgoal.y/PATHNODE_SIZE, newgoal.x/PATHNODE_SIZE, newgoal.y/PATHNODE_SIZE))
				continue;
#elif 1

#if 0
			//high level check first
			std::list<Vec2s> tpath;

			TilePath(mv->type, jobtype, mv->cmpos.x, mv->cmpos.y,
				target, target2, targtype, cdtype,
				&tpath,
				u, targu, targb,
				newgoal.x, newgoal.y,
				newgoal.x, newgoal.y, newgoal.x, newgoal.y,
				g_mapsz.x*g_mapsz.y);

			if(tpath.size() <= 0)
				continue;
#endif

			//corpd fix 2016
			ResetMode(u);

			//2016/05/04 doesn't prepath to new job now, like for any initial job
#if 1
			//mv->freecollider();

			std::list<Vec2i> path;
			Vec2i subgoal;
			MvType* mvt = &g_mvtype[mv->type];

			PartialPath(mv->type, jobtype, mv->cmpos.x, mv->cmpos.y,
					target, target2, targtype, cdtype, -1,
					&path, &subgoal,
					u, targu, targb,
					newgoal.x, newgoal.y,
					newgoal.x, newgoal.y, newgoal.x, newgoal.y,
					MAXPATHN /* * mvt->cmspeed */,	//corpd fix 2016
					ectrue, ecfalse);

			if (path.size() <= 0)
			{
				//if (jobtype == UMODE_DRIVE)
				{
					//Log("drive job fail\r\n");
				}

				//2016/05/04 doesn't double-fill colliders now
				//mv->fillcollider();
				continue;
			}
			
			//2016/05/04 doesn't double-fill colliders now
			//mv->fillcollider();
#endif

			//if (jobtype == UMODE_DRIVE)
			{
				//Log("drive job win\r\n");
			}
#endif

			if(jobtype == UMODE_GODRIVE &&
				Trapped( &g_mv[target], u ))
			{
				Mv* tu = &g_mv[target];

				Jamify(tu);

				continue;
			}

			//otherwise, switch to new job
			//2016/05/04 doesn't use special, unknown resetpath method now which might be causing the "shedding"
			//mv->resetpath();
			mv->goal = newgoal;
			mv->mode = jobtype;
			mv->target = target;
			mv->target2 = target2;
			mv->cdtype = cdtype;
			mv->exputil = newutil;
			mv->owner = owner;
			//isn't necessary for labourers, and might not be reset,
			//being passed to a pathing function and causing wrong behaviour.
			//mv->targtype = targtype;

#if 0		
	if(mv->owner >= PLAYERS)	//TODO figure why this case happens and fix it
	{
		char m[126];
		sprintf(m, "njuown=%d jobtype=%d cdt=%d xy=%d,%d", mv->owner, jobtype, cdtype, target, target2);
		RichText rm = m;
		Mess(&rm);
		//return;
	}
#endif
#if 0
			CUnitType* t = &g_unitType[mv->type];

			float currtimeearn = currreqlab * currpay / (currpathlen / t->speed / (float)FRAME_RATE + currreqlab * currworkdelay / 1000.0f);

			mv->target = target;
			mv->target2 = target2;
			mv->mode = jobtype;
			mv->goal = newgoal;

			float newpathlen;
			float newreqlab2;
			float newtimeearn;

			if(!mv->Pathfind())
			{
				goto worsejob;
			}

			newpathlen = mv->pathlength();
			newreqlab2 = min(mv->labour, newreqlab);
			newtimeearn = newreqlab2 * newpay / (newpathlen / t->speed / (float)FRAME_RATE + newreqlab2 * newworkdelay / 1000.0f);

			//if(newpay / newpathlen > currpay / currpathlen)
			if(newtimeearn > currtimeearn)
			{
				//Log("found better job ["<<jobname<<"] path length = "<<newpathlen<<" ("<<(newpathlen/TILE_SIZE)<<" tiles)");
				//

				continue;
			}

worsejob:
			mv->target = prevtarget;
			mv->target2 = prevtarget2;
			mv->mode = prevjobtype;
			mv->goal = prevgoal;
			mv->subgoal = prevsubgoal;
			mv->path.clear();
			for(int j=0; j<prevpath.size(); j++)
				mv->path.push_back(prevpath[j]);
			mv->step = prevstep;
#endif
		}
	}
}
