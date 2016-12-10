













#include "condition.h"
#include "../sim/building.h"
#include "../sim/conduit.h"
#include "../platform.h"
#include "../sim/player.h"
#include "../sim/map.h"

Condition::Condition()
{
	type = CONDITION_NONE;
	strcpy(name, "");
	met = ecfalse;
	target = 0;
	fval = 0;
	count = 0;
	owner = 0;
}

void OnLoadMap()
{
	ConditionMet(CONDITION_ONLOADMAP, 0, 0, 0, 0);
}

void OnRoadInacc()
{
	ConditionMet(CONDITION_ROADINACCESSABILITY, 0, 0, 0, 0);
}

void OnAllRoadAc()
{
	ConditionMet(CONDITION_ALLROADACCESSIBLE, 0, 0, 0, 0);
}

void OnCaptureB(int target)
{
	ConditionMet(CONDITION_CAPTUREB, target, 0, 0, 0);
}

void OnPlaceB(int target)
{
	if(ConditionMet(CONDITION_ONPLACEB, target, 0, 0, 0))
		return;
	if(ConditionMet(CONDITION_ONPLACEOTHERB, target, 0, 0, 0))
		return;
}

void OnBuildU(int target)
{
	ConditionMet(CONDITION_ONBUILDU, target, 0, 0, 0);
}

void OnFinishedB(int target)
{
	if(ConditionMet(CONDITION_ONFINISHEDB, target, 0, 0, 0))
		return;
    
	//check if all the player's constructions are finished
    
	for(int i=0; i<BUILDINGS; i++)
	{
		Bl* b = &g_bl[i];
        
		if(b->on && !b->finished && b->owner == g_localP)
			return;
	}
    
	for(unsigned char cdtype=0; cdtype<CD_TYPES; cdtype++)
		for(int x=0; x<g_mapsz.x; x++)
			for(int y=0; y<g_mapsz.y; y++)
			{
				CdTile* ctile = GetCd(cdtype, x, y, ecfalse);

				if(!ctile->on)
					continue;

				if(ctile->finished)
					continue;

				if(ctile->owner != g_localP)
					continue;

				return;
			}
    
	ConditionMet(CONDITION_ALLCONSTRUCTIONFINI, 0, 0, 0, 0);
}

void OnPriceSet(int targ, float val)
{
	ConditionMet(CONDITION_PRICESET, targ, val, 0, 0);
}

void OnCloseMessage()
{
	ConditionMet(CONDITION_MESSAGECLOSED, 0, 0, 0, 0);
}

void Click_CloseMessage()
{
	Widget *gui = (Widget*)&g_gui;
	gui->hide("game message");
	OnCloseMessage();
}