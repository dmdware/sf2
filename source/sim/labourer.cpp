











#include "labourer.h"
#include "../platform.h"
#include "../econ/demand.h"
#include "mvtype.h"
#include "player.h"
#include "building.h"
#include "../econ/utility.h"
#include "../trigger/console.h"
#include "simdef.h"
#include "job.h"
#include "umove.h"
#include "../gui/layouts/chattext.h"
#include "../render/transaction.h"
#include "../sound/sound.h"
#include "truck.h"
#include "../math/fixmath.h"
#include "../path/pathjob.h"
#include "../path/fillbodies.h"
#include "simflow.h"
#include "../render/fogofwar.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../path/tilepath.h"
#include "../language.h"

//not engine
#include "../gui/layouts/playgui.h"

short g_labsnd[LAB_SOUNDS] = {-1,-1,-1};

ecbool NeedFood(Mv* mv)
{
	//if(mv->belongings[RES_RETFOOD] < STARTING_RETFOOD/2)
	//	return ectrue;

	Bl* b;
	int bestbi;
	int bestutil;
	
	BestFood(u, &bestbi, &bestutil);

	if(bestbi < 0)
		return ecfalse;

	b = &g_bl[bestbi];

	if(ConsumProp(u, b->price[RES_RETFOOD]) > 0)
		return ectrue;

	return ecfalse;
}

void BestFood(Mv* mv, int* bestbi, int* bestutil)
{
	Vec2i bcmpos;
	*bestbi = -1;
	*bestutil = -1;

	//for(int bi=0; bi<BUILDINGS; bi++)
	for(std::list<unsigned short>::iterator bit=g_onbl.begin(); bit!=g_onbl.end(); bit++)
	{
		unsigned short bi = *bit;

		Bl* b = &g_bl[bi];

		//if(!b->on)
		//	continue;

		if(!b->finished)
			continue;

#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Py* py = &g_py[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->price[RES_RETFOOD] > mv->belongings[RES_DOLLARS])
			continue;

		//leave enough money for next cycle's rent payment
		if(mv->home >= 0)
		{
			Bl* hm = &g_bl[mv->home];
			if(mv->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
				continue;
		}

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - mv->cmpos;
		int cmdist = FUELHEUR(cmoff);
		int thisutil = PhUtil(b->price[RES_RETFOOD], cmdist);

		if(*bestutil > thisutil && *bestbi >= 0)
			continue;

		if(b->occupier.size() >= MAX_SHOP * bt->width.x * bt->width.y)
			continue;

		*bestbi = bi;
		*bestutil = thisutil;
	}
}

ecbool FindFood(Mv* mv)
{
	int bestbi;
	int bestutil;

	BestFood(u, &bestbi, &bestutil);

	ResetGoal(u);
	mv->target = bestbi;
	mv->mode = UMODE_GOSHOP;
	Bl* b = &g_bl[mv->target];
	mv->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	mv->exputil = bestutil;

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return ectrue;
}

ecbool NeedRest(Mv* mv)
{
	if(mv->belongings[RES_LABOUR] <= 0)
		return ectrue;

	return ecfalse;
}

void GoHome(Mv* mv)
{
	ResetGoal(u);
	mv->target = mv->home;
	mv->mode = UMODE_GOREST;
	Bl* b = &g_bl[mv->target];
	mv->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	const Vec2i cmoff = mv->goal - mv->cmpos;
	int cmdist = JOBHEUR( cmoff );
	int price = b->price[RES_HOUSING];
	mv->exputil = PhUtil(price, cmdist);

	//Pathfind();
	//float pathlen = pathlength();
	//Log("go home path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//
}

ecbool FindRest(Mv* mv)
{
	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;
		
#if 0
		//only allow firm or state belonging to same state
		if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
			continue;
#endif

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_HOUSING] <= 0)
			continue;

		//if(b->stocked[RES_HOUSING] <= 0)
		//	continue;

		//if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
		//	continue;

		if(b->occupier.size() >= bt->output[RES_HOUSING])
			continue;

		if(b->price[RES_HOUSING] > mv->belongings[RES_DOLLARS])
			continue;

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - mv->cmpos;
		int cmdist = FUELHEUR(cmoff);
		int thisutil = PhUtil(b->price[RES_HOUSING], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return ecfalse;

	ResetGoal(u);
	mv->target = bestbi;
	mv->mode = UMODE_GOREST;
	Bl* b = &g_bl[mv->target];
	mv->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	mv->home = mv->target;
	b->occupier.push_back(u-g_mv);
	//g_bl[target].addoccupier(this);

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return ectrue;
}

// check building construction job availability
ecbool CanCstBl(Mv* mv)
{
	if(mv->belongings[RES_LABOUR] <= 0)
		return ecfalse;

	Bl* b = &g_bl[mv->target];
	BlType* bt = &g_bltype[b->type];
	Py* py = &g_py[b->owner];

	if(!b->on)
		return ecfalse;

	if(b->finished)
		return ecfalse;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
		return ecfalse;
#endif

	if(b->conmat[RES_LABOUR] >= bt->conmat[RES_LABOUR])
		return ecfalse;
	
	if(b->worker.size() >= CS_WORKERS &&
		mv->mode != UMODE_CSTJOB)
		return ecfalse;

	if(py->global[RES_DOLLARS] < b->conwage)
	{
		char reason[32];
		sprintf(reason, "%s construction", bt->name);
		Bankrupt(b->owner, reason);
		return ecfalse;
	}

	return ectrue;
}

