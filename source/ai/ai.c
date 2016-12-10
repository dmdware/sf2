









#include "../sim/player.h"
#include "ai.h"
#include "../econ/demand.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/build.h"
#include "../sim/simdef.h"
#include "../econ/utility.h"
#include "../sim/unit.h"
#include "../sim/simflow.h"
#include "../net/lockstep.h"
#include "../gui/layouts/chattext.h"
#include "../path/tilepath.h"
#include "../sim/conduit.h"
#include "../net/packets.h"
#include "../gui/widgets/spez/gengraphs.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/messbox.h"

void UpdAI()
{
	Py* py;

	/* only host updates AI and issues commands */
	if(g_netmode != NETM_HOST &&
		g_netmode != NETM_SINGLE)
		return;

	for(py=g_py; py<g_py+PLAYERS; ++py)
	{
		if(!py->on)
			continue;

		if(!py->ai)
			continue;

		if(g_simframe - py->lastthink < AI_FRAMES)
			continue;

		py->lastthink = g_simframe;

		UpdAI2(py);
	}
}

/*
Bl proportion number ai build rate

Houses 2 other 1 part

Trial and error wages and prices adjust

Rule if population decreasing then lower prices and increase wages
*/

void UpdAI2(Py* py)
{
#define BUILD_FRAMES		(NETTURN+13)

	unsigned __int64 blphase = g_simframe / BUILD_FRAMES;
	char pyi = py - g_py;
	
	if((blphase+pyi+g_simframe) % PLAYERS == 1)
		AIBuild(p);
	
	if((blphase*pyi+g_simframe) % (PLAYERS*100/60) == 2)
		AIManuf(p);
	
	if((blphase+g_simframe) % PLAYERS == 3)
		BuyProps(p);

	if((blphase+g_simframe/300) % PLAYERS == 3)
		AdjTrPrWg(p, NULL);

	if((blphase+g_simframe/3000) % PLAYERS == 1)
		AdjCs(p);
}

void AIBuyProp(Py* py, 
			   unsigned char propi, 
			   unsigned char proptype, 
			   int tx, 
			   int ty)
{
	BuyPropPacket bpp;
	unsigned char pyi;
	
	pyi = py - g_py;

	bpp.header.type = PACKET_BUYPROP;
	bpp.pi = pyi;
	bpp.proptype = proptype;
	bpp.propi = propi;
	bpp.tx = tx;
	bpp.ty = ty;
	LockCmd((PacketHeader*)&bpp);
}

void BuyProps(Py* py)
{
	Bl* b;
	unsigned char pyi;
	Node* chit;	/* <CycleHist>*/
	CycleHist *lastch, *lastch2;
	Mv* mv;
	CdTile* ctile;
	unsigned char ctype;
	int tx, ty;
	
	pyi = py - g_py;

	for(b=g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;
		
		if(!b->forsale)
			continue;

		if(!b->finished)
		{
			AIBuyProp(p, bi, PROP_BL_BEG + b->type, -1, -1);
			continue;
		}

		if(b->owner == pyi)
			continue;

		if(b->propprice <= 0)
		{
			AIBuyProp(p, bi, PROP_BL_BEG + b->type, -1, -1);
			continue;
		}

		if(b->cyclehist.size < 2)
			continue;
		
		chit = b->cyclehist.tail;
		lastch = (CycleHist*)chit->data;
		chit = chit->prev;
		lastch2 = (CycleHist*)chit->data;

		int profit = lastch->prod[RES_DOLLARS] + 
			lastch2->prod[RES_DOLLARS] - 
			lastch->cons[RES_DOLLARS] -
			lastch2->cons[RES_DOLLARS];

		if((10 + profit) * 2 < b->propprice)
			continue;
		
		AIBuyProp(p, b-g_bl, PROP_BL_BEG + b->type, -1, -1);
	}

	for(mv=g_mv; mv<g_mv+MOVERS; ++mv)
	{
		if(!mv->on)
			continue;

		if(!mv->forsale)
			continue;

		if(mv->owner == pyi)
			continue;

		if(mv->price <= 0 ||
			mv->price <= (p->transpcost - p->truckwage) * 12)
		{
			AIBuyProp(p, mv-g_mv, PROP_U_BEG + mv->type, -1, -1);
		}
	}

	for(ctype=0; ctype<CD_TYPES; ++ctype)
	{
		for(tx=0; tx<g_mapsz.x; ++tx)
		{
			for(ty=0; ty<g_mapsz.y; ++ty)
			{
				ctile = GetCd(ctype, tx, ty, ecfalse);

				if(!ctile->on)
					continue;

				if(ctile->owner == pyi)
					continue;

				if(!ctile->selling)
					continue;
				
				AIBuyProp(py, -1, PROP_CD_BEG + ctype, tx, ty);
			}
		}
	}
}

