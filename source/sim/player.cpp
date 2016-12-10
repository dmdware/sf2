











#include "player.h"
#include "resources.h"
#include "../sim/building.h"
#include "../trigger/console.h"
#include "../net/client.h"
#include "simflow.h"
#include "simdef.h"
#include "../net/lockstep.h"
#include "building.h"
#include "unit.h"

#ifndef MATCHMAKER
#include "../gui/cursor.h"
#include "../render/heightmap.h"
#include "../sim/build.h"
#include "../gui/layouts/chattext.h"
#include "../ai/ai.h"
#endif

PlayerColor g_pycols[PLAYER_COLORS] =
{
	{{0x7e, 0x1e, 0x9c}, "Purple"},
	{{0x15, 0xb0, 0x1a}, "Green"},
	{{0x03, 0x43, 0xdf}, "Blue"},
	{{0xff, 0x81, 0xc0}, "Pink"},
	{{0x65, 0x37, 0x00}, "Brown"},
	{{0xe5, 0x00, 0x00}, "Red"},
	{{0x95, 0xd0, 0xfc}, "Light Blue"},
	{{0x02, 0x93, 0x86}, "Teal"},
	{{0xf9, 0x73, 0x06}, "Orange"},
	{{0x96, 0xf9, 0x7b}, "Light Green"},
	{{0xc2, 0x00, 0x78}, "Magenta"},
	{{0xff, 0xff, 0x14}, "Yellow"},
	{{0x75, 0xbb, 0xfd}, "Sky Blue"},
	{{0x92, 0x95, 0x91}, "Grey"},
	{{0x89, 0xfe, 0x05}, "Lime Green"},
	{{0xbf, 0x77, 0xf6}, "Light Purple"},
	{{0x9a, 0x0e, 0xea}, "Violet"},
	{{0x33, 0x35, 0x00}, "Dark Green"},
	{{0x06, 0xc2, 0xac}, "Turquoise"},
	{{0xc7, 0x9f, 0xef}, "Lavender"},
	{{0x00, 0x03, 0x5b}, "Dark Blue"},
	{{0xd1, 0xb2, 0x6f}, "Tan"},
	{{0x00, 0xff, 0xff}, "Cyan"},
	{{0x13, 0xea, 0xc9}, "Aqua"},
	{{0x06, 0x47, 0x0c}, "Forest Green"},
	{{0xae, 0x71, 0x81}, "Mauve"},
	{{0x35, 0x06, 0x3e}, "Dark Purple"},
	{{0x01, 0xff, 0x07}, "Bright Green"},
	{{0x65, 0x00, 0x21}, "Maroon"},
	{{0x6e, 0x75, 0x0e}, "Olive"},
	{{0xff, 0x79, 0x6c}, "Salmon"},
	{{0xe6, 0xda, 0xa6}, "Beige"},
	{{0x05, 0x04, 0xaa}, "Royal Blue"},
	{{0x00, 0x11, 0x46}, "Navy Blue"},
	{{0xce, 0xa2, 0xfd}, "Lilac"},
	{{0x00, 0x00, 0x00}, "Black"},
	{{0xff, 0x02, 0x8d}, "Hot Pink"},
	{{0xad, 0x81, 0x50}, "Light Brown"},
	{{0xc7, 0xfd, 0xb5}, "Pale Green"},
	{{0xff, 0xb0, 0x7c}, "Peach"},
	{{0x67, 0x7a, 0x04}, "Olive Green"},
	{{0xcb, 0x41, 0x6b}, "Dark Pink"},
	{{0x8e, 0x82, 0xfe}, "Periwinkle"},
	{{0x53, 0xfc, 0xa1}, "Sea Green"},
	{{0xaa, 0xff, 0x32}, "Lime"},
	{{0x38, 0x02, 0x82}, "Indigo"},
	{{0xce, 0xb3, 0x01}, "Mustard"},
	{{0xff, 0xd1, 0xdf}, "Light Pink"}
};