// go to construction job
void GoCstJob(Mv* mv)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}
}

// do construction job
void DoCstJob(Mv* mv)
{
	if(!CanCstBl(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(mv->framesleft > 0)
	if(mv->cyframes > 0)
	{
		//mv->framesleft --;
		return;
	}

	//last = GetTicks();
	//mv->framesleft = WORK_DELAY;

	Bl* b = &g_bl[mv->target];
	Py* py = &g_py[b->owner];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	
	int subt = imin(mv->belongings[RES_LABOUR], WORK_RATE);

	b->conmat[RES_LABOUR] += subt;
	mv->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= b->conwage * subt / WORK_RATE;
	mv->belongings[RES_DOLLARS] += b->conwage * subt / WORK_RATE;
	mv->incomerate += b->conwage * subt / WORK_RATE;
	//2015/11/16
	//lastch->cons[RES_DOLLARS] += b->conwage;
	py->gnp += b->conwage * subt / WORK_RATE;

#ifdef PYG
	PyGraph* pyg = &g_pygraphs[b->owner][PYGRAPH_CSEXP];
	*pyg->fig.rbegin() += b->conwage;
	pyg = &g_pygraphs[b->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += b->conwage;
#endif

#if 1
	//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_LABOUR];
			char numpart[128];
			sprintf(numpart, "%+d", subt);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -b->conwage * subt / WORK_RATE);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), &transx);

		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
			PlayClip(g_labsnd[LABSND_WORK]);
	}
#endif

	if(b->checkconstruction())
		ResetMode(u);

	//char msg[128];
	//sprintf(msg, "construction %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->conwage, msg);
}

// check target building's job availability
ecbool CanBlJob(Mv* mv)
{
	//CBuildingType* t = &g_buildingType[b->type];

	if(mv->belongings[RES_LABOUR] <= 0)
		return ecfalse;

	Bl* b = &g_bl[mv->target];

	if(!b->on)
		return ecfalse;

	if(!b->finished)
		return ecfalse;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
		return ecfalse;
#endif

	//LastNum("checknorm1");

	if(b->metout())
		return ecfalse;

	if(b->excin(RES_LABOUR))
		return ecfalse;

	if(b->opwage <= 0)
		return ecfalse;

	BlType* bt = &g_bltype[b->type];

	if(b->worker.size() > iceil(bt->input[RES_LABOUR] * b->prodlevel, RATIO_DENOM * WORK_RATE) /* &&
		mv->mode != UMODE_BLJOB */)
		return ecfalse;

	//LastNum("checknorm3");

	//if(b->occupier.size() > 0 && !b->hasworker(u - g_mv))
	//	return ecfalse;

	Py* py = &g_py[b->owner];

	if(py->global[RES_DOLLARS] < b->opwage)
	{
		char reason[64];
		sprintf(reason, "%s expenses", g_bltype[b->type].name);
		Bankrupt(b->owner, reason);
		return ecfalse;
	}

	return ectrue;
}

// go to building job
void GoBlJob(Mv* mv)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}
}

// do building job
void DoBlJob(Mv* mv)
{
	//LastNum("gotonormjob1");
	if(!CanBlJob(u))
	{
		//LastNum("gotonormjob2!");
		ResetMode(u);
		//LastNum("gotonormjob3!");
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(mv->cyframes > 0)
		return;

	//last = GetTicks();
	Bl* b = &g_bl[mv->target];
	Py* py = &g_py[b->owner];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	
	int subt = imin(mv->belongings[RES_LABOUR], WORK_RATE);

	b->stocked[RES_LABOUR] += subt;
	mv->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= b->opwage * subt / WORK_RATE;
	mv->belongings[RES_DOLLARS] += b->opwage * subt / WORK_RATE;
	mv->incomerate += b->opwage * subt / WORK_RATE;
	lastch->cons[RES_DOLLARS] += b->opwage * subt / WORK_RATE;
	py->gnp += b->opwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[b->owner][PYGRAPH_WORKLABEXP];
	*pyg->fig.rbegin() += b->opwage;
	pyg = &g_pygraphs[b->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += b->opwage;
#endif

	if(!b->tryprod())
	{
		//b->Emit(HAMMER);
#ifdef LOCAL_TRANSX
		if(b->owner == g_localP)
#endif
		{
			RichText tt;	//transaction text

			tt.part.push_back(RichPart(RICH_ICON, ICON_LABOUR));

			char ft[32];	//labour text
			sprintf(ft, "%+d ", subt);
			tt.part.push_back(RichPart(UStr(ft)));

			tt.part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));

			char mt[32];	//money text
			sprintf(mt, "%+d ", b->opwage * subt / WORK_RATE);
			tt.part.push_back(RichPart(UStr(mt)));

			NewTransx(mv->cmpos, &tt);
		}
	}
	
	if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
		g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
		PlayClip(g_labsnd[LABSND_WORK]);

	//char msg[128];
	//sprintf(msg, "job %s", g_buildingType[b->type].name);
	//LogTransx(b->owner, -p->wage[b->type], msg);
}

