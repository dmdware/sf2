










#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/appviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "blgraphs.h"
#include "blview.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../math/fixmath.h"
#include "../../../net/lockstep.h"
#include "../../../sim/manuf.h"
#include "../../../net/client.h"
#include "../../../language.h"
#include "../../../path/pathjob.h"
#include "../../../econ/utility.h"
#include "../../../sim/simdef.h"
#include "../../../sim/labourer.h"
#include "../../layouts/messbox.h"
#include "../../layouts/chattext.h"

//Need a better way to organize buttons, text, etc.

//TODO put these in econ/
struct DemCompo
{
	int32_t source;
	int32_t bestutil;
	int32_t amount;
	int32_t totalcost;
};

struct DemPoint
{
	Vec2i cmpos;
	std::list<DemCompo> demcompo;
};

static uint64_t g_graphstart = 0;
static uint64_t g_graphtime = 0;

void CalcSup(Building* demb, DemPoint* dp, int32_t ri, int32_t calcbi, int32_t price)
{
	int32_t req;
	
	Resource* r = &g_resource[ri];

	if(r->physical)
	{
		req = demb->netreq(ri);
	}
	//don't subtract stocked/established connections suppliers for capacity type resources that are continuously used up
	else
	{
		BlType* dembt = &g_bltype[demb->type];
		req = dembt->input[ri] * demb->prodlevel / RATIO_DENOM;
	}

	if(req <= 0)
		return;

	Player* dempy = &g_player[demb->owner];
	int32_t remcash = dempy->global[RES_DOLLARS];

	int32_t bestutil;
	int32_t bestbi;

	Vec2i dembcmpos = demb->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);

	while(true)
	{
		bestutil = -1;
		bestbi = -1;
		DemCompo dc;
		dc.bestutil = -1;
		dc.source = -1;

		for(int32_t supbi=0; supbi<BUILDINGS; supbi++)
		{
			Building* supb = &g_building[supbi];

			if(!supb->on)
				continue;

			if(!supb->finished)
				continue;

			BlType* supbt = &g_bltype[supb->type];

			if(supbt->output[ri] <= 0)
				continue;

			Player* suppy = &g_player[supb->owner];

			/*
			Ignore it if the building of interest bi doesn't
			have enough stock because we want to show maximum
			possible demand.
			*/
			if(r->physical && supb->stocked[ri] + suppy->global[ri] <= 0 && supbi != calcbi)
				continue;

			int32_t have = supb->stocked[ri] + suppy->global[ri];

			//if we don't have any more of the resource from this supplier, again
			if(r->capacity && supbi != calcbi)
			{
				//TODO maybe base this on past production achieved
				have = supbt->output[ri] * supb->prodlevel / RATIO_DENOM;

				for(std::list<CapSup>::iterator csit=supb->capsup.begin(); csit!=supb->capsup.end(); csit++)
				{
					if(csit->src == supbi)
						have -= csit->amt;
				}

				if(have <= 0)
					continue;
			}

			bool added = false;

			for(std::list<DemCompo>::iterator dcit=dp->demcompo.begin(); dcit!=dp->demcompo.end(); dcit++)
			{
				if(dcit->source == supbi)
				{
					added = true;
					break;
				}
			}

			if(added)
				continue;
			
			int32_t bprice = supb->price[ri];

			if(supbi == calcbi)
				bprice = price;

			//if the supplier and demander bl have same owner
			if(supb->owner == demb->owner)
			{
				//if the supplier is the bl we're calculating the demand for
				if(supbi == calcbi)
					continue;	//then don't count demand from internal, non-income, sources

				/*
				otherwise we want to record the demand detracted from the calc'd 
				bl in question we're getting the demand curve for, but price=0
				*/
				bprice = 0;
			}

			Vec2i supbcmpos = supb->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
			const Vec2i cmoff = dembcmpos - supbcmpos;
			int32_t cmdist2 = PATHHEUR(cmoff);
			int32_t util = r->physical ? PhUtil(bprice, cmdist2) : GlUtil(bprice);

			if(bestutil >= 0 && util < bestutil)
				continue;

			bestutil = util;
			bestbi = supbi;
			dc.source = supbi;
			dc.bestutil = util;
			dc.amount = have;
			
			//if it's the bl we're calc'ing demand for, don't limit qty
			if(supbi == calcbi)
				dc.amount = INT_MAX;
		}

		if(bestutil < 0)
			break;

		if(req <= 0)
			break;

		if(dc.source < 0)
			break;
		
		Building* supb = &g_building[dc.source];

		int32_t bprice = supb->price[ri];

		if(dc.source == calcbi)
			bprice = price;
		
		if(supb->owner == demb->owner)
			bprice = 0;

		if(remcash < bprice && 
			demb->owner != supb->owner)
			break;

		//limit by supply available
		dc.amount = imin(dc.amount, req);

		if(dc.amount <= 0)
			break;

		dc.totalcost = dc.amount * bprice;
		dc.totalcost = imin(dc.totalcost, remcash);

		if(bprice > 0)
			dc.amount = imin(dc.amount, dc.totalcost / bprice);

		if(dc.amount <= 0)
			break;

		remcash -= dc.totalcost;
		req -= dc.amount;

		dp->demcompo.push_back(dc);
	}
}

