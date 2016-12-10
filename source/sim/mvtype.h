











#ifndef MVTYPE_H
#define MVTYPE_H

#include "../math/vec3i.h"
#include "../math/vec2i.h"
#include "../math/vec2s.h"
#include "../render/heightmap.h"
#include "resources.h"
#include "../math/vec3f.h"
#include "../path/pathnode.h"

#define USND_SEL		0	//unit selected
#define U_SOUNDS		1

struct MvType
{
public:
#if 0
	unsigned int texindex;
	Vec2i bilbsize;
#endif
	//ecbool on;
	//unsigned int* sprite[DIRS];
	unsigned int splist;	//sprite list
	int nframes;
	Vec2s size;
	char name[256];
	int starthp;
	int cmspeed;
	int cost[RESOURCES];
	ecbool walker;
	ecbool landborne;
	ecbool roaded;
	ecbool seaborne;
	ecbool airborne;
	ecbool military;
	short visrange;
	short sound[U_SOUNDS];
	int prop;	//proportion to bl ratio out of RATIO_DENOM

	MvType()
	{
#if 0
		//TODO move all bl, u, cd constructors to Init();
		for(int s=0; s<DIRS; s++)
		{
			sprite[s] = NULL;
		}
#endif

		nframes = 0;
		//on = ecfalse;
	}

	~MvType()
	{
		free();
	}

	void free()
	{
#if 0
		for(int s=0; s<DIRS; s++)
		{
			if(!sprite[s])
				continue;

			delete [] sprite[s];
			sprite[s] = NULL;
		}
#endif

		nframes = 0;
		//on = ecfalse;
	}
};

#if 0
#define MV_LABOURER		0
#define MV_BATTLECOMP		1
#define MV_TRUCK			2
#define MV_CARLYLE		3
#define MV_TYPES			4
#else
#define MV_LABOURER		0
#define MV_TRUCK			1
#define MV_TYPES			2
#endif

//#define MV_TYPES		128

extern MvType g_mvtype[MV_TYPES];

void DefU(int type, const char* sprel, int nframes,
	Vec2s size, const char* name,
	int starthp,
	ecbool landborne, ecbool walker, ecbool roaded, ecbool seaborne, ecbool airborne,
	int cmspeed, ecbool military,
	int visrange,
	int prop);
void UCost(int type, int res, int amt);

#endif