// check conduit construction job availability
ecbool CanCstCd(Mv* mv)
{
	if(mv->belongings[RES_LABOUR] <= 0)
		return ecfalse;

	CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
	Py* py = &g_py[ctile->owner];

	if(!ctile->on)
		return ecfalse;

	if(ctile->finished)
		return ecfalse;
	
#if 0
	//only allow firm or state belonging to same state
	if(ctile->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
		return ecfalse;
#endif

	if(ctile->conwage <= 0)
		return ecfalse;

	CdType* ct = &g_cdtype[mv->cdtype];

	if(ctile->conmat[RES_LABOUR] >= ct->conmat[RES_LABOUR])
		return ecfalse;
	
	if(CdWorkers(mv->cdtype, mv->target, mv->target2) >= CS_WORKERS &&
		mv->mode != UMODE_CDJOB)
		return ecfalse;

	if(py->global[RES_DOLLARS] < ctile->conwage)
	{
		Bankrupt(ctile->owner, "conduit construction");
		return ecfalse;
	}

	return ectrue;
}

// go to conduit (construction) job
void GoCdJob(Mv* mv)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

// do conduit (construction) job
void DoCdJob(Mv* mv)
{
	if(!CanCstCd(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(mv->cyframes > 0)
		return;

	//last = GetTicks();
	CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
	Py* py = &g_py[ctile->owner];

#if 0
	if(ctile->owner < 0 || ctile->owner >= PLAYERS)
	{
		InfoMess("!","!");
	}
#endif

	int subt = imin(mv->belongings[RES_LABOUR], WORK_RATE);

	ctile->conmat[RES_LABOUR] += subt;
	mv->belongings[RES_LABOUR] -= subt;

	py->global[RES_DOLLARS] -= ctile->conwage * subt / WORK_RATE;
	mv->belongings[RES_DOLLARS] += ctile->conwage * subt / WORK_RATE;
	mv->incomerate += ctile->conwage * subt / WORK_RATE;
	py->gnp += ctile->conwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[ctile->owner][PYGRAPH_CSEXP];
	*pyg->fig.rbegin() += ctile->conwage;
	pyg = &g_pygraphs[ctile->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += ctile->conwage;
#endif

	//r->Emit(HAMMER);
#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_LABOUR];
			char numpart[128];
			sprintf(numpart, "%+d", subt);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}
		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -ctile->conwage * subt / WORK_RATE);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		NewTransx(mv->cmpos, &transx);

		//NewTransx(RoadPosition(target, target2), CURRENC, -p->conwage);
		//NewTransx(RoadPosition(target, target2), LABOUR, 1, CURRENC, -p->conwage);
		
		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
			PlayClip(g_labsnd[LABSND_WORK]);
	}

	if(ctile->checkconstruction(mv->cdtype))
		ResetMode(u);

	//LogTransx(r->owner, -p->conwage, "road job");
}

// check shop availability
ecbool CanShop(Mv* mv)
{
	Bl* b = &g_bl[mv->target];
	Py* p = &g_py[b->owner];

	if(!b->on)
		return ecfalse;

	if(!b->finished)
		return ecfalse;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
		return ecfalse;
#endif

	if(b->stocked[RES_RETFOOD] + p->global[RES_RETFOOD] <= 0)
		return ecfalse;

	if(b->price[RES_RETFOOD] > mv->belongings[RES_DOLLARS])
		return ecfalse;

	if(mv->belongings[RES_RETFOOD] >= MUL_RETFOOD)
		return ecfalse;

	if(mv->home >= 0)
	{
		Bl* hm = &g_bl[mv->home];
		if(mv->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
			return ecfalse;
	}

	int shopq = ConsumProp(u, b->price[RES_RETFOOD]);

	if(shopq <= 0)
		return ecfalse;

	BlType* bt = &g_bltype[b->type];

	if(b->occupier.size() >= MAX_SHOP * bt->width.x * bt->width.y)
		return ecfalse;

	return ectrue;
}

// go shop
void GoShop(Mv* mv)
{
	if(!CanShop(u))
	{
		ResetMode(u);
		return;
	}
}

// check apartment availabillity
ecbool CanRest(Mv* mv, ecbool* eviction)
{
	if(mv->home < 0)
		return ecfalse;

	Bl* b = &g_bl[mv->target];
	Py* p = &g_py[b->owner];
	*eviction = ecfalse;

	if(!b->on)
		return ecfalse;

	if(!b->finished)
		return ecfalse;
	
#if 0
	//only allow firm or state belonging to same state
	if(b->owner / (FIRMSPERSTATE+1) != mv->owner / (FIRMSPERSTATE+1))
		return ecfalse;
#endif

	//if(b->stock[HOUSING] + p->global[HOUSING] <= 0.0f)
	//	return ecfalse;

	//if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
	//	continue;

	BlType* bt = &g_bltype[b->type];

	if(b->occupier.size() > bt->output[RES_HOUSING])
		return ecfalse;

	if(b->price[RES_HOUSING] > mv->belongings[RES_DOLLARS])
	{
		//char msg[128];
		//sprintf(msg, "eviction %f < %f", currency, p->price[HOUSING]);
		//LogTransx(b->owner, 0.0f, msg);
		*eviction = ectrue;
		return ecfalse;
	}

	if(mv->belongings[RES_LABOUR] >= STARTING_LABOUR)
		return ecfalse;

	return ectrue;
}

//silent
void Evict(Bl* b)
{
	while(b->occupier.size() > 0)
	{
		int i = *b->occupier.begin();
		//Evict(&g_mv[i]);
		Mv* mv = &g_mv[i];
		mv->home = -1;
		b->occupier.erase( b->occupier.begin() );
		//ResetMode(u);
		if(mv->mode == UMODE_RESTING ||
			mv->mode == UMODE_GOREST)
			ResetMode(u);
	}
}

void Evict(Mv* mv, ecbool silent)
{
	if(mv->home < 0)
		return;

	if(!mv->on)
		silent = ectrue;	//bug?

	Bl* b = &g_bl[mv->home];

	int ui = u - g_mv;

	for(std::list<int>::iterator uiter = b->occupier.begin(); uiter != b->occupier.end(); uiter++)
	{
		if(*uiter == ui)
		{
			b->occupier.erase( uiter );
			break;
		}
	}

	mv->home = -1;

	if(!silent)
	{
		char msg[512];
		sprintf(msg, "%s ", Time().c_str());
		RichText em = RichText(msg) + STRTABLE[STR_EVICTION];
		//SubmitConsole(&em);
		AddNotif(&em);
	}
}

// go rest - perform checks that happen as the labourer is going to his home to rest
void GoRest(Mv* mv)
{
	ecbool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CanRest()");
		if(eviction)
			Evict(u, ecfalse);
		
		ResetMode(u);
		return;
	}
	/*
	if(goal - camera.Position() == Vec3f(0,0,0))
	{
	char msg[128];
	sprintf(msg, "faulty %d", UnitID(this));
	Chat(msg);
	ResetMode();
	}*/
}

// check if labourer has enough food to multiply
void CheckMul(Mv* mv, int foodpr)
{
	if(mv->belongings[RES_RETFOOD] < MUL_RETFOOD)
		return;

	if(mv->home < 0)
		return;

#if 0
	int i = NewUnit();
	if(i < 0)
		return;

	CUnit* u = &g_mv[i];
	mv->on = ectrue;
	mv->type = LABOURER;
	mv->home = -1;

	CUnitType* t = &g_unitType[LABOURER];

	PlaceUAround(u, camera.Position().x-t->radius, camera.Position().z-t->radius, camera.Position().x+t->radius, camera.Position().z+t->radius, ecfalse);

	mv->ResetLabourer();
	mv->fuel = MULTIPLY_FUEL/2.0f;
	fuel -= MULTIPLY_FUEL/2.0f;
#endif

#if 1

	foodpr = imax(1, foodpr);

	int saverent = 0;

	if(mv->home >= 0)
	{
		Bl* b = &g_bl[mv->home];
		saverent = b->price[RES_HOUSING] * 2;	//save
	}

	int savefood = STARTING_RETFOOD * foodpr;
	int divmoney = ( saverent + savefood ) / 2;	//min division money
	int tospend = mv->belongings[RES_DOLLARS] - saverent;
	int maxbuy = tospend / foodpr;

	maxbuy = imax(1, maxbuy);

	int portion = STARTING_RETFOOD * RATIO_DENOM / maxbuy;	//max division buys
	portion = imax(0, portion);
	portion = imin(RATIO_DENOM, portion);

	int inherit = mv->belongings[RES_DOLLARS] * portion / RATIO_DENOM;	//leave a portion equal among how many children possible
	inherit = imax(inherit, divmoney);	//don't have more children than can have divmoney
	inherit = imin(inherit, mv->belongings[RES_DOLLARS] / 2);	//don't give more than half

	if(inherit < divmoney)
		return;

#endif

	Vec2i cmpos;

	if(!PlaceUAb(MV_LABOURER, mv->cmpos, &cmpos))
		return;

	int ui = -1;

	if(!PlaceUnit(MV_LABOURER, cmpos, mv->owner, &ui))
		return;

	Mv* u2 = &g_mv[ui];
	StartBel(u2);

	//mv->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

#if 0
	u2->belongings[RES_RETFOOD] = mv->belongings[RES_RETFOOD] / 2;
	mv->belongings[RES_RETFOOD] -= u2->belongings[RES_RETFOOD];

	u2->belongings[RES_DOLLARS] = mv->belongings[RES_DOLLARS] / 2;
	mv->belongings[RES_DOLLARS] -= u2->belongings[RES_DOLLARS];
#else

	u2->belongings[RES_RETFOOD] = STARTING_RETFOOD;
	mv->belongings[RES_RETFOOD] -= STARTING_RETFOOD;

	u2->belongings[RES_DOLLARS] = inherit;
	mv->belongings[RES_DOLLARS] -= inherit;
#endif

	char msg[512];
	sprintf(msg, "%s %s (%s %d.)", 
		Time().c_str(), 
		STRTABLE[STR_GROWTH].rawstr().c_str(),
		STRTABLE[STR_POPULATION].rawstr().c_str(),
		CntMv(MV_LABOURER));
	RichText gr;
	gr.part.push_back(UStr(msg));
	AddNotif(&gr);
	//SubmitConsole(&gr);
}

//react to changed saving req's on the spot
void CheckMul(Mv* mv)
{
	if(mv->belongings[RES_RETFOOD] < MUL_RETFOOD)
		return;

	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFOOD] <= 0)
			continue;

		Py* py = &g_py[b->owner];

		if(b->stocked[RES_RETFOOD] + py->global[RES_RETFOOD] <= 0)
			continue;

		if(b->price[RES_RETFOOD] > mv->belongings[RES_DOLLARS])
			continue;

		//leave enough money for next cycle's rent payment
		if(mv->home >= 0)
		{
			Bl* hm = &g_bl[mv->home];
			if(mv->belongings[RES_DOLLARS] <= hm->price[RES_HOUSING]*2)
				continue;
		}

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - mv->cmpos;
		int cmdist = FUELHEUR(cmoff);
		int thisutil = PhUtil(b->price[RES_RETFOOD], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return;

#if 0
	ResetGoal(u);
	mv->target = bestbi;
	mv->mode = UMODE_GOSHOP;
	Bl* b = &g_bl[mv->target];
	mv->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
#endif

	Bl* b = &g_bl[bestbi];
	int foodpr = b->price[RES_RETFOOD];
	CheckMul(u, foodpr);
}

