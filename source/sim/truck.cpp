













#include "truck.h"
#include "unit.h"
#include "transport.h"
#include "mvtype.h"
#include "building.h"
#include "bltype.h"
#include "conduit.h"
#include "simdef.h"
#include "../econ/utility.h"
#include "umove.h"
#include "../render/transaction.h"
#include "conduit.h"
#include "../sim/job.h"
#include "../econ/demand.h"
#include "../path/pathjob.h"
#include "../gui/layouts/chattext.h"
#include "../math/vec2i.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "simflow.h"
#include "../ai/ai.h"

short g_trsnd[TR_SOUNDS] = {-1,-1,-1};

ecbool FindFuel(Mv* mv)
{
	Vec2i bcmpos;
	int bestbi = -1;
	int bestutil = -1;

	CdTile* rtile = GetCd(CD_ROAD, mv->cmpos.x/TILE_SIZE, mv->cmpos.y/TILE_SIZE, ecfalse);
	short roadnet = rtile->netw;

	//InfoMess("ff", "ff?");

	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];

		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		BlType* bt = &g_bltype[b->type];

		if(bt->output[RES_RETFUEL] <= 0)
			continue;

		Py* py = &g_py[b->owner];

		if(b->stocked[RES_RETFUEL] + py->global[RES_RETFUEL] <= 0)
			continue;

		ecbool samenet = ecfalse;

		for(std::list<short>::iterator rnet=b->roadnetw.begin(); rnet!=b->roadnetw.end(); rnet++)
		{
			if(*rnet == roadnet)
			{
				samenet = ectrue;
				break;
			}
		}

		if(!samenet)
			continue;

		//if(b->price[RES_RETFUEL] > mv->belongings[RES_DOLLARS])
		//	continue;

		bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		//thisutil = MAG_VEC3F(pos - camera.Position()) * p->price[CONSUMERGOODS];
		const Vec2i cmoff = bcmpos - mv->cmpos;
		int cmdist = FUELHEUR(cmoff);
		int thisutil = PhUtil(b->price[RES_RETFUEL], cmdist);

		if(bestutil > thisutil && bestbi >= 0)
			continue;

		bestbi = bi;
		bestutil = thisutil;
	}

	if(bestbi < 0)
		return ecfalse;

	ResetGoal(u);
	mv->fuelstation = bestbi;
	mv->mode = UMODE_GOREFUEL;
	Bl* b = &g_bl[mv->fuelstation];
	mv->goal = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;


#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset goal track u 1", "reset goal track u 1");
	}
#endif

	//Pathfind();
	//float pathlen = pathlength();
	//Log("found food path length = "<<pathlen<<" ("<<(pathlen/TILE_SIZE)<<" tiles)");
	//

	return ectrue;
}

ecbool NeedFuel(Mv* mv)
{
	if(mv->belongings[RES_RETFUEL] < STARTING_FUEL*3/4)
		return ectrue;

	return ecfalse;
}

void GoSup(Mv* mv)
{
	//nothing here
}

