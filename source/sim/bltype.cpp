












#include "bltype.h"
#include "../sound/sound.h"
#include "../tool/rendersprite.h"

BlType g_bltype[BL_TYPES];

BlType::BlType()
{
	//sprite = NULL;
	//csprite = NULL;
	nframes = 0;
	cnframes = 0;
	//on = ecfalse;
}

BlType::~BlType()
{
	free();
}

void BlType::free()
{
	//on = ecfalse;

#if 0
	if(sprite)
	{
		delete [] sprite;
		sprite = NULL;
	}

	if(csprite)
	{
		delete [] csprite;
		csprite = NULL;
	}
#endif
}

void DefB(int type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  ecbool hugterr,
		  const char* sprel,
		  int nframes,
		  const char* csprel,
		  int cnframes,
		  int foundation,
		  int reqdeposit,
		  int maxhp,
		  int visrange,
		  int opwage,
		  unsigned char flags,
		  int prop,
		  int wprop,
		  int srate)
{
	BlType* t = &g_bltype[type];

	t->free();

	t->width.x = size.x;
	t->width.y = size.y;
	sprintf(t->name, name);

	QueueTex(&t->icontex, iconrel, ectrue, ecfalse);

	//QueueModel(&t->model, sprel, scale, translate);
	//QueueModel(&t->cmodel, csprel, cscale, ctranslate);

#if 0
	/*
	TODO
	Neater way to do this by adding up a string together
	from pieces.
	*/
	if(hugterr)
	{
		t->sprite = new unsigned int [INCLINES];
		t->csprite = new unsigned int [INCLINES];

		for(int i=0; i<INCLINES; i++)
		{
			char specific[SFH_MAX_PATH+1];
			sprintf(specific, "%s_inc%s", sprel, INCLINENAME[i]);
			QueueSprite(specific, &t->sprite[i], ectrue, ectrue);
		}
		
		for(int i=0; i<INCLINES; i++)
		{
			char specific[SFH_MAX_PATH+1];
			sprintf(specific, "%s_inc%s", csprel, INCLINENAME[i]);
			QueueSprite(specific, &t->csprite[i], ectrue, ectrue);
		}
	}
	else
	{
		t->sprite = new unsigned int [nframes];
		t->csprite = new unsigned int [cnframes];

		for(int f=0; f<nframes; f++)
		{
			char specific[SFH_MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", sprel, f);
			QueueSprite(specific, &t->sprite[f], ectrue, ectrue);
		}

		for(int f=0; f<cnframes; f++)
		{
			char specific[SFH_MAX_PATH+1];
			sprintf(specific, "%s_fr%03d", csprel, f);
			QueueSprite(specific, &t->csprite[f], ectrue, ectrue);
		}
	}
#elif 0
	if(!LoadSpriteList(sprel, &t->splist, ectrue, ectrue, ectrue))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}

	if(!LoadSpriteList(csprel, &t->csplist, ectrue, ectrue, ectrue))
	{
		char m[128];
		sprintf(m, "Failed to load sprite list %s", sprel);
		ErrMess("Error", m);
	}
#else
	//hugterr=ecfalse;
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		ectrue, hugterr, ecfalse, ecfalse,
		nframes, 1, 
		ectrue, ectrue, ectrue, ectrue,
		1);
	QueueRend(csprel, RENDER_UNSPEC,
		&t->csplist,
		ectrue, hugterr, ecfalse, ecfalse,
		cnframes, 1, 
		ectrue, ectrue, ectrue, ectrue,
		1);
#endif

	t->foundation = foundation;
	t->hugterr = hugterr;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
	t->visrange = visrange;

	Zero(t->price);

	t->opwage = opwage;
	t->flags = flags;
	t->prop = prop;
	t->wprop = wprop;
	t->stockingrate = srate;

	t->nframes = nframes;
}


void DefB(int type,
		  const char* name,
		  const char* iconrel,
		  Vec2i size,
		  Vec2i cmalign,
		  Vec2i cmalignoff,
		  ecbool hugterr,
		  const char* sprel,
		  int nframes,
		  const char* csprel,
		  int cnframes,
		  int foundation,
		  int reqdeposit,
		  int maxhp,
		  int visrange,
		  int opwage,
		  unsigned char flags,
		  int prop,
		  int wprop,
		  int srate)
{
	BlType* t = &g_bltype[type];

	t->free();

	t->width.x = size.x;
	t->width.y = size.y;
	sprintf(t->name, name);
	t->cmalign = cmalign;
	t->cmalignoff = cmalignoff;

	QueueTex(&t->icontex, iconrel, ectrue, ecfalse);

//	hugterr=ecfalse;
	QueueRend(sprel, RENDER_UNSPEC,
		&t->splist,
		ectrue, hugterr, ecfalse, ecfalse,
		nframes, 1, 
		ectrue, ectrue, ectrue, ectrue,
		1);
	QueueRend(csprel, RENDER_UNSPEC,
		&t->csplist,
		ectrue, hugterr, ecfalse, ecfalse,
		cnframes, 1, 
		ectrue, ectrue, ectrue, ectrue,
		1);

	t->foundation = foundation;
	t->hugterr = hugterr;

	Zero(t->input);
	Zero(t->output);
	Zero(t->conmat);

	t->reqdeposit = reqdeposit;

	for(int i=0; i<BL_SOUNDS; i++)
		t->sound[i] = -1;

	t->maxhp = maxhp;
	t->manuf.clear();
	t->visrange = visrange;

	Zero(t->price);

	t->opwage = opwage;
	t->flags = flags;
	t->prop = prop;
	t->wprop = wprop;
	t->stockingrate = srate;
}

void BMan(int type, unsigned char mvtype)
{
	BlType* t = &g_bltype[type];
	t->manuf.push_back(mvtype);
}

void BSon(int type, int stype, const char* relative)
{
	BlType* t = &g_bltype[type];
	LoadSound(relative, &t->sound[stype]);
}

void BDes(int type, const char* desc)
{
	BlType* t = &g_bltype[type];
	t->desc = desc;
}

void BMat(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->conmat[res] = amt;
}

void BIn(int type, int res, int amt)
{
	BlType* t = &g_bltype[type];
	t->input[res] = amt;
}

//defl = default price
void BOut(int type, int res, int amt, int defl)
{
	BlType* t = &g_bltype[type];
	t->output[res] = amt;
	t->price[res] = defl;
}

void BEmit(int type, int emitterindex, int ptype, Vec3i offset)
{
	BlType* t = &g_bltype[type];
	EmitterPlace* e = &t->emitterpl[emitterindex];
	*e = EmitterPlace(ptype, Vec3f(offset.x,offset.y,offset.z));	//TODO use Vec3i everywhere
}