int ConsumProp(int satiety, int incomerate, int bprice)
{
	bprice = imax(1, bprice);
	
	//TODO need limiting/cutting back on demand for elasticity?
	//int extrashop = imax(0, incomerate / ( bprice * bprice ) );
	int extrashop = 0;
	
	int totalshop = extrashop - satiety;

	return totalshop;
}

//consumption propensity? TODO: better names
int ConsumProp(Mv* mv, int bprice)
{
	int satiety = mv->belongings[RES_RETFOOD] - STARTING_RETFOOD;

	return ConsumProp(satiety,  mv->incomerate, bprice);
}

// do shop
void DoShop(Mv* mv)
{
	Bl* b = &g_bl[mv->target];
	CheckMul(u, b->price[RES_RETFOOD]);

	if(!CanShop(u))
	{
		ResetMode(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	// TO DO: make consume 50-15 per sec from py->global and then b->stocked

	if(mv->cyframes > 0)
	//if(mv->cyframes % 2 == 0)
		return;

	//last = GetTicks();

	Py* p = &g_py[b->owner];

#if 0
	if(p->global[RES_RETFOOD] > 0)
		p->global[RES_RETFOOD] -= 1;
	else
	{
		b->stocked[RES_RETFOOD] -= 1;
		p->local[RES_RETFOOD] -= 1;
	}

	mv->belongings[RES_RETFOOD] += 1;
	p->global[RES_DOLLARS] += b->price[RES_RETFOOD];
	mv->belongings[RES_DOLLARS] -= b->price[RES_RETFOOD];
	//b->recenth.consumed[CONSUMERGOODS] += 1.0f;
#endif
	
	CycleHist *lastch = &*b->cyclehist.rbegin();

	//add some so they don't come back for 1 more each next frame
	int totalshop = ConsumProp(u, b->price[RES_RETFOOD]) + CYCLE_FRAMES / FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM;

	//totalshop = imin(SHOP_RATE, totalshop);

	//divide trying shop rate by 2 each time until we can afford it and there's enough food available
	//for(int trysub=SHOP_RATE; trysub>0; trysub>>=1)
	for(int trysub=totalshop; trysub>0; trysub>>=1)
	{
		int cost = b->price[RES_RETFOOD] * trysub;

		if(cost > mv->belongings[RES_DOLLARS])
			continue;

		int sub[RESOURCES];
		Zero(sub);
		sub[RES_RETFOOD] = trysub;

		if(!TrySub(sub, p->global, b->stocked, p->local, NULL, NULL))
			continue;

		mv->belongings[RES_RETFOOD] += trysub;
		p->global[RES_DOLLARS] += b->price[RES_RETFOOD] * trysub;
		p->gnp += b->price[RES_RETFOOD] * trysub;
		mv->belongings[RES_DOLLARS] -= b->price[RES_RETFOOD] * trysub;
		lastch->prod[RES_DOLLARS] += b->price[RES_RETFOOD] * trysub;	//"cons" is expenses and CONSUMPTION, edit: but "prod" is EARNINGS and production
		lastch->cons[RES_RETFOOD] += trysub;	//corpd fix xp
		
#if 1
	//b->Emit(SMILEY);
#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText tt;	//transaction text

		tt.part.push_back(RichPart(RICH_ICON, ICON_DOLLARS));

		char mt[32];	//money text
		sprintf(mt, "%+d ", b->price[RES_RETFOOD]);
		tt.part.push_back(RichPart(UStr(mt)));

		tt.part.push_back(RichPart(RICH_ICON, ICON_RETFOOD));

		char ft[32];	//food text
		sprintf(ft, "%+d ", -trysub);
		tt.part.push_back(RichPart(UStr(ft)));

		NewTransx(mv->cmpos, &tt);
		
		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
			PlayClip(g_labsnd[LABSND_SHOP]);
	}
#endif

		break;
	}
}

// do rest
void DoRest(Mv* mv)
{
	ecbool eviction;
	if(!CanRest(u, &eviction))
	{
		//Chat("!CheckApartmentAvailability()");
		if(eviction)
			Evict(u, ecfalse);
		
		ResetMode(u);
		return;
	}

	if(mv->cyframes > 0)
		return;

	mv->belongings[RES_LABOUR] += WORK_RATE;

	Bl *b = &g_bl[mv->home];
	CycleHist *lastch = &*b->cyclehist.rbegin();
	//lastch->cons[RES_HOUSING] += 1;	//"cons" is expenses and CONSUMPTION

#if 1
	RichText rt(STRTABLE[STR_REST]);
	//SubmitConsole(&em);
	//AddNotif(&rt);
	NewTransx(mv->cmpos, &rt);
	if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
		g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
		PlayClip(g_labsnd[LABSND_REST]);
#endif
}

// check transport vehicle availability
ecbool CanDrive(Mv* op)
{
	Mv* tr = &g_mv[op->target];

//#define HIERDEBUG

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive1");
	}
#endif

	if(!tr->on)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive2");
	}