int CountB()
{
	Bl* b;
	int c;
	
	c = 0;

	for(b+g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;

		++c;
	}

	return c;
}

//TODO Py* p -> Py* py
void AdjTrPrWg(Py* py, Mv* mv)
{
	unsigned char pyi,
		jobsgiven, jobsfini,
		lastjobs;
	int newtrwg, c,
		earn, earn2,
		trprice, trprice2, 
		spend, spend2;
	Mv* mv;
	Node* chit; /* TrCycleHist */
	TrCycleHist *lastch, *lastch2;
	int newtrpr,
		ratio, ratio2;
	ecbool pricepositive;
	int pricesign, prch, mult;
	ChValPacket cvp;

	pyi = py - g_py;
	newtrwg = py->truckwage;
	c = 0;
	jobsgiven = 0;
	jobsfini = 0;
	earn = 0;
	earn2 = 0;
	trprice = 0;
	trprice2 = 0;
	spend = 0;
	spend2 = 0;

	for(mv=g_mv; mv<g_mv+MOVERS; ++mv)
	{
		if(!mv->on)
			continue;

		if(mv->owner != pi)
			continue;

		if(mv->cyclehist.size < 3)
			continue;

		chit = mv->cyclehist.tail;

		lastch = (TrCycleHist*)chit->data;
		chit =chit->prev;
		lastch2 = (TrCycleHist*)chit->data;
		
		lastjobs = lastch->jobsgiven;

		jobsgiven = iceil( jobsgiven * c + lastjobs, c+1);
		jobsfini = iceil( jobsfini * c + lastch->jobsfini, c+1);
		
		earn = ( earn * c + lastch->earn ) / (c+1);
		earn2 = ( earn2 * c + lastch2->earn ) / (c+1);
		
		spend = ( spend * c + lastch->pay ) / (c+1);
		spend2 = ( spend2 * c + lastch2->pay ) / (c+1);
		
		trprice = ( trprice * c + lastch->trprice ) / (c+1);
		trprice2 = ( trprice2 * c + lastch2->trprice ) / (c+1);

		++c;
	}
	
	if(!c)
		return;

	{
		if(jobsgiven > jobsfini &&
			g_simframe*321 % 11 == 1 &&
			earn + earn2 >= spend + spend2)
		{
			newtrwg = newtrwg + newtrwg / 5 + 2;
			newtrwg = imin(INST_FUNDS/100, newtrwg);
		}
		else if(!jobsgiven)
			;
		else
		{
			newtrwg = newtrwg - newtrwg / 5;
			newtrwg = imax(10, newtrwg);
		}
	}

	newtrpr = py->transpcost;
	pricepositive = ecfalse;
	
	//if(earn > earn2)
	//	pricepositive = ectrue;
	
	spend = imax(spend, 1);
	spend2 = imax(spend2, 1);
	
	ratio = earn*RATIO_DENOM/spend;
	ratio2 = earn2*RATIO_DENOM/spend2;

	if(ratio > ratio2)
		pricepositive = ectrue;
	else if(ratio == ratio2 && jobsgiven)
		pricepositive = (ecbool)((g_simframe*pi/(CYCLE_FRAMES+1)) % 2);
	else
		pricepositive = ecfalse;

	pricesign = 1;

	if(!pricepositive)
		pricesign = -1;

	prch = trprice2 - trprice;

	mult = 1;

	if(pricepositive)
		mult = 3;

	if(iabs(prch) < 2)
		prch += 2 + prch * 2;

	/* 2016/05/02 trying to reduce driver expenses */
	if(!ratio || !ratio2 || 
		(earn+earn2) < (spend+spend2) ||
		newtrpr <= newtrwg)
	{
		pricesign = 1;
		prch = iabs(prch);
	}

	newtrpr = newtrpr + pricesign * prch * mult / 2;

	newtrwg = imax(MINWAGE, newtrwg);
	newtrwg = imin(INST_FUNDS/100, newtrwg);

	if(newtrwg != py->truckwage)
	{
		//TODO don't do this for every owned player truck
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_TRWAGE;
		cvp.player = py-g_py;
		cvp.bi = -1;
		cvp.value = newtrwg;
		LockCmd((PacketHeader*)&cvp);
	}

	newtrpr = imax(10, newtrpr);
	newtrpr = imin(INST_FUNDS/100, newtrpr);

	if(newtrpr != py->transpcost)
	{
		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_TRPRICE;
		cvp.player = py-g_py;
		cvp.bi = -1;
		cvp.value = newtrpr;
		LockCmd((PacketHeader*)&cvp);
	}
}