void CalcShop(Unit* u, DemPoint* dp, int32_t ri, int32_t bi, int32_t price, 
		std::list<DemPoint>* demptlist)
{
	if(u->type == UNIT_TRUCK)
	{
		int32_t satiety = u->belongings[RES_RETFUEL] - STARTING_FUEL;

		if(satiety >= 0)
			return;

		Player* tpy = &g_player[u->owner];

		int32_t remcash = tpy->global[RES_DOLLARS];
		int32_t bestutil;
		int32_t bestbi;

		while(true)
		{
			bestutil = -1;
			bestbi = -1;
			DemCompo dc;
			dc.bestutil = -1;
			dc.source = -1;

			for(int32_t bi2=0; bi2<BUILDINGS; bi2++)
			{
				Building* b2 = &g_building[bi2];

				if(!b2->on)
					continue;

				if(!b2->finished)
					continue;

				BlType* bt2 = &g_bltype[b2->type];

				if(bt2->output[ri] <= 0)
					continue;

				Player* py2 = &g_player[b2->owner];

				/*
				Ignore it if the building of interest bi doesn't
				have enough stock because we want to show maximum
				possible demand.
				*/
				if(b2->stocked[ri] + py2->global[ri] <= 0 && bi2 != bi)
					continue;

				bool added = false;

				for(std::list<DemCompo>::iterator dcit=dp->demcompo.begin(); dcit!=dp->demcompo.end(); dcit++)
				{
					if(dcit->source == bi2)
					{
						added = true;
						break;
					}
				}

				if(added)
					continue;
			
				int32_t bprice = b2->price[ri];

				if(bi2 == bi)
					bprice = price;

				Vec2i b2cmpos = b2->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				const Vec2i cmoff = u->cmpos - b2cmpos;
				int32_t cmdist2 = PATHHEUR(cmoff);
				int32_t util = PhUtil(bprice, cmdist2);

				if(bestutil >= 0 && util < bestutil)
					continue;

				bestutil = util;
				bestbi = bi2;
				dc.source = bi2;
				dc.bestutil = util;
				dc.amount = b2->stocked[ri] + py2->global[ri];

				//if it's the bl we're calc'ing demand for, don't limit qty
				if(bi2 == bi)
					dc.amount = INT_MAX;
			}

			if(bestutil < 0)
				break;

			if(satiety > 0)
				break;

			if(dc.source < 0)
				break;

			if(dc.amount <= 0)
				break;
		
			Building* b2 = &g_building[dc.source];

			if(remcash < b2->price[ri])
				break;

			//int32_t shopq = ConsumProp(satiety, u->incomerate, b2->price[ri]);
			int32_t shopq = STARTING_FUEL - u->belongings[RES_RETFUEL];

			//limit based on supply available
			shopq = imin(shopq, dc.amount);

			if(shopq <= 0)
				break;

			int32_t bprice = b2->price[ri];

			if(dc.source == bi)
				bprice = price;

			if(b2->owner == u->owner)
				bprice = 0;

			dc.amount = shopq;
			dc.totalcost = shopq * bprice;
			dc.totalcost = imin(dc.totalcost, remcash);

			if(bprice > 0)
				dc.amount = imin(dc.amount, dc.totalcost / bprice);

			if(dc.amount <= 0)
				break;

			remcash -= dc.totalcost;
			satiety += shopq;

			dp->demcompo.push_back(dc);

			//g_applog<<"u "<<(u-g_unit)<<" demcompo "<<dc.source<<"bi amt"<<dc.amount<<" totcst"<<dc.totalcost<<" remcash"<<remcash<<std::endl;
		}// end while(true)
	}
	else if(u->type == UNIT_LABOURER && ri == RES_RETFOOD)
	{
		int32_t satiety = u->belongings[RES_RETFOOD] - STARTING_RETFOOD;
		//int32_t totalshop = ConsumProp(u, b->price[RES_RETFOOD]) + CYCLE_FRAMES * LABOURER_FOODCONSUM;

		if(satiety < 0)
			satiety -= CYCLE_FRAMES * LABOURER_FOODCONSUM;

		if(satiety >= 0)
			return;

		int32_t remcash = u->belongings[RES_DOLLARS];
		int32_t bestutil;
		int32_t bestbi;

		while(true)
		{
			bestutil = -1;
			bestbi = -1;
			DemCompo dc;
			dc.bestutil = -1;
			dc.source = -1;

			for(int32_t bi2=0; bi2<BUILDINGS; bi2++)
			{
				Building* b2 = &g_building[bi2];

				if(!b2->on)
					continue;

				if(!b2->finished)
					continue;

				BlType* bt2 = &g_bltype[b2->type];

				if(bt2->output[ri] <= 0)
					continue;

				Player* py2 = &g_player[b2->owner];

				/*
				Ignore it if the building of interest bi doesn't
				have enough stock because we want to show maximum
				possible demand.
				*/
				if(b2->stocked[ri] + py2->global[ri] <= 0 && bi2 != bi)
					continue;

				bool added = false;

				for(std::list<DemCompo>::iterator dcit=dp->demcompo.begin(); dcit!=dp->demcompo.end(); dcit++)
				{
					if(dcit->source == bi2)
					{
						added = true;
						break;
					}
				}

				if(added)
					continue;
			
				int32_t bprice = b2->price[ri];

				if(bi2 == bi)
					bprice = price;

				Vec2i b2cmpos = b2->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				const Vec2i cmoff = u->cmpos - b2cmpos;
				int32_t cmdist2 = PATHHEUR(cmoff);
				int32_t util = PhUtil(bprice, cmdist2);

				if(bestutil >= 0 && util < bestutil)
					continue;

				bestutil = util;
				bestbi = bi2;
				dc.source = bi2;
				dc.bestutil = util;
				dc.amount = b2->stocked[ri] + py2->global[ri];

				//if it's the bl we're calc'ing demand for, don't limit qty
				if(bi2 == bi)
					dc.amount = INT_MAX;
			}

			if(bestutil < 0)
				break;

			if(satiety > 0)
				break;

			if(dc.source < 0)
				break;

			if(dc.amount <= 0)
				break;
		
			Building* b2 = &g_building[dc.source];

			if(remcash < b2->price[ri])
				break;

			int32_t shopq = ConsumProp(satiety, u->incomerate, b2->price[ri]);

			//limit based on supply available
			shopq = imin(shopq, dc.amount);

			if(shopq <= 0)
				break;

			int32_t bprice = b2->price[ri];

			if(dc.source == bi)
				bprice = price;

			dc.amount = shopq;
			dc.totalcost = shopq * bprice;
			dc.totalcost = imin(dc.totalcost, remcash);

			if(bprice > 0)
				dc.amount = imin(dc.amount, dc.totalcost / bprice);

			if(dc.amount <= 0)
				break;

			remcash -= dc.totalcost;
			satiety += shopq;

			dp->demcompo.push_back(dc);

			//g_applog<<"u "<<(u-g_unit)<<" demcompo "<<dc.source<<"bi amt"<<dc.amount<<" totcst"<<dc.totalcost<<" remcash"<<remcash<<std::endl;
		}// end while(true)
	}
	else if(u->type == UNIT_LABOURER && ri == RES_HOUSING)
	{
		int32_t remcash = u->belongings[RES_DOLLARS];
		int32_t bestutil;
		int32_t bestbi;

		while(true)
		{
			bestutil = -1;
			bestbi = -1;
			DemCompo dc;
			dc.bestutil = -1;
			dc.source = -1;

			for(int32_t bi2=0; bi2<BUILDINGS; bi2++)
			{
				Building* b2 = &g_building[bi2];

				if(!b2->on)
					continue;

				if(!b2->finished)
					continue;

				BlType* bt2 = &g_bltype[b2->type];

				if(bt2->output[ri] <= 0)
					continue;

				Player* py2 = &g_player[b2->owner];

				//if(b->stocked[RES_HOUSING] <= 0)
				//	continue;

				//if(bt->output[RES_HOUSING] - b->inuse[RES_HOUSING] <= 0)
				//	continue;

#if 1
				int32_t nc = 0;

				for(std::list<DemPoint>::iterator dlit=demptlist->begin(); dlit!=demptlist->end(); ++dlit)
				{
					for(std::list<DemCompo>::iterator dpit=dlit->demcompo.begin(); dpit!=dlit->demcompo.end(); dpit++)
					{
						if(dpit->source == bi2)
							++nc;
					}
				}

				if(nc >= bt2->output[RES_HOUSING])
					continue;
#endif

				//if(b2->occupier.size() >= bt2->output[RES_HOUSING])
				//	continue;

				if(b2->price[RES_HOUSING] > u->belongings[RES_DOLLARS])
					continue;

#if 0
				bool added = false;

				for(std::list<Widget*>::iterator dcit=dp->demcompo.begin(); dcit!=dp->demcompo.end(); dcit++)
				{
					if(dcit->source == bi2)
					{
						added = true;
						break;
					}
				}

				if(added)
					continue;
#endif

				int32_t bprice = b2->price[ri];

				//if(bi2 == bi)
				//	bprice = price;
				
				if(bi2 == bi)
					bprice = price;

				Vec2i b2cmpos = b2->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
				const Vec2i cmoff = u->cmpos - b2cmpos;
				int32_t cmdist2 = PATHHEUR(cmoff);
				int32_t util = PhUtil(bprice, cmdist2);

				if(bestutil >= 0 && util < bestutil)
					continue;

				bestutil = util;
				bestbi = bi2;
				dc.source = bi2;
				dc.bestutil = util;
				dc.amount = 1;

				//if it's the bl we're calc'ing demand for, don't limit qty
				//if(bi2 == bi)
				//	dc.amount = INT_MAX;
			}

			if(bestutil < 0)
				break;

			if(dc.source < 0)
				break;

			if(dc.amount <= 0)
				break;
		
			Building* b2 = &g_building[dc.source];

			if(remcash < b2->price[ri])
				break;

			int32_t shopq = dc.amount;

			//limit based on supply available
			shopq = imin(shopq, dc.amount);

			if(shopq <= 0)
				break;

			int32_t bprice = b2->price[ri];

			if(dc.source == bi)
				bprice = price;

#if 0
			dc.amount = shopq;
			dc.totalcost = shopq * bprice;
			dc.totalcost = imin(dc.totalcost, remcash);

			if(bprice > 0)
				dc.amount = imin(dc.amount, dc.totalcost / bprice);

			if(dc.amount <= 0)
				break;
#else
			dc.totalcost = bprice;
#endif

			remcash -= dc.totalcost;
			//satiety += shopq;

			dp->demcompo.push_back(dc);

			return;

			//g_applog<<"u "<<(u-g_unit)<<" demcompo "<<dc.source<<"bi amt"<<dc.amount<<" totcst"<<dc.totalcost<<" remcash"<<remcash<<std::endl;
		}// end while(true)
	}
}

void AddDataPt()
{
}

//TODO move struct
struct DataPoint
{
	union
	{
		int32_t x;
		int32_t price;
	};
	union
	{
		int32_t y;
		int32_t qty;
	};
};

