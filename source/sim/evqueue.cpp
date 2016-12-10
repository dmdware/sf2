









#include "simflow.h"
#include "evqueue.h"
#include "../net/client.h"
#include "unit.h"

std::list<SimEv> g_simev;

void PlanEvent(unsigned short type, unsigned __int64 execframe, Mv* mv)
{
	SimEv se;

	se.evtype = type;
	se.planframe = g_simframe;
	se.execframe = execframe;

	std::list<SimEv>::iterator sit=g_simev.begin();
	ecbool place = ecfalse;

	while(ectrue)
	{
		if(sit == g_simev.end())
		{
			place = ectrue;
		}
		else
		{
			SimEv* spotse = &*sit;

			if(spotse->execframe > execframe)
				place = ectrue;
		}

		if(!place)
			sit++;

		g_simev.insert(sit, se);
		break;
	}
}

void ExecEvent(SimEv* se)
{
}

void ExecEvents()
{
}