void AdjPrWg(Py* py, Bl* b)
{
	Node* chit;	/* CycleHist */
	CycleHist *lastch, *lastch2;
	int profit, profit2,
		earnings, earnings2,
		totprof,
		spend, spend2;
	int rprch[RESOURCES];
	int haverch;
	int wch; /* wage chg */
	/*list of resources this bltype produces*/
	List prr; /* char */
	int ratio[RESOURCES], ratio2[RESOURCES];
	short nlab, nbl, nlabreq;
	ecbool subreqlab;
	BlType* bt;
	char ri;
	ecbool wagepositive, pricepositive[RESOURCES];
	int rearn, rearn2,
		wagesign,
		pricesign[RESOURCES];
	ecbool dowch;
	int nextw;
	ChValPacket cvp;
	char riti;
	Node* rit; /* char prr */ 
	char ri;
	int nrpod, rprodcost;
	char ri2;
	int remfixcost, outearn, basecost, nextrpr;
	ecbool doprch;

	if(b->cyclehist.size < 3)
	{
		Zero(b->varcost);
		b->fixcost = 0;
		return;
	}
	
	/*the completed cycle is [end-1], because tryprod() that calls this
	func pushes a new one for the beginning of the new cycle.*/
	chit = b->cyclehist.tail;
	chit = chit->prev;
	lastch = (CycleHist*)chit->data;
	chit = chit->prev;
	lastch2 = CycleHist*)chit->data;
	
	/* total profits (might be negative) */
	profit = lastch->prod[RES_DOLLARS] - lastch->cons[RES_DOLLARS];
	profit2 = lastch2->prod[RES_DOLLARS] - lastch2->cons[RES_DOLLARS];
	earnings = lastch->prod[RES_DOLLARS];
	earnings2 = lastch2->prod[RES_DOLLARS];
	totprof = profit + profit2;
	spend = lastch->cons[RES_DOLLARS];
	spend2 = lastch2->cons[RES_DOLLARS];

	Zero(rprch);
	haverch = -1;

	List_init(&prr);

	spend = imax(spend, 1);
	spend2 = imax(spend2, 1);

	wch = lastch->wage - lastch2->wage;

	nlab = CntMv(MV_LABOURER);
	nbl = CountB();
	nlabreq = nbl * 30 / BL_TYPES;
	//int nlabreq = nbl * g_nlabperbl;
	subreqlab = (nlab < nlabreq);

	bt = &g_bltype[b->type];

	for(ri=0; ri<RESOURCES; ++ri)
	{
		if(bt->output[ri] > 0)
			List_pushback2(&prr, &ri, sizeof(ri));

		rprch[ri] = lastch->price[ri] - lastch2->price[ri];

		if(rprch[ri] != 0)
			haverch = ri;
	}

	/*
	The general idea is to try adjusting price up,
	then the next cycle down, for the same resource,
	and then move on to the next resource if that fails.
	Then move on to the wage.
	*/

	/*
	Too complicated
	Plan 2
	Just decrease wage and increase all the prices if need more money
	And opposite if can spend
	*/
	
	/* TODO calc earnings from each res using lastch->cons and price, and adj pricepositive for each out res */

	/* positive reinforcement? */
	wagepositive = ecfalse;
	pricepositive[RESOURCES] = {ecfalse};
	
	for(ri=0; ri<RESOURCES; ++ri)
	{
		rearn = lastch->cons[ri] * lastch->price[ri];
		rearn2 = lastch2->cons[ri] * lastch2->price[ri];
		
		ratio[ri] = rearn*RATIO_DENOM/spend;
		ratio2[ri] = rearn2*RATIO_DENOM/spend2;
		
		if(ratio[ri] > ratio2[ri])
			pricepositive[ri] = ectrue;
		else if(ratio[ri] == ratio2[ri] && rearn)
			pricepositive[ri] = (ecbool)((g_simframe*ri/(CYCLE_FRAMES+1)) % 2);
		else
			pricepositive[ri] = ecfalse;
	}

	if(lastch->cons[RES_LABOUR] /* + lastch2->cons[RES_LABOUR] */ + 
		b->stocked[RES_LABOUR] < b->prodlevel * bt->input[RES_LABOUR] / RATIO_DENOM)
	{
		/* TODO this is not enough as the labour might not 
		have been consumed simply because some other requisite 
		res was missing */
		wagepositive = ectrue;
	}

	/* TODO decent options menu with labels and layout instead of using one resizing func
	TODO adjust truck wages */

	/*
	TODO adjust pr wg based on 4 cycles with 2 together, not just 2, 
	because it's erratic and sometimes there's no sales on 1 cycle
	TODO use realistic month timescale, so that it's guaranteed to
	have some transactions, or year for farms, depending on economic 
	cycle length for bl
	*/
	/* TODO make sure conduits connect, because 
	this faulty method failed to connect oil well, 
	which I was running for a day */

	/* TODO ability to make manual deals micromanage instead of AI manager */
	/*
	TODO
	load and unload only needed assets within view
	run on any platform spec even crappy android tablet
	*/

	/*
	If positive, do the same change but more.
	And if negative, try in the opposite direction, but half.
	*/
	wagesign = 1;
	//int pricesign = 1;
	pricesign[RESOURCES] = {1};

	if(!wagepositive)
		wagesign = -1;
	//if(!pricepositive)
	///	pricesign = -1;
	for(ri=0; ri<RESOURCES; ++ri)
	{
		if(pricepositive[ri])
			pricesign[ri] = 1;
		else
			pricesign[ri] = -1;
	}

	/* wage adjustment... */
	dowch = ecfalse;

	/* new wage rule: increase if insufficient labour provided, wagepositive ectrue = need increase wage */

	if(b->prodlevel)
	{
		if(wagepositive)
		{
			nextw = b->opwage + b->opwage / 5;
			dowch = ectrue;
		}
		/* try lowering? */
		else if(lastch->cons[RES_LABOUR])
		{
			nextw = b->opwage - b->opwage / 7;
			dowch = ectrue;
		}
	}

	if(dowch)
	{
		if(nextw < MINWAGE)
			nextw = MINWAGE;
		else if(nextw > INST_FUNDS / 100)
			nextw = INST_FUNDS / 100;

		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_BLWAGE;
		cvp.player = py-g_py;
		cvp.bi = b-g_b;
		cvp.value = nextw;
		LockCmd((PacketHeader*)&cvp);
	}

	/* price adjustments... */
	riti = 0;
	for(rit=prr.head; rit; rit=rit->next, ++riti)
	{
		ri = *(char*)rit->data;
		
		/* add on base cost to final price... */

		nrpod = bt->output[ri] * b->prodlevel / RATIO_DENOM;
		rprodcost = 0;

		/* calculate variable cost of inputs for last cycle's production batch of this output ri */
		for(ri2=0; ri2<RESOURCES; ++ri2)
		{
			/* TODO multiple production chains */

			if(bt->input[ri2] <= 0)
				continue;

			if(b->prodlevel <= 0)
				continue;

			/*
			the problem with this is we don't know which inputs go to which outputs,
			as they are equally necessary for all outputs.
			*/

			rprodcost += b->varcost[ri2];
		}
		
		/* remaining fixed cost not covered by earnings from (other?) sold outputs */
		int remfixcost = 0;

		remfixcost = b->fixcost;
		int outearn = 0;

		for(ri2=0; ri2<RESOURCES; ++ri2)
		{
			/* TODO multiple production chains */

			if(bt->output[ri2] > 0)
			{
				outearn += lastch->cons[ri2] * b->price[ri2];
				//prof = imax(0, prof);
				//remfixcost -= prof;
			}
		}

		outearn = imax(0, outearn - rprodcost);

		/* final base */
		basecost = rprodcost + remfixcost;
		/* basically: basecost = variable input costs + 
		imax(0, (fixed transport cost - imax(0, (output sales earnings - variable input costs)))
		*/

		/* divide cost per sale */
		basecost /= imax(1, lastch->cons[ri]);

		doprch = ecfalse;
		nextrpr = 0;

		if(rprch[ri] != 0)
		{
			nextrpr = b->price[ri] + pricesign[ri] * rprch[ri] * 10 / 9;
			doprch = ectrue;
		}

		/* extra rule for negative reinforcement: try changing one of the prices or 
		the wage in the opposite direction, because there are many factors */
		{
			//2015/11/16/ - pricesign -> + pricesign, isign
			if(iabs(rprch[ri]) > 4)
				nextrpr = b->price[ri] + pricesign[ri] * isign(rprch[ri]) * iceil(b->price[ri], 20);
			else if(rprch[ri] != 0)
				nextrpr = b->price[ri] + pricesign[ri] * isign(rprch[ri]) * iceil(b->price[ri] * 40, 39);
			else
				nextrpr = b->price[ri] + pricesign[ri] * isign(1 - (g_simframe%4)) * 4;

			doprch = ectrue;
		}

		if(doprch)
		{
			if(nextrpr < 1)
				nextrpr = 1;
			else if(nextrpr > INST_FUNDS / 100)
				nextrpr = INST_FUNDS / 100;

			cvp.header.type = PACKET_CHVAL;
			cvp.chtype = CHVAL_BLPRICE;
			cvp.player = py-g_py;
			cvp.bi = b-g_b;
			cvp.value = nextrpr;
			cvp.res = ri;
			LockCmd((PacketHeader*)&cvp);
		}
	}
	Zero(b->varcost);
	b->fixcost = 0;
	List_free(&prr);
}

