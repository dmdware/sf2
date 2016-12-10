













#ifndef SIMSTATE_H
#define SIMSTATE_H

#include "unit.h"
#include "conduit.h"
#include "building.h"
#include "../render/foliage.h"
#include "../render/heightmap.h"
#include "../render/fogofwar.h"
#include "border.h"

//Everything inside this struct must:
//1. Change completely deterministically
//2. Be synchronized between clients

struct SimState
{
public:
	//unsigned __int64 simframe;
	Mv unit[MOVERS];
	Bl bl[BUILDINGS];
	CdTile* cd[CD_TYPES][2];	//cdtiles, 0 = actual placed, 1 = plan proposed
	Foliage fl[FOLIAGES];
	signed char* border;
	VisTile* vistile;
	//TO DO

	SimState()
	{
		border = NULL;
		vistile = NULL;

		for(unsigned char cdtype=0; cdtype<CD_TYPES; ++cdtype)
		{
			for(unsigned char fin=0; fin<2; ++fin)
			{
				cd[cdtype][fin] = NULL;
			}
		}
	}

	void free()
	{
		for(int i=0; i<MOVERS; ++i)
			unit[i].destroy();

		for(int i=0; i<BUILDINGS; ++i)
			bl[i].destroy();

		for(int i=0; i<FOLIAGES; ++i)
			fl[i].destroy();

		for(unsigned char cdtype=0; cdtype<CD_TYPES; ++cdtype)
		{
			CdType* ct = &g_cdtype[cdtype];

			//if(!ct->on)
			//	continue;

			for(unsigned char fin=0; fin<2; ++fin)
			{
				for(int i=0; i<g_mapsz.x*g_mapsz.y; ++i)
					cd[cdtype][fin][i].destroy();

				delete cd[cdtype][fin];
				cd[cdtype][fin] = NULL;
			}
		}

		delete border;
		border = NULL;

		delete vistile;
		vistile = NULL;
	}

	~SimState()
	{
		free();
	}
	//TODO assign op
};

extern SimState* g_joinstate;	//for client buffering

void UnpackSimState(SimState* ss);
void PackSimState(SimState* ss);

#endif