Py g_py[PLAYERS];
int g_localP = 0;
int g_playerm;
ecbool g_diplomacy[PLAYERS][PLAYERS] = {1};

Py::Py()
{
	Zero(local);
	Zero(global);
	Zero(resch);
	truckwage = 0;
	transpcost = 0;
	on = ecfalse;
	client = -1;
	truckwage = DEFL_DRWAGE;
	transpcost = DEFL_TRFEE;
	util = 0;
	gnp = 0;
}

Py::~Py()
{
}

void FreePys()
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

		py->on = ectrue;
		py->ai = ecfalse;

		//py->insttype = INST_STATE;
		py->instin = (signed char)i;

		if(i % (FIRMSPERSTATE+1) == 0)
			py->insttype = INST_STATE;
		else
			py->insttype = INST_FIRM;

		py->parentst = i/(FIRMSPERSTATE+1)*(FIRMSPERSTATE+1);

		py->protectionism = ecfalse;
		py->imtariffratio = 0;
		py->extariffratio = 0;
		
		//py->lastthink = 0;
		py->lastthink = i*(NETTURN+2);

		//protectionist state player
		//if(i == PROTEC_PY)
		if(ecfalse)
		{
			py->protectionism = ectrue;
			//py->imtariffratio = RATIO_DENOM * 40 / 100;	//40% import tax on everything
			//py->imtariffratio = RATIO_DENOM * 70 * 000 / 100;	//70.000% import tax on everything
			//py->imtariffratio = RATIO_DENOM * 40 * 1000 / 100;	//40.000% import tax on everything
			py->imtariffratio = RATIO_DENOM * 50 * 1000 / 100;	//50.000% import tax on everything
			//py->imtariffratio = RATIO_DENOM * 40 * 1000 / 100;	//40.000% import tax on everything
			//py->extariffratio = RATIO_DENOM * 40 / 100;	//40% export tax on everything
		}

		if(!py->on)
			continue;

		if(py->client >= 0)
		{
			Client* c = &g_cl[py->client];
			c->player = -1;
			//ResetCl(c);
		}

		py->client = -1;
		//py->on = ecfalse;
		py->truckwage = DEFL_DRWAGE;
		py->transpcost = DEFL_TRFEE;
		py->util = 0;
		py->gnp = 0;
	}
}

void IniRes()
{
#if 1
	for(int i=0; i<PLAYERS; i++)
	{
		Py* p = &g_py[i];

		//p->on = ectrue;
		//p->ai = (i == g_localP) ? ecfalse : ectrue;
		//p->ai = ecfalse;

		p->global[RES_DOLLARS] = INST_FUNDS;
		p->global[RES_FARMPRODUCTS] = 90 * CYCLE_FRAMES/WORK_DELAY; //4000;
		p->global[RES_RETFOOD] = 90 * CYCLE_FRAMES/WORK_DELAY; //4000;
		p->global[RES_CEMENT] = 150;
		p->global[RES_STONE] = 4000;
		p->global[RES_RETFUEL] = 4000;
		p->global[RES_URANIUM] = 4000;
		//p->global[RES_ELECTRONICS] = 4000;
		p->global[RES_METAL] = 150;
		p->global[RES_PRODUCTION] = 400000;
	}
#endif
}

void AssocPy(int player, int client)
{
	if(player >= 0)
	{
		Py* py = &g_py[player];

		if(py->client >= 0 && py->client != client)
		{
			Client* c = &g_cl[py->client];
			c->player = -1;
		}

		py->client = client;
	}

	if(client >= 0)
	{
		Client* c = &g_cl[client];

		if(c->player >= 0 && c->player != player)
		{
			Py* py = &g_py[c->player];
			py->client = -1;
		}

		c->player = player;
	}

	if(client >= 0 && client == g_localC)
		g_localP = player;
}

int NewPlayer()
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

		if(!py->on)
			continue;

		if(py->client < 0)
			return i;
	}

	return -1;
}