//2016/05/22 now maximizes continued throughput
void AdjCs(Py* py)
{
	int tx, ty;
	unsigned char cti;
	CdTile* ctile;
	int conwage;
	ChValPacket cvp;
	char pyi;
	
	pyi = py-g_py;

	return;

	for(tx=0; tx<g_mapsz.x; ++tx)
	{
		for(ty=0; ty<g_mapsz.y; ++ty)
		{
			for(cti=0; cti<CD_TYPES; ++cti)
			{
				ctile = GetCd(cti, tx, ty, ecfalse);

				if(ctile->owner != pi)
					continue;

				if(ctile->finished)
					continue;

				conwage = ctile->conwage;

				conwage = conwage * 3/2;
				conwage = imin(conwage, p->global[RES_DOLLARS]/20);

				if(conwage == ctile->conwage)
					continue;

				cvp.header.type = PACKET_CHVAL;
				cvp.chtype = CHVAL_CDWAGE;
				cvp.player = pi;
				cvp.cdtype = cti;
				cvp.x = tx;
				cvp.y = ty;
				cvp.bi = -1;
				cvp.value = conwage;
				LockCmd((PacketHeader*)&cvp);
			}
		}
	}

	/* g_onb todo */
	for(b=g_b; b<g_bl[BUILDINGS]; ++b)
	{
		if(b->owner != pyi)
			continue;

		if(b->finished)
			continue;

		conwage = b->conwage;

		conwage = conwage * 3/2;
		conwage = imin(conwage, p->global[RES_DOLLARS]/20);

		if(conwage == b->conwage)
			continue;

		cvp.header.type = PACKET_CHVAL;
		cvp.chtype = CHVAL_CSTWAGE;
		cvp.player = pyi;
		cvp.cdtype = -1;
		cvp.x = -1;
		cvp.y = -1;
		cvp.bi = b-g_b;
		cvp.value = conwage;
		LockCmd((PacketHeader*)&cvp);
	}
}