void DrawBlGraph_Dem_DrawData(float* frame, std::list<DataPoint>* dempt, std::list<DataPoint>* suppt, std::list<DataPoint>* avcost, std::list<DataPoint>* mgcost,
							  std::list<DataPoint>* wagept, BlGraphs *blg, int32_t ri, float maxqty)
{
	Shader* s = &g_shader[g_curS];

	//glLineWidth(2.0f);
	glLineWidth(1.0f);

	//glUniform4f(s->slot[SSLOT_COLOR], 0, 1, 0, 1);

	float subfr[4];
	subfr[0] = frame[0] + 45;
	subfr[1] = frame[1] + 3;
	subfr[2] = frame[2] - 3;
	subfr[3] = frame[3] - 25;

	float maxpr = 0;
	float minpr = 0;
#if 1
	//float maxqty = 0;
	//maxqty = 0;

#if 1
	for(std::list<DataPoint>::iterator dpit=dempt->begin(); dpit!=dempt->end(); dpit++)
	{
		if(dpit->price > maxpr)
			maxpr = dpit->price;
		//if(dpit->qty > maxqty)
		//	maxqty = dpit->qty;
		if(dpit->price < minpr)
			minpr = dpit->price;
	}
#endif

#if 1
	for(std::list<DataPoint>::iterator dpit=suppt->begin(); dpit!=suppt->end(); dpit++)
	{
		if(dpit->price > maxpr)
			maxpr = dpit->price;
		//if(dpit->qty > maxqty)
		//	maxqty = dpit->qty;
		if(dpit->price < minpr)
			minpr = dpit->price;
	}
#endif
	
#if 1
	for(std::list<DataPoint>::iterator dpit=avcost->begin(); dpit!=avcost->end(); dpit++)
	{
		if(dpit->price > maxpr)
			maxpr = dpit->price;
		//if(dpit->qty > maxqty)
		//	maxqty = dpit->qty;
		if(dpit->price < minpr)
			minpr = dpit->price;
	}
#endif
	
#if 1
	for(std::list<DataPoint>::iterator dpit=mgcost->begin(); dpit!=mgcost->end(); dpit++)
	{
		if(dpit->price > maxpr)
			maxpr = dpit->price;
		//if(dpit->qty > maxqty)
		//	maxqty = dpit->qty;
		if(dpit->price < minpr)
			minpr = dpit->price;
	}
#endif

	uint64_t t = GetTicks();

	//show the whole large graph / just within production capability alternately every 3 secs
	if(t / (1000*3/2) % 4 == 0)
	{		
		//if true, unbounded whole graph
		//maxpr = 0;
		maxqty = 0;
	#if 1
		for(std::list<DataPoint>::iterator dpit=dempt->begin(); dpit!=dempt->end(); dpit++)
		{
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
		}
	#endif

	#if 1
		for(std::list<DataPoint>::iterator dpit=suppt->begin(); dpit!=suppt->end(); dpit++)
		{
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
		}
	#endif
	
	#if 1
		for(std::list<DataPoint>::iterator dpit=avcost->begin(); dpit!=avcost->end(); dpit++)
		{
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
		}
	#endif
		
	#if 1
		for(std::list<DataPoint>::iterator dpit=mgcost->begin(); dpit!=mgcost->end(); dpit++)
		{
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
		}
	#endif
	}
	else if(t / (1000*3/2) % 4 == 1)
	{
		//bound only by supply curve
		maxpr = 0;
		minpr = 0;
		maxqty = 0;
		for(std::list<DataPoint>::iterator dpit=dempt->begin(); dpit!=dempt->end(); dpit++)
		{
			if(dpit->price > maxpr)
				maxpr = dpit->price;
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
			//if(dpit->price < minpr)
			//	minpr = dpit->price;
		}
	}

	else if(t / (1000*3/2) % 4 == 2)
	{
		//bound price by demand curve range
		maxpr = 0;
		minpr = 0;
		maxqty = 0;
		for(std::list<DataPoint>::iterator dpit=suppt->begin(); dpit!=suppt->end(); dpit++)
		{
			if(dpit->price > maxpr)
				maxpr = dpit->price;
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
			//if(dpit->price < minpr)
			//	minpr = dpit->price;
		}
	}
	else
	{
		maxpr = 0;
		minpr = 0;
		maxqty = 0;
		for(std::list<DataPoint>::iterator dpit=wagept->begin(); dpit!=wagept->end(); dpit++)
		{
			if(dpit->price > maxpr)
				maxpr = dpit->price;
			if(dpit->qty > maxqty)
				maxqty = dpit->qty;
			//if(dpit->price < minpr)
			//	minpr = dpit->price;
		}
	}
#endif

	//maxqty = 10000000;

	int32_t i = 0;
	int32_t sz = dempt->size();
	std::list<DataPoint>::iterator dpit=dempt->begin();
	std::list<DataPoint>::iterator dpit2=dempt->begin();
	dpit2++;

#if 1
	//dem
	for(; dpit2!=dempt->end(); dpit2++)
	{
#if 0
		DrawLine(0, 1, 0, 0.5f, 
			subfr[0] + fmin(maxpr,dpit->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit->qty) / maxqty * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxpr,dpit2->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit2->qty) / maxqty * (subfr[3]-subfr[1]), 
			blg->m_crop);
#else
		DrawLine(0, 1, 0, 0.5f, 
			subfr[0] + fmin(maxqty,dpit->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxqty,dpit2->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit2->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			blg->m_crop);
#endif

		dpit = dpit2;
		i++;
	}
#endif
	
	//glUniform4f(s->slot[SSLOT_COLOR], 1, 0, 0, 1);

	//sup
	i = 0;
	sz = suppt->size();
	dpit=suppt->begin();
	dpit2=suppt->begin();
	dpit2++;

	for(; dpit2!=suppt->end(); dpit2++)
	{
#if 0
		DrawLine(0, 0, 1, 0.5f, 
			subfr[0] + fmin(maxpr,dpit->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit->qty) / maxqty * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxpr,dpit2->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit2->qty) / maxqty * (subfr[3]-subfr[1]), 
			blg->m_crop);
#else
		DrawLine(0, 0, 1, 0.5f, 
			subfr[0] + fmin(maxqty,dpit->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxqty,dpit2->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit2->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			blg->m_crop);
#endif

		dpit = dpit2;
		i++;
	}

	//avc
	i = 0;
	sz = avcost->size();
	dpit=avcost->begin();
	dpit2=avcost->begin();
	dpit2++;

	for(; dpit2!=avcost->end(); dpit2++)
	{
#if 0
		DrawLine(1, 0, 0, 0.5f, 
			subfr[0] + fmin(maxpr,dpit->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit->qty) / maxqty * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxpr,dpit2->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit2->qty) / maxqty * (subfr[3]-subfr[1]), 
			blg->m_crop);
#else
		DrawLine(1, 0, 0, 0.5f, 
			subfr[0] + fmin(maxqty,dpit->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxqty,dpit2->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit2->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			blg->m_crop);
#endif

		dpit = dpit2;
		i++;
	}

	//mgc
	i = 0;
	sz = mgcost->size();
	dpit=mgcost->begin();
	dpit2=mgcost->begin();
	dpit2++;

	for(; dpit2!=mgcost->end(); dpit2++)
	{
#if 0
		DrawLine(1, 0, 0, 0.5f, 
			subfr[0] + fmin(maxpr,dpit->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit->qty) / maxqty * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxpr,dpit2->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit2->qty) / maxqty * (subfr[3]-subfr[1]), 
			blg->m_crop);
#else
		DrawLine(1, 1, 0, 0.5f, 
			subfr[0] + fmin(maxqty,dpit->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxqty,dpit2->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit2->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			blg->m_crop);
#endif

		dpit = dpit2;
		i++;
	}

	//wg
	i = 0;
	sz = wagept->size();
	dpit=wagept->begin();
	dpit2=wagept->begin();
	dpit2++;

	for(; dpit2!=wagept->end(); dpit2++)
	{
#if 0
		DrawLine(1, 0, 0, 0.5f, 
			subfr[0] + fmin(maxpr,dpit->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit->qty) / maxqty * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxpr,dpit2->price) / maxpr * (subfr[2]-subfr[0]), subfr[3] - fmin(maxqty,dpit2->qty) / maxqty * (subfr[3]-subfr[1]), 
			blg->m_crop);
#else
		DrawLine(0, 1, 1, 0.5f, 
			subfr[0] + fmin(maxqty,dpit->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			subfr[0] + fmin(maxqty,dpit2->qty) / maxqty * (subfr[2]-subfr[0]), subfr[3] - (fmin(maxpr,dpit2->price)-minpr) / maxpr * (subfr[3]-subfr[1]), 
			blg->m_crop);
#endif

		dpit = dpit2;
		i++;
	}

	glLineWidth(1.0f);
	
	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);

	BmpFont* f = &g_font[MAINFONT16];
	
	float dy = f->gheight;
	//float dyqty = (maxqty) / ((subfr[3]-subfr[1])/dy);
	float dypr = (maxpr-minpr) / ((subfr[3]-subfr[1])/dy);

	float dx = (f->gheight*5);
	//float dxpr = maxpr / ((subfr[2]-subfr[0])/dx);
	float dxqty = maxqty / ((subfr[2]-subfr[0])/dx);

	float color[4] = {1,1,1,1};

#if 0
	char m[123];
	sprintf(m, "sup %d", (int32_t)suppt->size());
	RichText rm = RichText(m);
	DrawShadowedTextF(MAINFONT16, frame[0] + 100, frame[1] + 100, frame[0], frame[1], frame[2], frame[3], &rm, NULL, -1);