void GoDemB(Mv* mv)
{
#if 0
	if(mode == GOINGTODEMANDERB)
	{
		Bl* b = &g_bl[target];

		if(!g_diplomacy[b->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMROAD)
	{
		CRoad* road = RoadAt(target, target2);

		if(!g_diplomacy[road->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPOWL)
	{
		CPowerline* powl = PowlAt(target, target2);

		if(!g_diplomacy[powl->owner][owner])
			ResetMode();
	}
	else if(mode == GOINGTODEMPIPE)
	{
		CPipeline* pipe = PipeAt(target, target2);

		if(!g_diplomacy[pipe->owner][owner])
			ResetMode();
	}
#endif

	//for building's we check for arrival upon a collision in MoveUnit()
	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
	//if(CheckIfArrived(u))
	//	OnArrived(u);

}

void GoRefuel(Mv* mv)
{
#if 0
	Bl* b = &g_bl[fuelStation];

	if(!g_diplomacy[b->owner][owner])
		ResetMode();
#endif

	//if(driver < 0)
	//	mode = AWAITINGDRIVER;
}

void DoneRefuel(Mv* mv)
{
	//Log("after bid done refueling";
	//

	mv->fuelstation = -1;
	/*
	if(target2 >= 0)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_bl[target].pos;
	}
	else if(target >= 0)
	{
	mode = GOINGTODEMANDERB;
	goal = g_bl[target].pos;
	}*//*
	if(transportAmt > 0)
	{
	if(targtype == GOINGTODEMANDERB)
	{
	mode = GOINGTODEMANDERB;
	goal = g_bl[target].pos;
	}
	else if(targtype == GOINGTODEMROAD)
	{
	mode = GOINGTODEMROAD;
	goal = RoadPosition(target, target2);
	}
	else if(targtype == GOINGTODEMPIPE)
	{
	mode = GOINGTODEMPIPE;
	goal = PipelinePhysPos(target, target2);
	}
	else if(targtype == GOINGTODEMPOWL)
	{
	mode = GOINGTODEMPOWL;
	goal = PowerlinePosition(target, target2);
	}
	else
	{
	mode = targtype;
	ResetGoal();
	Pathfind();
	}
	}
	else if(targtype != NONE)
	{
	mode = GOINGTOSUPPLIER;
	goal = g_bl[supplier].pos;
	}
	else*/
	{
		ResetMode(u);
		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);


#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 1", "reset mode track u 1");
	}
#endif
	}
	//else
	//	ResetMode();
}

void DoRefuel(Mv* mv)
{
	//InfoMess("do ref", "refling");

	if(mv->belongings[RES_RETFUEL] >= STARTING_FUEL)
	{
		DoneRefuel(u);
		return;
	}

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(mv->cyframes != 0)
	//	return;

	//last = GetTicks();

	Bl* b = &g_bl[mv->fuelstation];
	Py* up = &g_py[mv->owner];
	Py* bp = &g_py[b->owner];

	CycleHist* lastch = &*b->cyclehist.rbegin();

	if(b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL] < 0)
	{
		DoneRefuel(u);
		return;
	}

	int req = STARTING_FUEL - mv->belongings[RES_RETFUEL];
	int got = req;

	if(b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL] < req)
		got = b->stocked[RES_RETFUEL] + bp->global[RES_RETFUEL];

	lastch->cons[RES_RETFUEL] += got;	//corpd fix xp

	int paid;

	if(mv->owner != b->owner)
	{
		//LogTransx(owner, -bp->price[ZETROL], "buy fuel");
		//LogTransx(b->owner, bp->price[ZETROL], "buy fuel");

		if(up->global[RES_DOLLARS] < b->price[RES_RETFUEL])
		{
			Bankrupt(mv->owner, "fuel costs");
			DoneRefuel(u);
			return;
		}

		int canafford = got;

		if(b->price[mv->cargotype] > 0)
			canafford = up->global[RES_DOLLARS] / b->price[RES_RETFUEL];

		if(canafford < got)
			got = canafford;

		paid = got * b->price[RES_RETFUEL];

		up->global[RES_DOLLARS] -= paid;
		bp->global[RES_DOLLARS] += paid;
		up->gnp += paid;
		bp->gnp += paid;

		lastch->prod[RES_DOLLARS] += paid;	//corpd fix xp

#if 0
		up->recentth.consumed[CURRENC] += paid;
		bp->recentph[ZETROL].earnings += paid;
#endif

		if(up->global[RES_DOLLARS] <= 0)
			Bankrupt(mv->owner, "fuel costs");
	}

	if(bp->global[RES_RETFUEL] >= got)
		bp->global[RES_RETFUEL] -= got;
	else
	{
		int subgot = got;
		subgot -= bp->global[RES_RETFUEL];
		bp->global[RES_RETFUEL] = 0;

		b->stocked[RES_RETFUEL] -= got;
		bp->local[RES_RETFUEL] -= got;
	}

#if 0
	b->recenth.consumed[RES_RETFUEL] += got;
#endif
	mv->belongings[RES_RETFUEL] += got;

	//b->Emit(BARREL);
#ifdef LOCAL_TRANSX
	if(
		b->owner == g_localP &&
		b->owner != mv->owner)
		NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), CURRENC, paid, ZETROL, -got);