#endif

	if(tr->type != MV_TRUCK)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive3");
	}
#endif

	if(tr->hp <= 0)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive4");
	}
#endif

	if(tr->driver >= 0 && &g_mv[tr->driver] != op)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive5");
	}
#endif

	if(tr->mode != UMODE_GOSUP &&
		tr->mode != UMODE_GODEMB &&
		tr->mode != UMODE_GODEMCD &&
		tr->mode != UMODE_GOREFUEL)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive6");
	}
#endif

	if(op->belongings[RES_LABOUR] <= 0)
		return ecfalse;

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive7");
	}
#endif

	Py* p = &g_py[tr->owner];

	//if(p->global[RES_DOLLARS] < tr->opwage)
	if(p->global[RES_DOLLARS] < p->truckwage)
	{
		Bankrupt(tr->owner, "truck expenses");
		return ecfalse;
	}

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive8");
	}
#endif

	Py* demp = NULL;
	int dempi = -1;

	if(tr->targtype == TARG_BL)
	{
		Bl* b = &g_bl[tr->target];
		dempi = b->owner;
		demp = &g_py[b->owner];
	}
	else if(tr->targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(tr->cdtype, tr->target, tr->target2, ecfalse);
		dempi = ctile->owner;
		demp = &g_py[ctile->owner];
	}

	if(demp && demp->global[RES_DOLLARS] < p->transpcost)
	{
		Bankrupt(dempi, "transport fees");
		return ecfalse;
	}

	if(Trapped(tr, op))
	{
		Jamify(tr);
		return ecfalse;
	}

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(op - g_mv == 82)
	{
		Log("the 13th unit: candrive9");
	}