#endif

	float scolor[4] = {0,0,1,1};
	RichText srm = STRTABLE[STR_SUP];
	DrawShadowedTextF(MAINFONT16, frame[0] + 100, frame[1] + 60, frame[0], frame[1], frame[2], frame[3], &srm, scolor, -1);

	float dcolor[4] = {0,1,0,1};
	RichText drm = STRTABLE[STR_DEM];
	DrawShadowedTextF(MAINFONT16, frame[0] + 100, frame[1] + 90, frame[0], frame[1], frame[2], frame[3], &drm, dcolor, -1);

	float acolor[4] = {1,0,0,1};
	RichText arm = STRTABLE[STR_AVC];
	DrawShadowedTextF(MAINFONT16, frame[0] + 100, frame[1] + 120, frame[0], frame[1], frame[2], frame[3], &arm, acolor, -1);
	
	float mcolor[4] = {1,1,0,1};
	RichText mrm = STRTABLE[STR_MGC];
	DrawShadowedTextF(MAINFONT16, frame[0] + 100, frame[1] + 30, frame[0], frame[1], frame[2], frame[3], &mrm, mcolor, -1);

	float wcolor[4] = {0,1,1,1};
	RichText wrm = STRTABLE[STR_WAGE];
	DrawShadowedTextF(MAINFONT16, frame[0] + 300, frame[1] + 120, frame[0], frame[1], frame[2], frame[3], &wrm, wcolor, -1);

	//Resource* r = &g_resource[RES_DOLLARS];
	//Icon* ric = &g_icon[r->icon];
	Resource* r = &g_resource[ri];

	for(float x=subfr[0], xval=0; x<subfr[2]; x+=dx, xval+=dxqty)
	{
		char ct[32];
		sprintf(ct, "%s", iform((int32_t)xval).c_str());
		//RichText rt = RichText(RichPart(RICH_ICON, r->icon)) + RichText(ct);
		RichText rt = RichText(RichPart(RICH_ICON, r->icon)) + RichText(ct);
		DrawShadowedTextF(MAINFONT8, x, subfr[3], frame[0], frame[1], frame[2], frame[3], &rt, color, -1);
	}

	//r = &g_resource[ri];
	r = &g_resource[RES_DOLLARS];

	for(float y=subfr[3], yval=minpr; y>=subfr[1]; y-=dy, yval+=dypr)
	{
		char ct[32];
		sprintf(ct, "%s", iform((int32_t)yval).c_str());
		//RichText rt = RichText(RichPart(RICH_ICON, r->icon)) + RichText(ct);
		RichText rt = RichText(RichPart(RICH_ICON, r->icon)) + RichText(ct);
		DrawShadowedTextF(MAINFONT8, frame[0], y, frame[0], frame[1], frame[2], frame[3], &rt, color, -1);
	}
}

void CalcTransp(std::list<int32_t>* usedtr, Building* b, Building* supb, int32_t* cost)
{
	*cost = -1;
	
	Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
	Vec2i supbcmpos = supb->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	int32_t bestcost = -1;
	int32_t bestui = -1;

	for(int32_t ui=0; ui<UNITS; ++ui)
	{
		Unit* u = &g_unit[ui];

		if(!u->on)
			continue;

		if(u->type != UNIT_TRUCK)
			continue;
		
		int32_t cost = 0;
		int32_t totdist = 0;
		Player* py;
		Vec2i supoff;
		Vec2i demoff;

		int32_t usedc = 0;

		for(std::list<int32_t>::iterator trit=usedtr->begin(); trit!=usedtr->end(); trit++)
		{
			if(*trit == ui)
				++usedc;
				//goto nexttr;
		}

		if(usedc > 10)
			goto nexttr;

		supoff = u->cmpos - supbcmpos;
		demoff = supbcmpos - bcmpos;
		totdist += PATHHEUR( supoff );
		totdist += PATHHEUR( demoff );

		py = &g_player[u->owner];

		cost = totdist * py->transpcost / TILE_SIZE;

		if(bestcost >= 0 && 
			cost >= bestcost)
			continue;

		bestcost = cost;
		bestui = ui;
nexttr:
		;
	}

	if(bestcost < 0)
		return;

	*cost = bestcost;
	usedtr->push_back(bestui);
}

