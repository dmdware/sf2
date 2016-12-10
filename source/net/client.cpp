












#include "client.h"

#ifndef MATCHMAKER
#include "../gui/layouts/chattext.h"
#include "../language.h"
#endif

Client g_cl[CLIENTS];
int g_localC;
char g_name[GENNAMELEN];

void ResetCls()
{
	g_localC = -1;
	g_localP = -1;
	g_speed = SPEED_PLAY;

	for(int i=0; i<CLIENTS; i++)
	{
		Client* c = &g_cl[i];

		c->on = ecfalse;
		c->name = RichText("Py");
		//c->color = 0;

		if(c->nc)
		{
			Disconnect(c->nc);
			c->nc->client = -1;
			c->nc = NULL;
		}

		//c->nc = NULL;
		c->ready = ecfalse;
		c->unresp = ecfalse;
		c->speed = SPEED_PLAY;
	}

	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

		py->client = -1;
	}
}

void ResetCl(Client* c)
{
	c->on = ecfalse;
	c->name = RichText("Py");
	c->ready = ecfalse;
	c->unresp = ecfalse;
	c->speed = SPEED_PLAY;

	if(c->nc)
	{
		c->nc->client = -1;
		c->nc = NULL;
	}

	if(c->player >= 0)
	{
		Py* py = &g_py[c->player];
		py->client = -1;
		//py->on = ecfalse;
		c->player = -1;
	}

	//if(c - g_cl == g_localC)
	//	g_speed = SPEED_PLAY;

	delete c->joinstate;
	c->joinstate = NULL;
	c->afterjoin.clear();

	UpdSpeed();
}

int NewClient()
{
	for(int i=0; i<CLIENTS; i++)
		if(!g_cl[i].on)
			return i;

	return -1;
}

//version where we already have a client index
void AddClient(NetConn* nc, RichText name, int ci, unsigned char clplati)
{
	if(nc)
		nc->client = ci;

	Client* c = &g_cl[ci];

	c->on = ectrue;
	c->player = -1;
	c->name = name;
	c->nc = nc;
	c->speed = SPEED_PLAY;
	c->downin = -1;
	c->clplat = clplati;

	if(nc)
		nc->client = ci;

#ifndef MATCHMAKER
	RichText clplat = RichText("?");

	if(clplati < CLPLAT_TYPES)
		clplat = RichText(CLPLATSTR[clplati]);

	RichText msg = name + RichText(" ") + STRTABLE[STR_JOINEDGAME] + RichText(" ") + clplat + RichText(".");
	AddNotif(&msg);
#endif
}

//version that gets a new client index from the slots
ecbool AddClient(NetConn* nc, RichText name, int* retci, unsigned char clplati)
{
	int ci = NewClient();

	if(ci < 0)
		return ecfalse;

	//calls the other version here
	AddClient(nc, name, ci, clplati);

	if(retci)
		*retci = ci;

	return ectrue;
}