#endif

	
//#undef HIERDEBUG

	return ectrue;
}

// go to transport for drive job
void GoDrive(Mv* mv)
{
	if(!CanDrive(u))
	{
		ResetMode(u);
		return;
	}

	if(CheckIfArrived(u))
		OnArrived(u);
}

//driver labourer to disembark driven transport vehicle
void Disembark(Mv* op)
{
#if 0
	char m[123];
	sprintf(m, "disembark-------- u%d m%d t%d d%d", (int)(op-g_mv), (int)op->mode, op->target, g_mv[op->target].driver);
	RichText rm(m);
	AddNotif(&rm);
#endif

	if(op->mode == UMODE_GODRIVE)
	{
		ResetMode(op);
		return;
	}

	if(op->mode != UMODE_DRIVE)
		return;

	Mv* tr = &g_mv[op->target];
	//camera.MoveTo( mv->camera.Position() );
	Vec2i trpos = tr->cmpos;
	MvType* t = &g_mvtype[tr->type];
	ResetMode(op, ecfalse);
	//must be a better way to do this - fillcollider is called already
	RemVis(op);
	//op->freecollider();
	Vec2i oppos;
	PlaceUAb(op->type, trpos, &oppos);
	op->cmpos = oppos;

	//if(op - g_mv == 182 && g_simframe > 118500)
		//Log("f7");

	op->fillcollider();
	AddVis(op);
	//TODO
	//op->drawpos = Vec3f(oppos.x, g_hmap.accheight(oppos.x, oppos.y), oppos.y);

	//corpc fix
	//make sure truck doesn't already have a different driver.
	if(tr->driver == op - g_mv)
		tr->driver = -1;
	/*mv->driver = -1;

	if(mv->mode != NONE)
	mv->mode = AWAITINGDRIVER;*/
}

// do transport drive job
void DoDrive(Mv* op)
{
	/*
	int uID = UnitID(this);

	if(uID == 2)
	{
	Log("u[2]dodrive");
	


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_mv[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velbeforedodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/

	if(!CanDrive(op))
	{
		/*
		if(uID == 2)
		{
		Log("u[2]DISEMBARK");
		
		}
		*/
		//Log("disembark";
		//
		//g_mv[op->target].driver = -1;
		Disembark(op);
		//2016/05/03 reset mode already called in disembark, might be causing the leftover blockages
		//ResetMode(op);
		return;
	}
	/*
	if(uID == 2)
	{
	Log("u[2]dodrive");
	


	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_mv[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velaftercheckava=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/

	if(g_simframe % DRIVE_WORK_DELAY != 0)
	//if(op->cyframes > 0)
		return;

	//if(op->cyframes > 0)
	//	return;

	//last = GetTicks();
	//op->framesleft = DRIVE_WORK_DELAY;

	Mv* tr = &g_mv[op->target];
	Py* py = &g_py[tr->owner];

	//op->belongings[RES_LABOUR] --;
	int subt = imin(op->belongings[RES_LABOUR], WORK_RATE);
	op->belongings[RES_LABOUR] -= subt;

	//py->global[RES_DOLLARS] -= tr->opwage;
	//op->belongings[RES_DOLLARS] += tr->opwage;
	py->global[RES_DOLLARS] -= py->truckwage * subt / WORK_RATE;
	py->gnp += py->truckwage * subt / WORK_RATE;
	op->belongings[RES_DOLLARS] += py->truckwage * subt / WORK_RATE;
	op->incomerate += py->truckwage * subt / WORK_RATE;

	TrCycleHist* trch = &*tr->cyclehist.rbegin();
	trch->pay += py->truckwage * subt / WORK_RATE;
	
#ifdef PYG
	PyGraph* pyg = &g_pygraphs[tr->owner][PYGRAPH_DREXP];
	*pyg->fig.rbegin() += py->truckwage;
	pyg = &g_pygraphs[tr->owner][PYGRAPH_TOTLABEXP];
	*pyg->fig.rbegin() += py->truckwage;
#endif
	
	Py* demp = NULL;
	int dempi = -1;

	if(tr->targtype == TARG_BL)
	{
		Bl* b = &g_bl[tr->target];
		if(b->cyclehist.size())
		{
			CycleHist* lastch = &*b->cyclehist.rbegin();
			lastch->cons[RES_DOLLARS] += py->truckwage;
		}
		
		b->fixcost += py->truckwage * subt / WORK_RATE;

		dempi = b->owner;
		demp = &g_py[b->owner];
	}
	else if(tr->targtype == TARG_CD)
	{
		CdTile* ctile = GetCd(tr->cdtype, tr->target, tr->target2, ecfalse);
		
		dempi = ctile->owner;
		demp = &g_py[ctile->owner];
	}

	//how could i forget 2015/11/16
	if(demp /* && 
		tr->owner != dempi */)
	{
		demp->global[RES_DOLLARS] -= py->transpcost;
		py->global[RES_DOLLARS] += py->transpcost;
		trch->earn += py->transpcost;
	}

	//TODO in RecStats and RecPyStats add a blank list item for starting cycle

	//LogTransx(truck->owner, -p->truckwage, "driver wage");