void CalcSup(Building* b, std::list<DataPoint>* suppt, std::list<DataPoint>* avgcpt, std::list<DataPoint>* mgcpt, std::list<DataPoint>* wagept, int32_t ri, std::list<DataPoint>* dempt)
{
	//float maxpr = 10;
	int32_t maxqty = 10;
	
	BlType* bt = &g_bltype[b->type];
	maxqty = bt->output[ri];

	//float maxpr = dempt->rbegin()->price;
	//float maxqty = dempt->begin()->qty;

	for(std::list<DataPoint>::iterator dpit=dempt->begin(); dpit!=dempt->end(); dpit++)
	{
		//if(dpit->price > maxpr)
		//	maxpr = dpit->price;
		//if(dpit->qty > maxqty)
		//	maxqty = dpit->qty;
	}

	int32_t price = 0;
	int32_t qty = 0;
	int32_t lastqty = qty;
	int32_t lastcost = 0;

	int32_t bi = b - g_building;
	Player* py = &g_player[b->owner];
	Vec2i bcmpos = b->tpos*TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;

	//if(ri == RES_HOUSING)
	//	return;

	if(bt->output[ri] <= 0)
		return;

	int32_t trcnt = CountU(UNIT_TRUCK);
	
	std::list<DemCompo> labpt;

	while( qty <= bt->output[ri] && (qty <= maxqty || qty <= bt->output[ri] * 10 || qty < 1000000 /* || price < maxpr */ ) && price >= 0 )
	{
		int32_t prodlevel = qty * RATIO_DENOM / bt->output[ri];
		DataPoint sp;

		int32_t totcost = 0;

		int32_t labreq = ceili(prodlevel * bt->input[RES_LABOUR], RATIO_DENOM);
		int32_t trywage = 1;
		int32_t labgot = 0;
		std::list<int32_t> usedtr;
		int32_t labcost;

		//1. get labour
		//2. get inputs and transport

		//while(labgot < labreq)
		while(true)
		{
			//if true, everbody gets same wage
			labgot = 0;

			//labgot += imin(labreq, labpt.size() * WORK_RATE);

			//if(labgot >= labreq)
			{
				//stop trying units
			//	break;
			}

			int32_t bestutil;
			labcost = 0;

			DemCompo addlabpt;
			addlabpt.bestutil = -1;	//building's score for employee?

			for(int32_t ui=0; ui<UNITS; ++ui)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->type != UNIT_LABOURER)
					continue;

				//if(u->belongings[RES_LABOUR] <= 0)
				//	continue;

#if 0
				if(u->mode == UMODE_BLJOB ||
					u->mode == UMODE_CSTJOB ||
					u->mode == UMODE_CDJOB ||
					u->mode == UMODE_DRIVE)
					continue;
#endif

				int32_t util;
				int32_t cmdist;
				int32_t bestutil;
				int32_t bestcmdist;
				int32_t thisutil;
				int32_t thiscmdist;
				int32_t addlab;
				int32_t b2reqlab;
				Vec2i b2cmpos;
				BlType* b2t;
				Player* b2py;
				CdType* ct;

#if 0	//don't need, all workers work for same wage, and have no score
				//TODO replace all double loop "founds" breaks with goto's like this
				for(std::list<Widget*>::iterator lit=labpt.begin(); lit!=labpt.end(); lit++)
				{
					if(lit->source == ui)
						goto notlab;
				}
#endif

				Vec2i cmoff = bcmpos - u->cmpos;
				thiscmdist = JOBHEUR(cmoff);
				thisutil = JobUtil(trywage, thiscmdist, WORK_DELAY);

				bestutil = thisutil;
				bestcmdist = thiscmdist;

				//determine if this job is best for worker

#if 1
				for(int32_t ui2=0; ui2<UNITS; ++ui2)
				{
					Unit* u2 = &g_unit[ui2];

					if(!u2->on)
						continue;

					if(u2->type != UNIT_TRUCK)
						continue;

					if(u2->driver >= 0)
						continue;

					if(u2->mode == UMODE_NONE)
						continue;

					b2py = &g_player[u2->owner];

					Vec2i cmoff = u2->cmpos - u->cmpos;
					cmdist = JOBHEUR(cmoff);
					util = JobUtil(b2py->truckwage, cmdist, DRIVE_WORK_DELAY);

					if(util > bestutil)
					{
						goto notlab;
						//trywage = InvJobUtilP(util + 1, thiscmdist, WORK_DELAY) + 1;
						//goto nextwage;
					}
				}

				for(int32_t bi2=0; bi2<BUILDINGS; ++bi2)
				{
					Building* b2 = &g_building[bi2];

					if(bi2 == bi)
						continue;

					if(!b2->on)
						continue;

					b2cmpos = b2->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
					b2t = &g_bltype[b2->type];

					int32_t b2wage;

					if(b2->finished)
					{
						b2reqlab = b2->netreq(RES_LABOUR);
						b2wage = b2->opwage;
					}
					else
					{
						b2reqlab = b2t->conmat[RES_LABOUR] - b2->conmat[RES_LABOUR];
						b2wage = b2->conwage;
					}

					//if(b2reqlab <= 0)
					//	continue;

					Vec2i cmoff = b2cmpos - u->cmpos;
					cmdist = JOBHEUR(cmoff);
					util = JobUtil(b2wage, cmdist, WORK_DELAY);

					if(util > bestutil)
					{
						goto notlab;
						//trywage = InvJobUtilP(util + 1, thiscmdist, WORK_DELAY) + 1;
						//goto nextwage;
					}
				}

				for(int32_t cdtype=0; cdtype<CD_TYPES; ++cdtype)
				{
					ct = &g_cdtype[cdtype];

					for(int32_t cdtx=0; cdtx<g_mapsz.x; ++cdtx)
					{
						for(int32_t cdty=0; cdty<g_mapsz.y; ++cdty)
						{
							CdTile* ctile = GetCd(cdtype, cdtx, cdty, false);

							if(!ctile->on)
								continue;

							if(ctile->finished)
								continue;

							b2reqlab = ct->conmat[RES_LABOUR] - ctile->conmat[RES_LABOUR];

							if(b2reqlab <= 0)
								continue;

							b2cmpos = Vec2i(cdtx,cdty)*TILE_SIZE + ct->physoff;
							Vec2i cmoff = b2cmpos - u->cmpos;
							cmdist = JOBHEUR(cmoff);
							b2py = &g_player[ctile->owner];
							util = JobUtil(ctile->conwage, cmdist, WORK_DELAY);

							if(util > bestutil)
							{
								goto notlab;
								//trywage = InvJobUtilP(util + 1, thiscmdist, WORK_DELAY) + 1;
								//goto nextwage;
							}
						}
					}
				}
#endif

#if 0	//don't need
				//determine if this worker is best for job

				if(bestutil < addlabpt.bestutil &&
					addlabpt.bestutil >= 0)
					continue;
#endif

addlab:
				addlab = imax(labreq - labgot, 0);
				addlab = imin(addlab, u->belongings[RES_LABOUR]);
				addlab = imin(addlab, WORK_RATE * CYCLE_FRAMES / WORK_DELAY);
				//labgot += addlab;
				//totcost += addlab * trywage;
				//labcost += trywage * imax(1, addlab / WORK_RATE);
				//labcost += trywage * addlab / WORK_RATE;

				//addlab = imin(labreq - labgot, WORK_RATE);//freeze farm
				addlab = WORK_RATE;

				addlabpt.source = ui;
				addlabpt.bestutil = bestutil;
				//addlabpt.amount = u->belongings[RES_LABOUR];
				addlabpt.amount = addlab;
				//addlabpt.totalcost = trywage * addlabpt.amount;
				addlabpt.totalcost = trywage * addlab / WORK_RATE;

#if 1
				//labpt.push_back(addlabpt);
				labgot += addlabpt.amount;
				//labcost += trywage * addlabpt.amount / WORK_RATE;
				labcost += addlabpt.totalcost;
				
				if(labgot >= labreq)
				{
					//stop trying units
					break;
				}
#endif

notlab:
				;
			}

#if 0
		//	if(addlabpt.bestutil < 0)
			//	goto trywage;
	
			labpt.push_back(addlabpt);
			labgot += addlabpt.amount;
			//labcost += trywage * addlabpt.amount / WORK_RATE;
			labcost += addlabpt.totalcost;
#endif

			if(labgot >= labreq)
			{
				//stop trying wage levels
				break;
			}
			
trywage:

			uint64_t pass = GetTicks() - g_graphstart;

			//TODO could be more accurate by narrowing down on the exact wage to achieve req lab
			//trywage = trywage * 33/32 + 1;
			trywage = trywage + (INV_WAGE_EFFECT * PATHNODE_SIZE / INV_DIST_EFFECT) * iceil(pass, 33);

			//if(trywage >= py->global[RES_DOLLARS] ||
			//	trywage >= INST_FUNDS)
			//	break;
			
nextwage:
			
			if(trywage >= INT_MAX/20 ||
				trywage < 0)
			{
						//rem
						//AddNotif(&RichText("b><2122111"));
				//g_applog<<"labgot<req"<<std::endl;
				goto notqty;
			}
				//break;
		}

		if(labgot < labreq)
		{
					//rem
					//AddNotif(&RichText("b><21111"));
			//g_applog<<"labgot<req"<<std::endl;
			goto notqty;
		}

		//TODO add to avcost graph
		//totcost += imax(1, labgot / WORK_RATE) * trywage;
		totcost += trywage * labgot / WORK_RATE;

		if(totcost >= INT_MAX/20)
		{
					//rem
					//AddNotif(&RichText("b><211"));
			goto notqty;
		}

		//2. get inputs and transport...

		//std::list<int32_t> usedtr;
		usedtr.clear();

		for(int32_t ri2=0; ri2<RESOURCES; ++ri2)
		{
			if(ri2 == ri)
				continue;

			if(ri2 == RES_LABOUR)
				continue;

			if(bt->input[ri2] <= 0)
				continue;

			int32_t rreq = bt->input[ri2] * prodlevel / RATIO_DENOM;

			Resource* r = &g_resource[ri2];

			std::list<int32_t> rsources;

			while(rreq > 0)
			{
				int32_t bestutil = -1;
				int32_t bestbi = -1;

				for(int32_t bi2=0; bi2<BUILDINGS; ++bi2)
				{
					Building* b2 = &g_building[bi2];

					if(!b2->on)
						continue;

					if(!b2->finished)
						continue;

					BlType* b2t = &g_bltype[b2->type];

					if(b2t->output[ri2] <= 0)
						continue;

					Vec2i b2cmpos;
					int32_t dist;
					int32_t util;
					Vec2i cmoff;

					for(std::list<int32_t>::iterator b2it=rsources.begin(); b2it!=rsources.end(); b2it++)
					{
						if(*b2it == bi2)
							goto nextbi2;
					}

					b2cmpos = b2->tpos * TILE_SIZE + Vec2i(TILE_SIZE,TILE_SIZE)/2;
					cmoff = b2cmpos - bcmpos;
					dist = PATHHEUR(cmoff);
					util = r->capacity ? GlUtil(b2->price[ri2]) : PhUtil(b2->price[ri2], dist);

					if(bestutil >= 0 && util < bestutil)
						continue;

					bestutil = util;
					bestbi = bi2;
nextbi2:
					;
				}

				if(bestutil < 0)
				{
					//rem
					//AddNotif(&RichText("b><"));
					//g_applog<<"bestu<0 rreq"<<rreq<<std::endl;
					goto notqty;
				}

				rsources.push_back(bestbi);

				Building* b2 = &g_building[bestbi];
				BlType* b2t = &g_bltype[b2->type];
				
				//int32_t ramt = imin(rreq, b2t->output[ri2]);
				//int32_t ramt = imin(rreq, b2t->output[ri2] * b2->prodlevel);
				int32_t ramt = 0;
				
				if(!r->capacity)
					//ramt = imin(rreq, b2->stocked[ri2]);
					ramt = imin(rreq, b2t->output[ri2]);
				else
				{
					//from building.cPP:
					
#if 0
					int32_t minlevel = b2->maxprod();
					//minlevel is not enough to determine ability to supply capacity type resources.
					//as soon as the inputs are used up, they into the "cymet" counter, and won't be resupplied
					//until cymet is reset to 0, next cycle. so use whichever is greater.
					int32_t suppliable = b2t->output[ri2] * minlevel / RATIO_DENOM;
					suppliable = imax(suppliable, b2->cymet * b2t->output[ri2] / RATIO_DENOM);
#else
					int32_t suppliable = b2t->output[ri2];
#endif

#if 0
					int32_t supplying = 0;

					//check how much this supplier is already supplying to other demanders
					for(std::list<Widget*>::iterator csit=b2->capsup.begin(); csit!=b2->capsup.end(); csit++)
					{
						if(csit->src != bestbi)
							continue;

						if(csit->rtype != ri2)
							continue;

						if(csit->dst == bi)
							continue;

						supplying += csit->amt;
					}

					suppliable -= supplying;
					
					ramt = imin(rreq, suppliable);
#else
					ramt = imin(rreq, b2t->output[ri2]);
#endif
				}


				totcost += ramt * b2->price[ri2];
				rreq -= ramt;

				if(r->capacity)
					continue;

				if(trcnt <= 0)
					continue;

				int32_t transpcost = -1;
				CalcTransp(&usedtr, b, b2, &transpcost);

				if(transpcost < 0)
				{
					//rem
					//AddNotif(&RichText("b><2"));
					//continue;
					goto notqty;
				}

				totcost += transpcost;
			}

			if(rreq > 0)
			{
					//rem
					//AddNotif(&RichText("b><23"));
				//g_applog<<"rreq>0"<<std::endl;
				goto notqty;
			}
		}
		
		price = 0;

		while(//price <= maxpr ||
			//suppt->size() < 10 ||
			//qty < bt->output[ri]
			price < INT_MAX/20 /* &&
			price * bt->output[ri] < INT_MAX/20 */ )
		{
			int32_t totearn = price * qty;

			if(totearn >= totcost)
			{
				goto foundqty;
			}
			
			//price = price * 3/2 + 1;
			price = price * 3/2 + 1;
			//TODO + qtyx to totearn
			//TODO floats maxpr maxqty to determ
			//price += price * bt->output[ri] / 200 + 1;
		}
		
					//rem
					//AddNotif(&RichText("b><2123"));
notqty:
		//g_applog<<"======= notq"<<qty<<" tc"<<totcost<<std::endl;

		//goto nextqty;

		return;