void AdjProd(Py* py, Bl* b)
{
	char pyi, bi;
	ChValPacket cvp;
	BlType* bt;
	ecbool stop;
	int value;
	char ri;
	Resource* r;
	CycleHist* lastch;
	int minout, more;

	pyi = p - g_py;
	bi = b-g_b;

	bt = &g_bltype[b->type];
	stop = ectrue;
	value = 0;

	for(ri=0; ri<RESOURCES; ++ri)
	{
		if(bt->output[ri] <= 0)
			continue;

		r = g_resource+ri;

		if(r->capacity)
		{
			lastch = (CycleHist*)b->cyclehist.tail->data;

			if(lastch->prod[ri] > lastch->cons[ri] && 
				b->prodlevel > 0 && 
				lastch->cons[ri] > 0)
			{
				stop = ecfalse;
				minout = iceil(RATIO_DENOM, bt->output[ri]);
				value = imax( value, RATIO_DENOM * lastch->cons[ri] / bt->output[ri] + minout*2 );
			}
			else if(lastch->prod[ri] <= lastch->cons[ri])
			{
				stop = ecfalse;
				minout = iceil(RATIO_DENOM, bt->output[ri]);
				value = imax( value, RATIO_DENOM * lastch->cons[ri] / bt->output[ri] + minout*2 );
			}
		}
		else
		{
			/* 2016/04/26 changed stocking behaviour */
			if(b->stocked[ri] < bt->output[ri] * bt->stockingrate / RATIO_DENOM ||
				!b->stocked[ri])
			{
				stop = ecfalse;
				more = (bt->output[ri] * bt->stockingrate / RATIO_DENOM) - b->stocked[ri];
				minout = iceil(RATIO_DENOM, bt->output[ri]);
				minout = imax( minout, RATIO_DENOM * more / bt->output[ri] );
				value = imax( value, minout );
				break;
			}
		}
	}

	value = imin( value, RATIO_DENOM );

	if( (stop && b->prodlevel == 0) ||
		(!stop && b->prodlevel == value) )
		return;

	cvp.header.type = PACKET_CHVAL;
	cvp.chtype = CHVAL_PRODLEV;
	cvp.bi = bi;
	cvp.value = value;
	cvp.player = pi;
	LockCmd((PacketHeader*)&cvp);
}

