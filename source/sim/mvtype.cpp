











#include "mvtype.h"
#include "../texture.h"
#include "resources.h"
#include "../path/pathnode.h"
#include "../utils.h"
#include "../tool/rendersprite.h"

MvType g_mvtype[MV_TYPES];

void DefU(int type, const char* sprel, int nframes,
	Vec2s size, const char* name,
	int starthp,
	ecbool landborne, ecbool walker, ecbool roaded, ecbool seaborne, ecbool airborne,
	int cmspeed, ecbool military,
	int visrange,
	int prop)
{
	MvType* t = &g_mvtype[type];

	//QueueTex(
	//QueueModel(&t->model, sprel);

	t->free();

	//t->on = ectrue;

#if 0
	for(int s=0; s<DIRS; s++)
	{
		t->sprite[s] = new unsigned int [ nframes ];

		for(int f=0; f<nframes; f++)
		{
			char frrel[SFH_MAX_PATH+1];
			sprintf(frrel, "%s_si%d_fr%03d", sprel, s, f);
			QueueSprite(frrel, &t->sprite[s][f], ectrue, ectrue);
			//Log("q "<<frrel);
		}
	}
#elif 0
	if(!LoadSpriteList(sprel, &t->splist, ectrue, ectrue, ectrue))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
#else
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		ectrue, ecfalse, ectrue, ecfalse,
		nframes, DIRS, 
		ectrue, ectrue, ectrue, ectrue,
		1);
#endif

	t->nframes = nframes;

	t->size = size;
	strcpy(t->name, name);
	t->starthp = starthp;
	Zero(t->cost);
	t->landborne = landborne;
	t->walker = walker;
	t->roaded = roaded;
	t->seaborne = seaborne;
	t->airborne = airborne;
	t->cmspeed = cmspeed;
	t->military = military;
	t->visrange = visrange;
	t->prop = prop;

	for(int i=0; i<U_SOUNDS; i++)
		t->sound[i] = -1;
}

void UCost(int type, int res, int amt)
{
	MvType* t = &g_mvtype[type];
	t->cost[res] = amt;
}