foundqty:

		//sp.price = price;
		//sp.qty = qty;
		//suppt->push_back(sp);
		
		DataPoint avc;
		avc.price = totcost / imax(1,qty);
		avc.qty = qty;
		avgcpt->push_back(avc);

		int32_t totearn = price * qty;
		//g_applog<<"sup pr"<<price<<" q"<<qty<<" w"<<trywage<<" l"<<labreq<<" tc"<<totcost<<" labcost"<<labcost<<" te"<<totearn<<std::endl;

		DataPoint mgc;
		mgc.price = (totcost - lastcost) / imax(1, qty - lastqty);
		mgc.qty = qty;
		mgcpt->push_back(mgc);

		sp.price = mgc.price + (qty ? ((trywage * 2) / imax(1, qty)) : 0) + mgc.price * 90 / 100;
		sp.qty = qty;
		suppt->push_back(sp);

		lastqty = qty;
		lastcost = totcost;

		DataPoint wgp;
		wgp.price = trywage;
		wgp.qty = qty;
		wagept->push_back(wgp);

		//char m[123];
		//sprintf(m, "p %d,%d", wgp.price, wgp.qty);
		//RichText rm = m;
		//AddNotif(&rm);

nextqty:

		uint64_t pass = GetTicks() - g_graphstart;

		//price = price * 3/2 + 1;
		//qty = 0;
		//price = 0;
		//qty = qty * 11/9 + 1 + bt->output[ri] / 100;
		qty = qty + (maxqty / 100 + 10) * iceil(pass, 33);

		//if(qty > bt->output[ri])
		//	break;

		
		////sprintf(m, "p %d,%d %llu", maxqty, qty, pass);
		// rm = m;
		//AddNotif(&rm);
	}
	
		//g_applog<<"======="<<qty<<std::endl;
}

void CalcDem(Building* b, int32_t bi, std::list<DataPoint>* datapt, int32_t ri)
{
	int32_t price = 0;
	uint64_t qty = 0;

	Resource* r = &g_resource[ri];

	while(true)
	{
		std::list<DemPoint> demptlist;

		if(ri == RES_LABOUR)
		{
			//from labourer suppliers
		}
		else if(ri == RES_HOUSING)
		{
			//to labourer demanders=
			
			for(int32_t ui=0; ui<UNITS; ui++)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->type != UNIT_LABOURER)
					continue;

				//int32_t dist = PATHHEUR(u->cmpos - bcmpos);

				//int32_t bestutil = PhUtil(b->price[ri], dist);
				//int32_t bestbi = bi;

				DemPoint dp;
				dp.cmpos = u->cmpos;
				
				CalcShop(u, &dp, ri, bi, price, &demptlist);
				demptlist.push_back(dp);

				//g_applog<<"ui "<<ui<<"/"<<UNITS<<std::endl;
				//g_applog.flush();
			}
		}
		else if(ri == RES_RETFUEL)
		{
			//to truck demanders

			for(int32_t ui=0; ui<UNITS; ui++)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->type != UNIT_TRUCK)
					continue;

				//int32_t dist = PATHHEUR(u->cmpos - bcmpos);

				//int32_t bestutil = PhUtil(b->price[ri], dist);
				//int32_t bestbi = bi;

				DemPoint dp;
				dp.cmpos = u->cmpos;
				
				CalcShop(u, &dp, ri, bi, price, &demptlist);
				demptlist.push_back(dp);

				//g_applog<<"ui "<<ui<<"/"<<UNITS<<std::endl;
				//g_applog.flush();
			}
		}
		else if(ri == RES_RETFOOD)
		{
			//to labourer demanders

			for(int32_t ui=0; ui<UNITS; ui++)
			{
				Unit* u = &g_unit[ui];

				if(!u->on)
					continue;

				if(u->type != UNIT_LABOURER)
					continue;

				//int32_t dist = PATHHEUR(u->cmpos - bcmpos);

				//int32_t bestutil = PhUtil(b->price[ri], dist);
				//int32_t bestbi = bi;

				DemPoint dp;
				dp.cmpos = u->cmpos;
				
				CalcShop(u, &dp, ri, bi, price, &demptlist);
				demptlist.push_back(dp);

				//g_applog<<"ui "<<ui<<"/"<<UNITS<<std::endl;
				//g_applog.flush();
			}//end for(int32_t ui=0; ui<UNITS; ui++)
		}
		else
		{
			//from and to building(s)
			//selling resource ri to other bl's

			for(int32_t dembi=0; dembi<BUILDINGS; dembi++)
			{
				Building* demb = &g_building[dembi];

				if(!demb->on)
					continue;

				if(!demb->finished)
					continue;

				BlType* dembt = &g_bltype[demb->type];

				if(dembt->input[ri] <= 0)
					continue;
				
				DemPoint dp;
				//dp.cmpos = u->cmpos;
				
				CalcSup(demb, &dp, ri, bi, price);
				demptlist.push_back(dp);
			}
		}

		qty = 0;
		//int32_t lastq = qty;

		for(std::list<DemPoint>::iterator dpit=demptlist.begin(); dpit!=demptlist.end(); dpit++)
		{
			for(std::list<DemCompo>::iterator cit=dpit->demcompo.begin(); cit!=dpit->demcompo.end(); cit++)
			{
				if(cit->source != bi)
					continue;

				qty += cit->amount;
			}
		}

		DataPoint pt;
		pt.price = price;
		pt.qty = qty;
		datapt->push_back(pt);

		if(qty <= 0)
			break;

		if(price >= INST_FUNDS / 4)
			break;

		if(datapt->size())
		{
			DataPoint* first = &*datapt->begin();
			
			//prevent elongated graphs with tiny values
			if(qty < first->qty / 100)
				break;
		}

		price = price * 3/2 + 1;
		qty = 0;
	}
}

void DrawBlGraph_Dem(RichText* bname, BlType* bt, int32_t bi, int32_t gi, int32_t ri)
{
	Player* py = &g_player[g_localP];
	Client* c = &g_client[py->client];
	
	GUI *gui = &g_gui;
	BlGraphs *blg = (BlGraphs*)gui->get("bl graphs");
	float* frame = blg->m_pos;

#if 0
	if(c->speed != SPEED_PAUSE)
	{
		RichText rt = RichText("Must be paused to calculate curves.");
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+30, &rt);
		return;
	}
#endif

	Building* b = &g_building[bi];

	Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
	
	EndS();
	UseS(SHADER_COLOR2D);
	Shader *s = &g_shader[g_curS];

	static std::list<DataPoint> dempt;
	
	static std::list<DataPoint> suppt;
	static std::list<DataPoint> avcpt;
	static std::list<DataPoint> mgcpt;
	static std::list<DataPoint> wagept;
	
	//if(GetTicks()-g_graphstart > 30000)
	if(GetTicks()-g_graphstart > 3*g_graphtime)
	{
		g_graphstart = GetTicks();
		
		dempt.clear();
		suppt.clear();
		avcpt.clear();
		mgcpt.clear();
		wagept.clear();
		
		CalcDem(b, bi, &dempt, ri);
		CalcSup(b, &suppt, &avcpt, &mgcpt, &wagept, ri, &dempt);
		
		g_graphtime = GetTicks() - g_graphstart;
	}
	
	DrawBlGraph_Dem_DrawData(frame, &dempt, &suppt, &avcpt, &mgcpt, &wagept, blg, ri, bt->output[ri]);
	//DrawBlGraph_Sup_DrawData(frame, &datapt, &suppt, blg, ri);

	if(ri == RES_LABOUR)
	{
		//from labourer suppliers
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+300, &rt);
	}
#if 0
	else if(ri == RES_HOUSING)
	{
		//to labourer demanders
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+300, &rt);
	}
	else if(ri == RES_RETFUEL)
	{
		//to truck demanders
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+300, &rt);
	}
#endif

#if 0
	RichText note = RichText("Note: the demand, supply, average, and marginal cost curves reflect the amount of output per input, "\
		"the change in the number of required suppliers of raw resources and labour at each level of output and the associated change in costs, "\
		"which depends on the current distances separating prospective labour suppliers and the utility function score based on the relation between the effect of distance and changes in wage, "
		"the asking prices of the required raw materials and associated transport costs, "\
		"and the number of alternative, competing demanders of the labour at each point in the utility score range. "\
		"The curves are approximations and are only as accurate as the underlying simulation data. "\
		"The may change from moment to moment, or from hour to hour, depending on the state of the simulation.");
#endif

	float nc[4] = {0.7,0.7,0.7,0.7};

	DrawBoxTextF(MAINFONT8, frame[0]+105, frame[1]+139, frame[2] - frame[0] - 110, frame[3] - frame[1] - 139, &STRTABLE[STR_CVNOTE], nc, 0, -1, frame[0], frame[1], frame[2], frame[3]);

#if 0
	char m[128] = "Under construction";
	sprintf(m, "dem %s datapts%d maxpr%f maxq%f endq%f,p%f", g_resource[ri].name.c_str(), (int32_t)datapt.size(), maxpr, maxqty, (float)datapt.rbegin()->qty, (float)datapt.rbegin()->price);
	RichText rt = RichText(m);

	DrawLine(MAINFONT16, frame[0], frame[1], &rt);
#endif
}