void Manuf(char pyi, char mvti)
{
	List manufers; /* char */
	Bl* b;
	BlType* bt;
	Node* mt; /* char */
	char bi;
	short cmi;
	char mi;
	OrderManPacket omp;

	List_init(&manufers);

	for(b=g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;

		if(!b->finished)
			continue;

		/* TODO enemey check
		TODO visibility check */

		bt = g_bltype+b->type;

		for(mt=bt->manuf.head; mt; mt=mt->next)
		{
			if(*(char*)mt->data == mvti)
			{
				bi = b-g_b;
				List_pushback2(&manufers, &bi, sizeof(bi));
				break;
			}
		}
	}

	if(!manufers.size)
		goto clean;

	/* chosen manuf index */
	cmi = g_simframe % manufers.size;
	mi = 0;
	mt = manufers.head;

	while(mt && mi<cmi)
	{
		mt = mt->next;
		++mi;
	}

	omp.header.type = PACKET_ORDERMAN;
	omp.player = pyi;
	omp.bi = *(char*)mt->data;
	omp.mvtype = mvti;
	LockCmd((PacketHeader*)&omp);

clean:
	List_free(&manufers);
}

void AIManuf(Py* py)
{
	short bcnt, mvcount[MV_TYPES];
	char pyi;
	Bl* b;
	Mv* mv;
	MvType* mvt;
	char needed, tomanuf, mvti;

	bcnt = 0;
	pyi = p - g_py;

	for(b=g_b; b<g_b+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;
		++bcnt;
	}

	memset(mvcnt, 0, sizeof(mvcnt));

	for(mv=g_mv; mv<g_mv+MOVERS; ++mv)
	{
		if(!mv->on)
			continue;
		++mvcnt[mv->type];
	}

	mvti = 0;
	for(mvt=g_mvtype; mvt<g_mvtype+MV_TYPES; ++mvt, ++mvti)
	{
		needed = mvt->prop * bcnt / RATIO_DENOM;
		tomanuf = needed - mvcnt[mvti];

		if(tomanuf <= 0)
			continue;

		//for(int j=0; j<tomanuf; j++)
		{
			Manuf(pi, mvti);
		}
	}
}