#ifdef LOCAL_TRANSX
	if(truck->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -py->truckwage * subt / WORK_RATE);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		NewTransx(tr->cmpos, &transx);
		
		if(g_mapview[0].x <= tr->cmpos.x && g_mapview[0].y <= tr->cmpos.y &&
			g_mapview[1].x >= tr->cmpos.x && g_mapview[1].y >= tr->cmpos.y)
			PlayClip(g_trsnd[TRSND_WORK]);
	}

	/*
	if(uID == 2)
	{
	//if(type == TRUCK)
	{
	char msg[128];
	//sprintf(msg, "offgoal=%f vw=%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), MAG_VEC3F(camera.Position() - camera.View()), MAG_VEC3F(camera.Velocity()));
	//Vec3f vw = camera.View() - camera.Position();
	//sprintf(msg, "offgoal=%f vw=%f,%f,%f speed=%f", MAG_VEC3F(camera.Position() - subgoal), vw.x, vw.y, vw.z, MAG_VEC3F(camera.Velocity()));
	Vec3f vel = g_mv[5].camera.Velocity();
	//Vec3f offg = subgoal - camera.Position();
	//sprintf(msg, "offgoal=(%f)%f,%f,%f vw=%f,%f,%f speed=%f,%f,%f", MAG_VEC3F(camera.Position() - subgoal), offg.x, offg.y, offg.z, vw.x, vw.y, vw.z, vel.x, vel.y, vel.z);
	sprintf(msg, "velafterdodrive=%f,%f,%f", vel.x, vel.y, vel.z);
	int unID = 5;

	Log("u["<<unID<<"]: "<<msg);
	

	Chat(msg);
	}
	}*/
}

//inherit the money of dying unit "u"
void Inherit(Mv* mv)
{
	Mv* bestu = NULL;
	int bestutil = -1;

	for(int i=0; i<MOVERS; i++)
	{
		Mv* u2 = &g_mv[i];

		if(!u2->on)
			continue;

		if(u2->type != MV_LABOURER)
			continue;

		if(u2 == u)
			continue;

		const Vec2i cmoff = u2->cmpos - mv->cmpos;
		int dist = FUELHEUR(cmoff);
		int util = PhUtil(u2->belongings[RES_DOLLARS], dist);

		if(bestutil >= 0 && util < bestutil)
			continue;

		bestutil = util;
		bestu = u2;
	}

	if(!bestu)
		return;

	bestu->belongings[RES_DOLLARS] += mv->belongings[RES_DOLLARS];
	mv->belongings[RES_DOLLARS] = 0;
}