void DrawBlGraph_DemEx(RichText* bname, BlType* bt, int32_t bi, int32_t gi, int32_t ri)
{
	Player* py = &g_player[g_localP];
	Client* c = &g_client[py->client];
	
	GUI *gui = &g_gui;
	BlGraphs *blg = (BlGraphs*)gui->get("bl graphs");
	float* frame = blg->m_pos;

	if(c->speed != SPEED_PAUSE)
	{
		RichText rt = RichText("Must be paused to calculate exact curves.");
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+30, &rt);
		return;
	}

	Building* b = &g_building[bi];

	Vec2i bcmpos = b->tpos * TILE_SIZE + Vec2i(TILE_SIZE/2, TILE_SIZE/2);
	
	EndS();
	UseS(SHADER_COLOR2D);
	Shader *s = &g_shader[g_curS];

	std::list<DataPoint> dempt;
	
	CalcDem(b, bi, &dempt, ri);
	
	std::list<DataPoint> suppt;
	std::list<DataPoint> avcpt;
	std::list<DataPoint> mgcpt;
	std::list<DataPoint> wagept;

	CalcSup(b, &suppt, &avcpt, &mgcpt, &wagept, ri, &dempt);

	DrawBlGraph_Dem_DrawData(frame, &dempt, &suppt, &avcpt, &mgcpt, &wagept, blg, ri, bt->output[ri]);
	//DrawBlGraph_Sup_DrawData(frame, &datapt, &suppt, blg, ri);

	if(ri == RES_LABOUR)
	{
		//from labourer suppliers
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+30, &rt);
	}
	else if(ri == RES_HOUSING)
	{
		//to labourer demanders
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+30, &rt);
	}
	else if(ri == RES_RETFUEL)
	{
		//to truck demanders
		RichText rt = STRTABLE[STR_DATAUNAV];
		DrawLine(MAINFONT16, frame[0]+30, frame[1]+30, &rt);
	}
#if 0
	char m[128] = "Under construction";
	sprintf(m, "dem %s datapts%d maxpr%f maxq%f endq%f,p%f", g_resource[ri].name.c_str(), (int32_t)datapt.size(), maxpr, maxqty, (float)datapt.rbegin()->qty, (float)datapt.rbegin()->price);
	RichText rt = RichText(m);

	DrawLine(MAINFONT16, frame[0], frame[1], &rt);
#endif
}

void DrawBlGraph_InOut(RichText* bname, BlType* bt, int32_t bi, int32_t gi, int32_t ri)
{
	Building* b = &g_building[bi];

	GUI *gui = &g_gui;
	BlGraphs *blg = (BlGraphs*)gui->get("bl graphs");

	float *frame = blg->m_pos;
	
	int32_t nhists = b->cyclehist.size();
	
	float wspan = frame[2] - frame[0];
	float wdiv = wspan / (float)(nhists + 2);
	
	int32_t greatest = 0;
	
	EndS();
	UseS(SHADER_COLOR2D);
	Shader *s = &g_shader[g_curS];

	int32_t chin = 1;
	for(std::list<CycleHist>::iterator chit=b->cyclehist.begin(); chit!=b->cyclehist.end(); chit++, ++chin)
	{
		//for(int32_t ri=0; ri<RESOURCES; ++ri)
		{
			/*if(bt->input[ri] <= 0 &&
			   bt->output[ri] <= 0 &&
			   ri != RES_DOLLARS)
				continue;
			*/
			greatest = imax(greatest, chit->cons[ri]);
			greatest = imax(greatest, chit->prod[ri]);
			
			DrawLine(0, 0, 0, 0.5f, frame[0] + wdiv * chin, frame[1], frame[0] + wdiv * chin, frame[3], blg->m_crop);
		}
	}
	
	float hspan = frame[3] - frame[1];
	float hscale = hspan / 2.0f / (1.0f + (float)greatest);
	
	int32_t zero[RESOURCES];
	Zero(zero);
	int32_t *previn = zero;
	int32_t *prevout = zero;
	
	float midy = (frame[3]+frame[1])/2.0f;
	
	//TODO cache graph on a backbuffer
	
	DrawLine(0, 0, 0, 1, frame[0], midy, frame[2], midy, blg->m_crop);
	
	//glLineWidth(2.0f);
	glLineWidth(1.0f);

	chin = 1;
	for(std::list<CycleHist>::iterator chit=b->cyclehist.begin(); chit!=b->cyclehist.end(); chit++, ++chin)
	{
		//for(int32_t ri=0; ri<RESOURCES; ++ri)
		{/*
			if(bt->input[ri] <= 0 &&
			   bt->output[ri] <= 0 &&
			   ri != RES_DOLLARS)
				continue;
			*/
			Resource *r = &g_resource[ri];
			float *color = r->rgba;
			
			Vec2f linein[2], lineout[2];
			
			linein[0].y = midy + hscale * previn[ri];
			linein[0].x = frame[0] + wdiv * chin;
			linein[1].y = midy + hscale * chit->cons[ri];
			linein[1].x = frame[0] + wdiv * (chin + 1);
			
			lineout[0].y = midy - hscale * prevout[ri];
			lineout[0].x = frame[0] + wdiv * chin;
			lineout[1].y = midy - hscale * chit->prod[ri];
			lineout[1].x = frame[0] + wdiv * (chin + 1);
			
			DrawLine(color[0], color[1], color[2], color[3], linein[0].x, linein[0].y, linein[1].x, linein[1].y, blg->m_crop);
			DrawLine(color[0], color[1], color[2], color[3], lineout[0].x, lineout[0].y, lineout[1].x, lineout[1].y, blg->m_crop);
		}
		
		previn = chit->cons;
		prevout = chit->prod;
	}

	glLineWidth(1.0f);
			 
	EndS();
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
	
	BmpFont *f = &g_font[MAINFONT16];
	BmpFont *f32 = &g_font[MAINFONT32];
	
	float y = frame[1] + f32->gheight;
	
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(bt->input[ri] <= 0 &&
		   bt->output[ri] <= 0 &&
		   ri != RES_DOLLARS)
			continue;

		Resource *r = &g_resource[ri];
		RichText line = RichText(r->name.c_str());
		DrawShadowedText(MAINFONT16, frame[0]+f->gheight * 5, y, &line, r->rgba, -1);
		y+= f->gheight;
	}
	
	float yspace = midy - frame[1];
	float ylinesfit = yspace / f->gheight;
	float yincrement = (float)greatest / ylinesfit;
	
	float yval = yincrement;
	float color[4] = {1,1,1,1};
	for(y=midy - f->gheight; y>frame[1]; y-=f->gheight, yval+=yincrement)
	{
		char ct[32];
		sprintf(ct, "+%s", iform((int32_t)yval).c_str());
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT8, frame[0], y, &rt, color, -1);
	}
	
	yval = yincrement;
	for(y=midy; y<frame[3]; y+=f->gheight, yval+=yincrement)
	{
		char ct[32];
		sprintf(ct, "-%s", iform((int32_t)yval).c_str());
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT8, frame[0], y, &rt, color, -1);
	}
	
	float xspace = chin * wdiv;
	float xlinesfit = xspace / (f->gheight * 4);
	float xincrement = (float)b->cyclehist.size() / xlinesfit;
	
	float xval = 0;
	for(float x=frame[0]+wdiv; xval<b->cyclehist.size(); xval+=xincrement, x+=xincrement*wdiv)
	{
		char ct[32];
		sprintf(ct, "%d:00", (int32_t)xval - nhists);
		RichText rt = RichText(ct);
		DrawShadowedText(MAINFONT8, x, frame[3] - f->gheight, &rt, color, -1);
	}
	
	RichText rtin = (STRTABLE[STR_EARNPROD]);
	RichText rtout = (STRTABLE[STR_EXPCON]);
	
	DrawShadowedText(MAINFONT16, frame[0] + f->gheight * 10, midy - f->gheight, &rtin, color, -1);
	DrawShadowedText(MAINFONT16, frame[0] + f->gheight * 10, midy - 1, &rtout, color, -1);
}

