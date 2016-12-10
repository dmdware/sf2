













#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "../platform.h"
#include "../sim/player.h"

struct VisTile
{
public:
	//std::list<short> uvis;
	//std::list<short> bvis;
	unsigned char vis[PLAYERS];
	ecbool explored[PLAYERS];

	VisTile()
	{
		for(int i=0; i<PLAYERS; i++)
		{
			explored[i] = ecfalse;
			//vis[i] = 0;
		}
	}
};

extern VisTile* g_vistile;

struct Mv;
struct Bl;

void RemVis(Mv* mv);
void AddVis(Mv* mv);
void RemVis(Bl* b);
void AddVis(Bl* b);
void Explore(Mv* mv);
void Explore(Bl* b);
ecbool IsTileVis(short py, short tx, short ty);
ecbool Explored(short py, short tx, short ty);
void UpdVis();

#endif