#endif
	if(
#ifdef LOCAL_TRANSX
		owner == g_localP &&
#endif
		b->owner != mv->owner)
	{
		RichText transx;

		{
			Resource* r = &g_resource[RES_RETFUEL];
			char numpart[128];
			sprintf(numpart, "%+d", got);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}
		{
			Resource* r = &g_resource[RES_DOLLARS];
			char numpart[128];
			sprintf(numpart, "%+d", -paid);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		NewTransx(mv->cmpos, &transx);

		//NewTransx(Vec2i(b->tpos.x*TILE_SIZE, b->tpos.y*TILE_SIZE), ZETROL, got, CURRENC, -paid);
	}

	DoneRefuel(u);
}

void AtDemB(Mv* mv)
{
	//LastNum("at demander b");

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//if(mv->cyframes != 0)
	//	return;

	Bl* b = &g_bl[mv->target];

	if(b->finished)
		//b->stock[mv->cargotype] += 1.0f;
			b->stocked[mv->cargotype] += mv->cargoamt;
	else
	{
		//b->conmat[mv->cargotype] += 1.0f;
		b->conmat[mv->cargotype] += mv->cargoamt;
		b->checkconstruction();
	}

#ifdef LOCAL_TRANSX
	if(b->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[mv->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", mv->cargoamt);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		NewTransx(mv->cmpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);
	}

	//mv->cargoamt -= 1;
	mv->cargoamt = 0;

	//char msg[128];
	//sprintf(msg, "unloaded %0.2f/%0.2f %s", 1.0f, transportAmt, g_resource[transportRes].name);
	//Chat(msg);

	if(mv->cargoamt <= 0)
	{
		b->transporter[mv->cargotype] = -1;
		ResetMode(u);
		
		std::list<TrCycleHist>::reverse_iterator chit = mv->cyclehist.rbegin();
		TrCycleHist* lastch = &*chit;
		chit->jobsfini++;

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 2", "reset mode track u 2");
	}
#endif
	}
	
	if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
		g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);
}

void DoneAtSup(Mv* mv)
{
	//Log("after bid done at sup";
	//

	//LastNum("dat sup 1");

	if(mv->cargoamt <= 0)
	{
		ResetMode(u);
		return;
	}
	//LastNum("dat sup 2");

	Bl* supb = &g_bl[mv->supplier];
	//supb->transporter[mv->cargotype] = -1;
	//mv->mode = mv->targtype;

	//LastNum("dat sup 3");
	if(mv->targtype == TARG_BL)
	{
		Bl* demb = &g_bl[mv->target];
		mv->goal = demb->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
		mv->mode = UMODE_GODEMB;
	}
	else if(mv->targtype == TARG_CD)
	{
		CdType* ctype = &g_cdtype[mv->cdtype];
		//CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
		mv->goal = Vec2i(mv->target, mv->target2) * TILE_SIZE + ctype->physoff;
		mv->mode = UMODE_GODEMCD;
	}
	//LastNum("dat sup 4");

	NewJob(UMODE_GODRIVE, (int)(u-g_mv), -1, CD_NONE);
	//LastNum("dat sup 5");
	
	if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
		g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
		PlayClip(g_trsnd[TRSND_DONEJOB]);
}