void DrawBlGraphs()
{
	Selection *sel = &g_sel;
	
	RichText bname;
	int32_t* price;
	
	Player* py = &g_player[g_localP];
	BlType* bt = NULL;
	Building* b = NULL;
	bool owned = false;	//owned by current player?
	Player* opy;
	int32_t bi;
	uint8_t bti;
	
	if(sel->buildings.size() > 0)
	{
		bi = *sel->buildings.begin();
		b = &g_building[bi];
		bti = b->type;
		bt = &g_bltype[b->type];
		
		if(b->owner == g_localP)
			owned = true;
		
		g_bptype = b->type;
		price = b->price;
		
		bname = RichText(UStr(bt->name));
		
#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
					(int32_t)b->transporter[RES_URANIUM],
					(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
					(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
					bi,
					(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
					(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif
		
		opy = &g_player[b->owner];
	}
	else 
		return;
	
	GUI *gui = &g_gui;
	BlGraphs *blg = (BlGraphs*)gui->get("bl graphs");
	
	int32_t ri = 0;
	DropList *rsel = &blg->m_rsel;
	int32_t sgi = rsel->m_selected;
	//ri = rsel->m_selected;
	uint8_t ggi = BlTypeGraphIn(bti, sgi);

	if(ggi < 0)
		return;

	if(ggi >= BLGR_INOUT_BEG && ggi < BLGR_INOUT_END)
		ri = ggi - BLGR_INOUT_BEG;
	else if(ggi >= BLGR_DEM_BEG && ggi < BLGR_DEM_END)
		ri = ggi - BLGR_DEM_BEG;
	else if(ggi >= BLGR_DEMEX_BEG && ggi < BLGR_DEMEX_END)
		ri = ggi - BLGR_DEMEX_BEG;
	
	if(ggi >= BLGR_INOUT_BEG && ggi < BLGR_INOUT_END)
		DrawBlGraph_InOut(&bname, bt, bi, ggi, ri);
	else if(ggi >= BLGR_DEM_BEG && ggi < BLGR_DEM_END)
		DrawBlGraph_Dem(&bname, bt, bi, ggi, ri);
	else if(ggi >= BLGR_DEMEX_BEG && ggi < BLGR_DEMEX_END)
		DrawBlGraph_DemEx(&bname, bt, bi, ggi, ri);

	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
}

void Resize_BG_Close(Widget *thisw)
{
	Widget *parw = thisw->m_parent;
	
	thisw->m_pos[0] = parw->m_pos[0];
	thisw->m_pos[1] = parw->m_pos[3]-30;
	thisw->m_pos[2] = parw->m_pos[0]+90;
	thisw->m_pos[3] = parw->m_pos[3];
	
	CenterLabel(thisw);
}

void Click_BG_Close()
{
	GUI *gui = &g_gui;
	gui->hide("bl graphs");
}

void Resize_BG_RSel(Widget *thisw)
{
	Widget *parw = thisw->m_parent;
	
	BmpFont *f = &g_font[thisw->m_font];
	
	thisw->m_pos[0] = parw->m_pos[2] - 190;
	thisw->m_pos[1] = parw->m_pos[3] - 60;
	thisw->m_pos[2] = parw->m_pos[2];
	thisw->m_pos[3] = parw->m_pos[3] - 60 + f->gheight;
}

void Change_BlGraph()
{
	g_graphstart = 0;
}

//TODO mouseover manuf button resource costs
BlGraphs::BlGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BLGRAPHS;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	
	m_close = Button(this, "hide", "gui/transp.png", (STRTABLE[STR_CLOSE]), RichText(), MAINFONT16, BUST_LINEBASED, Resize_BG_Close, Click_BG_Close, NULL, NULL, NULL, NULL, -1, NULL);
	m_graphs = InsDraw(this, DrawBlGraphs);
	m_rsel = DropList(this, "rsel", MAINFONT16, Resize_BG_RSel, Change_BlGraph);
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}

void BlGraphs::regvals(Selection* sel)
{

}

void BlGraphs::draw()
{
	Win::draw();

	m_graphs.draw();
	m_close.draw();
	m_rsel.draw();
}

void BlGraphs::drawover()
{
	Win::drawover();

	m_close.drawover();
	m_rsel.drawover();
}

void BlGraphs::reframe()
{
	Win::reframe();

	m_close.reframe();
	m_rsel.reframe();
}

void BlGraphs::inev(InEv* ie)
{
	m_close.inev(ie);
	m_rsel.inev(ie);

	Win::inev(ie);
}

//viewport
void Resize_BG_VP(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1];
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];
}

//title
void Resize_BG_Tl(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10 + 250;
	w->m_pos[1] = parw->m_pos[1] + 10;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[3] - 10;
}

//owner name
void Resize_BG_Ow(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 15 + 250;
	w->m_pos[1] = parw->m_pos[1] + 39;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[3] - 10;
}

bool ValidBlInOutRes(uint8_t bti, uint8_t ri)
{
	BlType* bt = &g_bltype[bti];
	
	if(ri == RES_DOLLARS)
		return true;

	if(bt->input[ri])
		return true;
	if(bt->output[ri])
		return true;
	return false;
}

bool ValidBlDemRes(uint8_t bti, uint8_t ri)
{
	//return false;
	BlType* bt = &g_bltype[bti];
	
	if(bt->output[ri])
		return true;
	return false;
}

uint8_t BlTypeGraphIn(uint8_t bti, uint8_t sgi)
{
	uint8_t ggi = -1;

	//BLGR_INOUT_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		ggi++;

		if(!ValidBlInOutRes(bti, ri))
			continue;

		if(!sgi)
			return ggi;
		--sgi;
	}

	//return 0;

#if 1
	//BLGR_DEM_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		ggi++;

		if(!ValidBlDemRes(bti, ri))
			continue;
		
		if(!sgi)
			return ggi;
		--sgi;
	}
#endif
	
	return 0;

#if 1
	//BLGR_DEMEX_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		ggi++;

		if(!ValidBlDemRes(bti, ri))
			continue;
		
		if(!sgi)
			return ggi;
		--sgi;
	}
#endif

	return 0;
}

void BlGraphs::regen(Selection* sel)
{
	RichText bname;
	int32_t* price;

	Player* py = &g_player[g_localP];
	BlType* bt = NULL;
	Building* b = NULL;
	bool owned = false;	//owned by current player?
	Player* opy;
	int32_t bi;
	uint8_t bti;

	if(sel->buildings.size() > 0)
	{
		bi = *sel->buildings.begin();
		b = &g_building[bi];
		bti = b->type;
		bt = &g_bltype[b->type];

		if(b->owner == g_localP)
			owned = true;

		g_bptype = b->type;
		price = b->price;

		bname = RichText(UStr(bt->name));

#if 0
		if(b->type == BL_NUCPOW)
		{
			char msg[1280];
			sprintf(msg, "blview \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n u->cargotype=%d",
				(int32_t)b->transporter[RES_URANIUM],
				(int32_t)g_unit[b->transporter[RES_URANIUM]].mode,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].target,
				bi,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].targtype,
				(int32_t)g_unit[b->transporter[RES_URANIUM]].cargotype);
			InfoMess(msg, msg);
		}
#endif

		opy = &g_player[b->owner];

		g_graphstart = 0;
	}
#if 0
	else if(sel->roads.size() > 0)
	{
		g_bptype = BL_ROAD;
		conmat = g_roadcost;
		qty = sel->roads.size();
		Vec2i tpos = *sel->roads.begin();
		RoadTile* road = RoadAt(tpos.x, tpos.y);
		maxcost = road->maxcost;
	}
	else if(sel->powls.size() > 0)
	{
		g_bptype = BL_POWL;
		conmat = g_powlcost;
		qty = sel->powls.size();
		Vec2i tpos = *sel->powls.begin();
		PowlTile* powl = PowlAt(tpos.x, tpos.y);
		maxcost = powl->maxcost;
	}
	else if(sel->crpipes.size() > 0)
	{
		g_bptype = BL_CRPIPE;
		qty = sel->crpipes.size();
		conmat = g_crpipecost;
		Vec2i tpos = *sel->crpipes.begin();
		CrPipeTile* crpipe = CrPipeAt(tpos.x, tpos.y);
		maxcost = crpipe->maxcost;
	}
#endif

	freech();

	RichText ownname = opy->name;
	if(opy->client >= 0)
	{
		Client* c = &g_client[opy->client];
		ownname = c->name;
	}

	//add(new Viewport(this, "viewport", Resize_BG_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	add(new Text(this, "owner", ownname, MAINFONT16, Resize_BG_Ow, true, opy->color[0], opy->color[1], opy->color[2], opy->color[3]));
	add(new Text(this, "title", bname, MAINFONT32, Resize_BG_Tl, true, 0.9f, 0.7f, 0.3f, 1));

	int32_t row = 0;
	
	m_rsel.m_options.clear();
	
	//BLGR_INOUT_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(!ValidBlInOutRes(bti, ri))
			continue;

		Resource *r = &g_resource[ri];
		RichText rt = RichText(r->name.c_str()) + RichText(" ") + STRTABLE[STR_INOUT];
		m_rsel.m_options.push_back(rt);
	}

#if 1
	//BLGR_DEM_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(!ValidBlDemRes(bti, ri))
			continue;

		Resource *r = &g_resource[ri];
		RichText rt = RichText(r->name.c_str()) + RichText(" ") + STRTABLE[STR_DEMSUPAVC];
		m_rsel.m_options.push_back(rt);
	}
#endif
	
#if 0
	//BLGR_DEMEX_BEG
	for(int32_t ri=0; ri<RESOURCES; ++ri)
	{
		if(!ValidBlDemRes(bti, ri))
			continue;

		Resource *r = &g_resource[ri];
		RichText rt = RichText("Ex. ") + RichText(r->name.c_str()) + RichText(" ") + STRTABLE[STR_DEMSUPAVC];
		m_rsel.m_options.push_back(rt);
	}
#endif

	reframe();
}
