

#include "demoprop.h"
#include "../gui/layouts/demogui.h"
#include "../sim/player.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../sim/unit.h"
#include "../sim/mvtype.h"
#include "../sim/conduit.h"
#include "../sim/map.h"
#include "../net/lockstep.h"
#include "../gui/layouts/messbox.h"
#include "selection.h"

void Click_DemoProp()
{
	//RichText r = RichText("Demolish?");
	//Mess(&r);
	Widget *gui = (Widget*)&g_gui;
	gui->hide("bl view");

	DemolPropPacket dpp;
	dpp.header.type = PACKET_DEMOLPROP;
	dpp.pi = g_localP;

	if(g_sel.bl.size())
	{
		int bi = *g_sel.bl.begin();
		Bl* b = &g_bl[bi];
		dpp.propi = bi;
		dpp.proptype = PROP_BL_BEG + b->type;
		LockCmd((PacketHeader*)&dpp);
		g_sel.bl.clear();
	}
	else if(g_sel.roads.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_ROAD;

		for(std::list<Vec2i>::iterator ctit=g_sel.roads.begin(); ctit!=g_sel.roads.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.roads.clear();
	}
	else if(g_sel.powls.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_POWL;

		for(std::list<Vec2i>::iterator ctit=g_sel.powls.begin(); ctit!=g_sel.powls.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.powls.clear();
	}
	else if(g_sel.crpipes.size())
	{
		dpp.proptype = PROP_CD_BEG + CD_CRPIPE;

		for(std::list<Vec2i>::iterator ctit=g_sel.crpipes.begin(); ctit!=g_sel.crpipes.end(); ctit++)
		{
			dpp.tx = ctit->x;
			dpp.ty = ctit->y;
			dpp.propi = ctit->x + ctit->y * g_mapsz.x;
			LockCmd((PacketHeader*)&dpp);
		}

		g_sel.crpipes.clear();
	}
}

void Demolish(int pyi, int proptype, int propi, int tx, int ty)
{
	if(proptype >= PROP_BL_BEG &&
		proptype < PROP_BL_END)
	{
		Bl* b = &g_bl[propi];
		b->destroy();
		b->on = ecfalse;
	}
	else if(proptype >= PROP_CD_BEG &&
		proptype < PROP_CD_END)
	{
		int ctype = proptype = PROP_CD_BEG;

		CdTile* ctile = GetCd(ctype, tx, ty, ecfalse);

		ctile->destroy();
		ctile->on = ecfalse;
	}
}