/*
To calc what bl next and where

For each bl on map multiply by 15 and decrease by radius from spot of bl

Spot with greatest num of all bl types is densest
Will be next spot where no bl placed

Then multiply bl proportion rate num by 15 also and subtract dense tile's num for all bl's of that type, and choose the bltype with greatest
For eg house proportion is 3 and theres only 1 gives 2

15 is some distance num for max city clustering distance that should effect interaction
*/
void AIBuild(Py* py)
{
	Bl* b;
	char pyi;
	int* bldensity;
	BlType* bt;
	Vec2uc tstart, tmin, tmax;
	int tx, ty;
	Vec2uc tspot;
	Vec2c toff
	int tdist, dens, *spoti;
	Vec2uc highspot;
	int high, highscore, score;
	char highbti, bti;
	BlType* bt;
	short bcnt, wcnt;
	Vec2uc tplace;
	PlaceBlPacket pbp;
	char ctype;
	CdType* ct;
	ecbool do1, do2, do3, do4;
	Vec2uc line[2];

	pyi = py-g_py;

	/* check for any unfinished bl's */
	for(b=g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;

		if(b->owner != pyi)
			continue;

		if(!b->finished)
			return;
	}

#define DIST_MULT		30

	bldensity = (int*)malloc(sizeof(int) * (g_mapsz.x+0) * (g_mapsz.y+0));
	memset(bldensity, 0, sizeof(int) * (g_mapsz.x+0) * (g_mapsz.y+0));

	/* TODO only get density for visible tiles, so that each player builds own base */

	/* calc density */
	for(b=g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;

		if(b->owner != pi)
			continue;

		bt = &g_bltype[b->type];
		tstart.x = b->tpos.x;
		tstart.y = b->tpos.y;

		tmin.x = (unsigned char)imax(0, (int)tstart.x - DIST_MULT);
		tmin.y = (unsigned char)imax(0, (int)tstart.y - DIST_MULT);
		tmax.x = (unsigned char)imin((int)g_mapsz.x, (int)tstart.x + DIST_MULT);
		tmax.y = (unsigned char)imin((int)g_mapsz.y, (int)tstart.y + DIST_MULT);

		for(tx=tmin.x; tx<tmax.x; ++tx)
		{
			for(ty=tmin.y; ty<tmax.y; ++ty)
			{
				toff.x = imax(tx,tstart.x) - imin(tx,tstart.x);
				toff.y = imax(ty,tstart.y) - imin(ty,tstart.y);

				tdist = MAG_VEC2I(toff);
				dens = DIST_MULT - tdist;

				if(dens <= 0)
					continue;

				spoti = bldensity + (tx + ty * g_mapsz.x);

				(*spoti) += dens;
			}
		}
	}

	/* add unit's density */
	for(mv+g_mv; mv<g_mv+MOVERS; ++mv)
	{
		if(!mv->on)
			continue;

		if(mv->owner != pi)
			continue;

		tstart.x = mv->cmpos.x / TILE_SIZE;
		tstart.y = mv->cmpos.y / TILE_SIZE;

		spoti = bldensity + ( tstart.x + tstart.y * g_mapsz.x );
		(*spoti) += 1;
	}

	high = -1;

	/* get highest density spot */
	for(tx=0; tx<g_mapsz.x; tx++)
	{
		for(ty=0; ty<g_mapsz.y; ty++)
		{	
			if(!Explored(pi, tx, ty))
				continue;

			spoti = bldensity + ( tx + ty * g_mapsz.x );

			if(*spoti < high)
				continue;

			high = *spoti;
			highspot.x = tx;
			highspot.y = ty;
		}
	}

	free(bldensity);

	highscore = 0;
	highbti = -1;
	bti=0;

	/* calc bltype to build */
	for(bty=g_bltype; bt<g_bltype+BL_TYPES; ++bt, ++bti)
	{
		score = 0;

		/* go through each bl of that type */
		for(b+g_bl; b<g_bl+BUILDINGS; ++b)
		{
			if(!b->on)
				continue;

			if(b->type != bti)
				continue;

			tstart.x = b->tpos.x;
			tstart.y = b->tpos.y;

			toff.x = imax(highspot.x, tstart.x) - imin(highspot.x, tstart.x);
			toff.y = imax(highspot.y, tstart.y) - imin(highspot.y, tstart.y);

			tdist = MAG_VEC2I(toff);
			tdist = imin(tdist, DIST_MULT);

			dens = DIST_MULT - tdist;
			dens = imax(0, dens);

			/* add contribution of that bl for this tile */
			score -= dens;
		}

		if(bt->prop == 0)
			continue;

		score /= bt->prop;

		if(score < highscore &&
			highbti >= 0)
			continue;

		highscore = score;
		highbti = bti;
	}

	if(highbti < 0)
		return;

	/* check if proportional to workers */

	bcnt = 0;

	for(b+g_bl; b<g_bl+BUILDINGS; ++b)
	{
		if(!b->on)
			continue;

		if(b->type != highbti)
			continue;

		++bcnt;
	}

	wcnt = CntMv(MV_LABOURER);
	bt = g_bltype+highbti;

	/* RATIO_DENOM * 1*1 */
	if(wcnt < bcnt * bt->wprop / RATIO_DENOM)
		return;

	/* find a place to build */
	if(!PlaceBAb4(highbti, highspot, &tplace))
	{
		p->lastthink += CYCLE_FRAMES*4;
		return;
	}

	pbp.header.type = PACKET_PLACEBL;
	pbp.btype = highbti;
	pbp.player = pi;
	pbp.tpos = tplace;
	LockCmd((PacketHeader*)&pbp);

	/* connect conduits */

	for(ctype=0; ctype<CD_TYPES; ctype++)
	{
		for(ri=0; ri<RESOURCES; ri++)
		{
			r = g_resource+ri;

			if(r->conduit != ctype)
				continue;

			if(bt->input[ri] > 0)
			{
				break;
			}

			if(bt->output[ri] > 0)
			{
				break;
			}
		}

		ct = g_cdtype+ctype;

		if(ct->cornerpl)
			ConnectCd(p, ctype, tplace);

		/*TODO math between vec2i and vec2uc
		TODO VEC2SUB VEC3SUB*/

		tmin.x = imax(tplace.x, bt->width.x/2+1) - imin(tplace.x, bt->width.x/2+1);
		tmin.y = imax(tplace.y, bt->width.y/2+1) - imin(tplace.y, bt->width.y/2+1);
		tmin.x = tmin.x + bt->width.x+1;
		tmin.y = tmin.y + bt->width.y+1;
		
		do1 = ((tmin.y) % 4 == 0) || (tmin.y == 0);
		do2 = ((tmax.y) % 4 == 0) || (tmax.y == 0);
		do3 = ((tmin.x) % 3 == 0) || (tmin.x == 0);
		do4 = ((tmax.x) % 3 == 0) || (tmax.x == 0);

		if(do1 && do4)
		{
			line[0].x = imax(0, (int)tmin.x-4);
			line[0].y = imax(0, tmin.y);
			line[1].x = imin(g_mapsz.x-1, (int)tmax.x+4);
			line[1].y = imin(g_mapsz.y-1, (int)tmin.y);
			
			if(ct->blconduct)
			{
				if(tmin.x % 3 == 0)
				{
					line[0].x = tmin.x;
					line[1].x = tmax.x;
				}
				else if(tmax.x % 3 == 0)
				{
					line[0].x = tmin.x;
					line[1].x = tmax.x;
				}
				else
				{
					goto plot2;
				}
			}

			PlotCd(pyi, ctype, line[0], line[1]);
		}

plot2:
		
		if(do2 && do3)
		{
			line[0].x = imax(0, (int)tmin.x-4);
			line[0].y = imax(0, tmax.y);
			line[1].x = imin(g_mapsz.x-1, (int)tmax.x+4);
			line[1].y = imin(g_mapsz.y-1, (int)tmax.y);

			if(ct->blconduct)
			{
				if(tmin.x % 3 == 0)
				{
					line[0].x = tmin.x;
					line[1].x = tmax.x;
				}
				else if(tmax.x % 3 == 0)
				{
					line[0].x = tmin.x;
					line[1].x = tmax.x;
				}
				else
				{
					goto plot3;
				}
			}

			PlotCd(pyi, ctype, line[0], line[1]);
		}
		
plot3:

		if(do3 && do2)
		{
			line[0].x = imax(0, tmin.x);
			line[0].y = imax(0, (int)tmin.y-4);
			line[1].x = imin(g_mapsz.x-1, (int)tmin.x);
			line[1].y = imin(g_mapsz.y-1, (int)tmax.y+4);

			if(ct->blconduct)
			{
				if(tmin.y % 4 == 0)
				{
					line[0].y = tmin.y;
					line[1].y = tmax.y;
				}
				else if(tmax.y % 4 == 0)
				{
					line[0].y = tmin.y;
					line[1].y = tmax.y;
				}
				else
				{
					goto plot4;
				}
			}

			PlotCd(pyi, ctype, line[0], line[1]);
		}
		
plot4:

		if(do4 && do1)
		{
			line[0].x = imax(0, tmax.x);
			line[0].y = imax(0, (int)tmin.y-4);
			line[1].x = imin(g_mapsz.x-1, (int)tmax.x);
			line[1].y = imin(g_mapsz.y-1, (int)tmax.y+4);
			
			if(ct->blconduct)
			{
				if(tmin.y % 4 == 0)
				{
					line[0].y = tmin.y;
					line[1].y = tmax.y;
				}
				else if(tmax.y % 4 == 0)
				{
					line[0].y = tmin.y;
					line[1].y = tmax.y;
				}
				else
				{
					continue;
				}
			}

			PlotCd(pyi, ctype, line[0], line[1]);
		}
	}
}