int NewClPlayer()
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

		if(!py->on)
			continue;

		if(py->ai)
			continue;

		if(py->client < 0)
			return i;
	}

	return -1;
}

void DefP(int ID, float red, float green, float blue, float alpha, RichText name)
{
	Py* py = &g_py[ID];
	py->color[0] = red;
	py->color[1] = green;
	py->color[2] = blue;
	py->color[3] = alpha;
	py->name = name;
}

void Bankrupt(int player, const char* reason)
{
	Py* p = &g_py[player];

	//if(!p->on)
	//	return;

	if(player == g_localP)
	{
		RichText lm(UStr("You've gone bankrupt."));
		//SubmitConsole(&lm);
		AddNotif(&lm);
	}
	else //if(p->activity != ACTIVITY_NONE)
	{
		//LogTransx(player, 0.0f, "BANKRUPT");

		char msg[256];
		sprintf(msg, "%s has gone bankrupt", g_py[player].name.rawstr().c_str());

		char add[64];

		if(reason[0] != '\0')
			sprintf(add, " (reason: %s).", reason);
		else
			sprintf(add, ".");

		strcat(msg, add);

		RichText lm;
		lm.part.push_back(RichPart(UStr(msg)));
		//SubmitConsole(&lm);
		AddNotif(&lm);
	}

	//cse fix
	if( ( g_netmode == NETM_HOST ||
		g_netmode == NETM_SINGLE ) &&
		p->ai)
	{
		//sell off all the property and turn off AI player slot?

		if(p->on)
		{
			for(int bi=0; bi<BUILDINGS; bi++)
			{
				Bl* b = &g_bl[bi];

				if(!b->on)
					continue;

				if(b->owner != player)
					continue;

				if(b->forsale)
					continue;

				SetSalePropPacket sspp;
				sspp.header.type = PACKET_SETSALEPROP;
				sspp.propi = bi;
				sspp.proptype = PROP_BL_BEG + b->type;
				sspp.selling = ectrue;
				sspp.price = 0;
				LockCmd((PacketHeader*)&sspp);
			}

			for(int ui=0; ui<MOVERS; ui++)
			{
				Mv* mv = &g_mv[ui];

				if(!mv->on)
					continue;

				if(mv->owner != player)
					continue;

				if(mv->type == MV_LABOURER)
					continue;

				if(mv->forsale)
					continue;
			
				SetSalePropPacket sspp;
				sspp.header.type = PACKET_SETSALEPROP;
				sspp.propi = ui;
				sspp.proptype = PROP_U_BEG + mv->type;
				sspp.selling = ectrue;
				sspp.price = 0;
				LockCmd((PacketHeader*)&sspp);
			}

			for(unsigned char tx=0; tx<g_mapsz.x; ++tx)
			{
				for(unsigned char ty=0; ty<g_mapsz.y; ++ty)
				{
					for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
					{
						CdTile* ctile = GetCd(ctype, tx, ty, ecfalse);

						if(!ctile->on)
							continue;

						if(ctile->finished)
							continue;

						if(ctile->owner != player)
							continue;

						if(ctile->selling)
							continue;
					
						SetSalePropPacket sspp;
						sspp.header.type = PACKET_SETSALEPROP;
						sspp.propi = -1;
						sspp.tx = tx;
						sspp.ty = ty;
						sspp.proptype = PROP_CD_BEG + ctype;
						sspp.selling = ectrue;
						sspp.price = 0;
						LockCmd((PacketHeader*)&sspp);
					}
				}
			}
		}

#if 0
		//if(g_simframe / (CYCLE_FRAMES/10) == 0)
		for(unsigned char p2i=0; p2i<PLAYERS; ++p2i)
		{
			if(p2i == player)
				continue;

			Py* p2 = &g_py[p2i];

			if(!p2->on)
				continue;

			if(!p2->ai)
				continue;

			BuyProps(p2);
		}
#endif
	}

	if(p->ai)
		p->on = ecfalse;
}