void UpdLab(Mv* mv)
{
	StartTimer(TIMER_UPDLAB);

	//return;	//do nothing for now

	mv->jobframes++;
	mv->cyframes--;

	if(g_simframe % CYCLE_FRAMES == 0)
	{
		mv->incomerate = 0;
	}

	if(mv->cyframes < 0)
		mv->cyframes = WORK_DELAY-1;

	//if(mv->cyframes == 0)
	if(g_simframe % FOOD_CONSUM_DELAY_FRAMES == 0)
	{
		mv->belongings[RES_RETFOOD] -= LABOURER_FOODCONSUM;
		CheckMul(u);
	}

	if(mv->belongings[RES_RETFOOD] <= 0)
	{
		char msg[512];
		int cnt = CntMv(MV_LABOURER)-1;
		sprintf(msg, "%s %s (%s %d.)", 
			Time().c_str(), 
			STRTABLE[STR_STARVATION].rawstr().c_str(),
			STRTABLE[STR_POPULATION].rawstr().c_str(),
			cnt);
		RichText sr;
		sr.part.push_back(UStr(msg));
		//SubmitConsole(&sr);
		AddNotif(&sr);
		Inherit(u);
		mv->destroy();

		if(cnt <= 0)
			Click_Pause();

		return;
	}

#if 0
	//if(g_simframe % U_AI_DELAY != 0)
	if(mv->cyframes > 0)
		return;
#endif
	
	int ui = u - g_mv;

	//if(mv->cyframes == 0)
	//if((ui + g_simframe) % 32 == 0)
	switch(mv->mode)
	{
	case UMODE_NONE:
	{
		
		//if(mv->cyframes)
		//	return;

		if(!mv->cyframes)
		{
			if(NeedFood(u) /* && rand()%5==1 */ && FindFood(u))
			{	/*
				if(uID == 2)
				{
				Log("u[2]findfood");
			
				}*/
			}
			else if(NeedRest(u))
			{	/*
				if(uID == 2)
				{
				Log("u[2]needrest");
			
				}*/
				/*
				if(UnitID(this) == 0)
				{
				Log("0 needrest");
			
				}
				*/
				if(mv->home >= 0)
				{	/*
					if(UnitID(this) == 0)
					{
					Log("home >= 0");
				
					}
					*/
					//Chat("go home");
					GoHome(u);
				}
				else
				{
					/*
					if(UnitID(this) == 0)
					{
					Log("findrest");
				
					}*/

					//Chat("find rest");
					if(!FindRest(u))
					{
						mv->belongings[RES_LABOUR] += WORK_RATE;

						goto checkjob;
	#if 0
						if(mv->belongings[RES_LABOUR] > 0)
						{	/*
							if(uID == 2)
							{
							Log("u[2]findjob");
			
							}*/
							if(!FindJob(u)
							   && mv->path.size() == 0 && mv->tpath.size() == 0 //TODO is this necessary? it probably stops workers from finding jobs. edit: shouldn't be run if FindJob returns ectrue.
							   )
							{
				#if 1
								unsigned int ui = u - g_mv;
								//if(rand()%(FRAME_RATE*2) == 1)
								//if( (g_simframe + mv->cmpos.x + mv->cmpos.y * g_mapsz.x*TILE_SIZE) % (SIM_FRAME_RATE * 30) == 1 )
								//if( (g_simframe + ui) % (SIM_FRAME_RATE * 30) == 1 )	//adjust now that it only execs when cyframe==0
				
								{
									//move randomly?
									//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					
									//set it in a new direction (out of 8 directions) every 3 simframes
									//unsigned int timeoff = (g_simframe % (8 * 3)) / 3;
									unsigned int timeoff = ( (g_simframe + ui) % (8 * 3)) / 3;
									//follow goal after subgoal; don't change goal (should be original position, since mode is none)
									mv->subgoal /*= mv->goal*/ = mv->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 2;
									mv->path.push_back( mv->goal );
								}
				#endif
							}
						}
	#endif
					}
					else
						return;
				}
			}
			else
				goto checkjob;
		}
		
checkjob:

		if(mv->jobframes >= LOOKJOB_DELAY_MAX)
		{
			if(mv->belongings[RES_LABOUR] > 0)
			{	/*
				if(uID == 2)
				{
				Log("u[2]findjob");
			
				}*/
				if(!FindJob(u)
				   && mv->path.size() == 0 && mv->tpath.size() == 0 //TODO is this necessary? it probably stops workers from finding jobs. edit: shouldn't be run if FindJob returns ectrue.
				   )
				{
	#if 0
					unsigned int ui = u - g_mv;
					//if(rand()%(FRAME_RATE*2) == 1)
					//if( (g_simframe + mv->cmpos.x + mv->cmpos.y * g_mapsz.x*TILE_SIZE) % (SIM_FRAME_RATE * 30) == 1 )
					//if( (g_simframe + ui) % (SIM_FRAME_RATE * 30) == 1 )	//adjust now that it only execs when cyframe==0
				
					{
						//move randomly?
						//goal = camera.Position() + Vec3f(rand()%TILE_SIZE - TILE_SIZE/2, 0, rand()%TILE_SIZE - TILE_SIZE/2);
					
						//set it in a new direction (out of 8 directions) every 3 simframes
						//unsigned int timeoff = (g_simframe % (8 * 3)) / 3;
						unsigned int timeoff = ( (g_simframe + ui) % (8 * 3)) / 3;
						//follow goal after subgoal; don't change goal (should be original position, since mode is none)
						mv->subgoal /*= mv->goal*/ = mv->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 4;
						mv->path.clear();
						mv->path.push_back( mv->goal );
					}
	#endif
				}
			}
		}
	} break;

	case UMODE_GOCSTJOB:
		GoCstJob(u);
		break;
	case UMODE_CSTJOB:
		if(mv->cyframes)
			return;
		DoCstJob(u);
		break;
	case UMODE_GOBLJOB:
		GoBlJob(u);
		break;
	case UMODE_BLJOB:
		if(mv->cyframes)
			return;
		DoBlJob(u);
		break;
	case UMODE_GOCDJOB:
		GoCdJob(u);
		break;
	case UMODE_CDJOB:
		if(mv->cyframes)
			return;
		DoCdJob(u);
		break;
	case UMODE_GOSHOP:
		GoShop(u);
		break;
	case UMODE_GOREST:
		GoRest(u);
		break;
	case UMODE_SHOPPING:
		if(mv->cyframes)
			return;
		DoShop(u);
		break;
	case UMODE_RESTING:
		if(mv->cyframes)
			return;
		DoRest(u);
		break;
	case UMODE_GODRIVE:
		GoDrive(u);
		break;
	case UMODE_DRIVE:
		if(mv->cyframes)
			return;
		DoDrive(u);
		break;
	default:
		break;
	}


	StopTimer(TIMER_UPDLAB);
}

void UpdLab2(Mv* mv)
{
	switch(mv->mode)
	{
	case UMODE_GODRIVE:
	case UMODE_GOBLJOB:
	case UMODE_GOCDJOB:
	case UMODE_GOCSTJOB:
		if(mv->pathblocked)	//anticlump
			ResetMode(u);
		break;
	default:
		break;
	}
}