void PlotCd(char pyi, char ctype, Vec2uc from, Vec2uc to)
{
	List places;	/*Vec2uc*/
	PlaceCdPacket* pcp;
	Vec2uc* v;
	Node* pit;	/*Vec2uc*/
	short pin;

	from.x = imin(from.x, g_mapsz.x-1);
	from.y = imin(from.y, g_mapsz.y-1);
	to.x = imin(to.x, g_mapsz.x-1);
	to.y = imin(to.y, g_mapsz.y-1);
	from.x = imax(from.x, 0);
	from.y = imax(from.y, 0);
	to.x = imax(to.x, 0);
	to.y = imax(to.y, 0);

	List_init(&places);

	TilePath(MV_LABOURER, UMODE_NONE, from.x*TILE_SIZE + TILE_SIZE/2, from.y*TILE_SIZE + TILE_SIZE/2,
		-1, -1, TARG_NONE, -1,
		ctype, &places, NULL, NULL, NULL,
		to.x*TILE_SIZE + TILE_SIZE/2, to.y*TILE_SIZE + TILE_SIZE/2, 
		to.x*TILE_SIZE, to.y*TILE_SIZE, to.x*TILE_SIZE + TILE_SIZE - 1, to.y*TILE_SIZE + TILE_SIZE - 1,
		g_mapsz.x * g_mapsz.y / 2, ectrue, ecfalse);

	if(!places.size)
		goto clean;

	pcp = (PlaceCdPacket*)malloc(
		sizeof(PlaceCdPacket) + 
		sizeof(Vec2uc)*places.size );
	pcp->header.type = PACKET_PLACECD;
	pcp->cdtype = ctype;
	pcp->player = pyi;
	pcp->ntiles = places.size;

	pit = places.head;
	for(pin = 0; pit; pit=pit->next, ++pin)
	{
		v = pcp->place+pin;
		v->x = ((Vec2uc*)pit->data)->x;
		v->y = ((Vec2uc*)pit->data)->y;
	}

	LockCmd((PacketHeader*)pcp);

	free(pcp);

clean:
	List_free(&places);
}