void AtSup(Mv* mv)
{
	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	//LastNum("at sup 1");

	//if(mv->cyframes != 0)
	//	return;

	Bl* supb = &g_bl[mv->supplier];
	Bl* demb;
	Py* demp = NULL;
	int supplayer = supb->owner;
	Py* supp = &g_py[supb->owner];
	BlType* demt;
	CdTile* demcd;
	//LastNum("at sup 2");

	//char msg[128];
	//sprintf(msg, "at sup targtype=%d @%d,%d", targtype, target, target2);
	//LastNum(msg);

	int demplayer = -1;

	if(mv->targtype == TARG_BL)
	{
		demb = &g_bl[mv->target];
		demplayer = demb->owner;
		demp = &g_py[demb->owner];
		demt = &g_bltype[demb->type];
	}
	else if(mv->targtype == TARG_CD)
	{
		demcd = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
		demplayer = demcd->owner;
		demp = &g_py[demcd->owner];
	}

	//LastNum("at sup 3");
	//if(supb->stock[transportRes] + supp->global[transportRes] <= 0.0f)
	//	DoneAtSupplier();

	//LastNum("at sup 1");

	//if(demb->stock[transportRes] + demp->global[transportRes] + transportAmt >= demt->input[transportRes])
	if(mv->cargoamt >= mv->cargoreq)
	{
		DoneAtSup(u);
		return;
	}

	//LastNum("at sup 2");
	//LastNum("at sup 4");

	//float required = demt->input[transportRes] - transportAmt - demb->stock[transportRes] - demp->global[transportRes];
	int reqnow = mv->cargoreq - mv->cargoamt;
	int got = reqnow;

	if(supb->stocked[mv->cargotype] + supp->global[mv->cargotype] < reqnow)
		got = supb->stocked[mv->cargotype] + supp->global[mv->cargotype];

	int paidtosup = 0;	//paid to supplier
	int paidtoimst = 0;	//paid to import state
	int paidtoexst = 0;	//paid to export state
	int paidtotal = 0;	//total paid
	int initprice = supb->price[mv->cargotype];	//initial price
	int imtariffprice = 0;	//import tariff component of price
	int extariffprice = 0;	//export tariff component of price
	int effectprice = initprice;	//effective price

	//supplier and demander states (export and import)
	//firms and states are grouped in pairs of (FIRMSPERSTATE+1)
	int exsti = (int)( supplayer / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
	int imsti = (int)( demplayer / (FIRMSPERSTATE+1) ) * (FIRMSPERSTATE+1);
	Py* exst = &g_py[exsti];
	Py* imst = &g_py[imsti];

	//LastNum("at sup 5");
	//LastNum("at sup 3");
	//if(targtype == GOINGTODEMANDERB)
	{
		//if(demb->owner != supb->owner)
		if(demp != supp)
		{
			//if trade between different countries
			if(imsti != exsti)
			{
				if(imst->protectionism)
				{
					imtariffprice = initprice * imst->imtariffratio / RATIO_DENOM;
					effectprice += imtariffprice;
				}

				if(exst->protectionism)
				{
					extariffprice = initprice * exst->extariffratio / RATIO_DENOM;
					effectprice += extariffprice;
				}
			}

			if(demp->global[RES_DOLLARS] < effectprice)
			{
				char reason[64];
				Resource* r = &g_resource[mv->cargotype];
				sprintf(reason, "buying %s", r->name.c_str());
				Bankrupt(demplayer, reason);
				DoneAtSup(u);
				return;
			}

			int canafford = got;

			//LastNum("at sup 5.2");
			if(supb->price[mv->cargotype] > 0)
				canafford = demp->global[RES_DOLLARS] / effectprice;

			if(canafford < got)
				got = canafford;
			//LastNum("at sup 4");

			paidtosup = got * initprice;
			paidtoexst = got * extariffprice;
			paidtoimst = got * imtariffprice;
			paidtotal = paidtosup + paidtoexst + paidtoimst;

			//LastNum("at sup 5.3");
			demp->global[RES_DOLLARS] -= paidtotal;
			supp->global[RES_DOLLARS] += paidtosup;
			exst->global[RES_DOLLARS] += paidtoexst;
			imst->global[RES_DOLLARS] += paidtoimst;

			if(mv->targtype == TARG_BL)
			{
				demb->varcost[mv->cargotype] += paidtotal;
			}

			//TODO GDP for countries, throughput for firms
			demp->gnp += paidtotal;
			supp->gnp += paidtotal;
			
#ifdef PYG
			PyGraph* pyg = &g_pygraphs[demplayer][PYGRAPH_DREXP];
			*pyg->fig.rbegin() += paid;
			pyg = &g_pygraphs[demplayer][PYGRAPH_TOTLABEXP];
			*pyg->fig.rbegin() += paid;
#endif

#if 0
			supb->recenth.produced[CURRENC] += paid;
			if(targtype == GOINGTODEMANDERB)
				demb->recenth.consumed[CURRENC] += paid;
			else
				demp->recentth.consumed[CURRENC] += paid;
			supp->recentph[transportRes].earnings += paid;
#endif

			//LastNum("at sup 5.4");
			//char msg[128];
			//sprintf(msg, "purchase %s", g_resource[transportRes].name);
			//LogTransx(supb->owner, paid, msg);
			//LogTransx(demplayer, -paid, msg);
		}
	}
	//LastNum("at sup 6");

	//LastNum("at sup 5");
	if(supp->global[mv->cargotype] >= got)
		supp->global[mv->cargotype] -= got;
	else
	{
		//LastNum("at sup 6");
		//float subgot = got;	//corpd fix !!!!!
		int subgot = got;
		subgot -= supp->global[mv->cargotype];
		supp->global[mv->cargotype] = 0;

		supb->stocked[mv->cargotype] -= subgot;
		supp->local[mv->cargotype] -= subgot;
	}

	CycleHist *supch = &*supb->cyclehist.rbegin();
	supch->cons[mv->cargotype] += got;
	
	if(demp != supp)
	{
		//is the demander a building?
		if(mv->targtype == TARG_BL)
		{
			//2015/11/16
			if(demb->finished)
			{
				CycleHist *demch = &*demb->cyclehist.rbegin();
				demch->cons[RES_DOLLARS] += paidtotal;
			}
		}

		supch->prod[RES_DOLLARS] += paidtosup;
	}
	//TODO rename "cons" "prod"
	//TODO truck expenses

	//LastNum("at sup 7");
	//LastNum("at sup 7");
	mv->cargoamt += got;
#if 0
	supb->recenth.consumed[mv->cargotype] += got;
#endif

	Vec2f transxpos = supb->drawpos;

#ifdef LOCAL_TRANSX
	if(supb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, CURRENC, paid, transportRes, -got);
	if(demb->owner == g_localP && demb->owner != supb->owner)
		NewTransx(transxpos, transportRes, got, CURRENC, -paid);
#else
	{
		RichText transx;

		{
			Resource* r = &g_resource[mv->cargotype];
			char numpart[128];
			sprintf(numpart, "%+d", -got);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		//NewTransx(transxpos, &transx); //TODO paid and show transx

		//NewTransx(transxpos, mv->cargotype, -got);
	}
#endif

	//LastNum("at sup 8");
	DoneAtSup(u);
	//LastNum("at sup 9");
}

void GoDemCd(Mv* mv)
{
	//transporters can only deliver a certain resource to a building/conduit one at a time
	//(two transporters can't deliver the same res type). so we don't need to check if somebody
	//already delivered enough. we just need to check if we've arrived at a conduit because
	//we don't collide with them.
	if(CheckIfArrived(u))
		OnArrived(u);
}

void AtDemCd(Mv* mv)
{
	//LastNum("at demander b");

	//if(GetTicks() - last < WORK_DELAY)
	//	return;

	if(mv->cyframes != 0)
		return;

	//last = GetTicks();

	CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);

	ctile->conmat[mv->cargotype] += 1;
	mv->cargoamt -= 1;

#ifdef LOCAL_TRANSX
	if(r->owner == g_localP)
#endif
	{
		RichText transx;

		{
			Resource* r = &g_resource[mv->cargotype];
			char numpart[128];
			//sprintf(numpart, "%+d", mv->cargoamt);
			sprintf(numpart, "%+d", 1);
			transx.part.push_back( RichPart( numpart ) );
			transx.part.push_back( RichPart( RICH_ICON, r->icon ) );
			//transx.part.push_back( RichPart( r->name.c_str() ) );
			transx.part.push_back( RichPart( " \n" ) );
		}

		CdType* ct = &g_cdtype[mv->cdtype];
		Vec2i cmpos = Vec2i(mv->target*TILE_SIZE + ct->physoff.x, mv->target2*TILE_SIZE + ct->physoff.y);

		NewTransx(cmpos, &transx);

		//NewTransx(b->pos, transportRes, 1.0f);
		//NewTransx(b->pos, transportRes, transportAmt);
		
		if(g_mapview[0].x <= mv->cmpos.x && g_mapview[0].y <= mv->cmpos.y &&
			g_mapview[1].x >= mv->cmpos.x && g_mapview[1].y >= mv->cmpos.y)
			PlayClip(g_trsnd[TRSND_DONEJOB]);
	}

	if(mv->cargoamt <= 0)
	{
		ctile->transporter[mv->cargotype] = -1;
		ResetMode(u);
		
		std::list<TrCycleHist>::reverse_iterator chit = mv->cyclehist.rbegin();
		TrCycleHist* lastch = &*chit;
		chit->jobsfini++;
	}

	ctile->checkconstruction(mv->cdtype);
}

void UpdTruck(Mv* mv)
{
	StartTimer(TIMER_UPDTRUCK);
	//return;	//do nothing for now
	
	Py* py = &g_py[mv->owner];

	if(py->ai)
	{
		//mv->jobframes ++;
		int ui = u - g_mv;
		if((g_simframe+ui) % CYCLE_FRAMES == 0)
		//if(mv->cyframes == 0)
		{
			//AdjTrPrWg(py, u);
		
			mv->cyclehist.push_back(TrCycleHist());

			while(mv->cyclehist.size() > 100)
				mv->cyclehist.erase(mv->cyclehist.begin());
		
			TrCycleHist* trch = &*mv->cyclehist.rbegin();

			trch->opwage = py->truckwage;
			trch->trprice = py->transpcost;
		}
	}

	if(mv->mode != UMODE_NONE)
	{
		int m = PATHHEUR((mv->cmpos - mv->prevpos));

		mv->cyframes--;

		if(mv->cyframes < 0)
			mv->cyframes = WORK_DELAY-1;

		//mv->cyframes += m;

		//if(mv->cyframes >= TRUCK_DIST)
		if((g_simframe * TRUCK_SPEED) % TRUCK_DIST < m)
		{
			mv->belongings[RES_RETFUEL] -= TRUCK_CONSUMPRATE;
			//mv->cyframes = 0;
		}

		//prevent job from expiring from inactivity ("stuck")
		if(mv->cmpos != mv->prevpos)
			mv->jobframes = TBID_DELAY;

		Mv* targu = NULL;

		if(mv->driver >= 0)
			targu = &g_mv[mv->driver];

#if 0
		//Doesn't work, in situations where
		//approaching driver blocks path.
		if(Trapped(u, targu))
		{
			ResetMode(u);
			return;
		}
#endif
	}

	if(mv->belongings[RES_RETFUEL] <= 0)
	{
		char msg[128];
		sprintf(msg, "%s Truck out of fuel! (Remaining %d)", Time().c_str(), CntMv(MV_TRUCK));
		Log(msg);
		Log("\tRemain: %d\r\n", CntMv(MV_TRUCK));
		RichText sr;
		sr.part.push_back(UStr(msg));
		//SubmitConsole(&sr);
		AddNotif(&sr);
		//SubmitConsole(&sr);
		AddNotif(&sr);
		//ResetMode(u);
		mv->destroy();
		return;
	}

#if 0
	if(mv->path.size() > 0)
	{
		InfoMess("trp", "trp");
	}
#endif

	//if(NeedFuel(u))
	//	InfoMess("nf", "nf");

	switch(mv->mode)
	{
	case UMODE_NONE:
		{
			if(NeedFuel(u) && FindFuel(u))
			{
				//InfoMess("ff", "ff");
				//Log("find fuel");
				//
			}
			else if(mv->mode == UMODE_NONE /* && mv->target < 0 */)
			{
				//	FindDemander();
				if(mv->jobframes >= TBID_DELAY)
				{
					//g_freetrucks.push_back(UnitID(this));
					//Log("free truck");
					//
				}
				mv->jobframes ++;
			}
		} break;

	case UMODE_GOSUP:
		GoSup(u);
		break;

	case UMODE_GODEMB:
		GoDemB(u);
		break;

	case UMODE_GOREFUEL:
		GoRefuel(u);
		break;

	case UMODE_REFUELING:
		DoRefuel(u);
		break;

	case UMODE_ATDEMB:
		AtDemB(u);
		break;

	case UMODE_ATSUP:
		AtSup(u);
		break;

	case UMODE_GODEMCD:
		GoDemCd(u);
		break;

	case UMODE_ATDEMCD:
		AtDemCd(u);
		break;

	default:
		break;
	}

	StopTimer(TIMER_UPDTRUCK);
}
