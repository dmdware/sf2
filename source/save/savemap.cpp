











#include "../render/heightmap.h"
#include "../texture.h"
#include "../utils.h"
#include "../math/vec3f.h"
#include "../window.h"
#include "../math/camera.h"
#include "../sim/unit.h"
#include "../sim/mvtype.h"
#include "../math/hmapmath.h"
#include "../render/foliage.h"
#include "../sim/unit.h"
#include "../render/foliage.h"
#include "savemap.h"
#include "../sim/building.h"
#include "../sim/deposit.h"
#include "../render/water.h"
#include "../sim/player.h"
#include "../sim/selection.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../path/pathnode.h"
#include "../debug.h"
#include "../sim/simflow.h"
#include "../sim/transport.h"
#include "../render/particle.h"
#include "../render/transaction.h"
#include "../path/fillbodies.h"
#include "../path/tilepath.h"
#include "../render/drawsort.h"
#include "../sim/map.h"
#include "../algo/checksum.h"
#include "../net/net.h"
#include "savetrigger.h"
#include "savehint.h"
#include "../trigger/hint.h"
#include "../trigger/trigger.h"
#include "../render/fogofwar.h"
#include "../sim/border.h"
#include "../render/graph.h"
#include "../gui/layouts/chattext.h"

float ConvertHeight(unsigned char brightness)
{
#if 0
	// Apply exponential scale to height data.
	float y = (float)brightness*IN_Y_SCALE/255.0f - IN_Y_SCALE/2.0f;
	y = y / fabs(y) * pow(fabs(y), IN_Y_POWER) * IN_Y_AFTERPOW;

	if(y <= WATER_LEVEL)
		y -= TILE_SIZE;
	else if(y > WATER_LEVEL && y < WATER_LEVEL + TILE_SIZE/100)
		y += TILE_SIZE/100;

	return y;
#else
	return brightness / 25 * TILE_SIZE / 4;
#endif
}

void PlaceUnits()
{
	MvType* mvt = &g_mvtype[MV_LABOURER];
	PathJob pj;
	pj.airborne = mvt->airborne;
	pj.landborne = mvt->landborne;
	pj.seaborne = mvt->seaborne;
	pj.roaded = mvt->roaded;
	pj.targb = 0;
	pj.targu = 0;
	pj.thisu = 0;
	pj.mvtype = MV_LABOURER;

	for(int li=0; li<6*PLAYERS; li++)
	{
		int ntries = 0;

		for(; ntries < 100; ntries++)
		{
			Vec2i cmpos(rand()%g_mapsz.x*TILE_SIZE, rand()%g_mapsz.y*TILE_SIZE);
			Vec2i tpos = cmpos / TILE_SIZE;
			Vec2i npos = cmpos / PATHNODE_SIZE;

			pj.cmstartx = cmpos.x;
			pj.cmstarty = cmpos.y;

			if(Standable2(&pj, cmpos.x, cmpos.y))
			{
				PlaceUnit(MV_LABOURER, cmpos, -1, NULL);
				//Log("placed "<<li<<" at"<<tpos.x<<","<<tpos.y);
				break;
			}
			//else
			//Log("not placed at"<<tpos.x<<","<<tpos.y);
		}
	}
}


void FreeMap()
{
	for(unsigned char gi=0; gi<GRAPHS; ++gi)
	{
		g_graph[gi].cycles = 0;
		g_graph[gi].points.clear();
	}
	
	for(unsigned char gi=0; gi<PLAYERS; ++gi)
	{
		g_protecg[gi].cycles = 0;
		g_protecg[gi].points.clear();
	}

	FreeScript();
	FreeBls();	//bl's must be freed before mv, because of Bl::destroy because of how it clears ->worker and ->occupier lists
	FreeUnits();
	FreeFol();
	FreeDeposits();
	g_hmap.destroy();
	FreeGrid();
	Py* py = &g_py[g_localP];
	g_sel.clear();
	FreeParts();
	FreeTransx();
	//FreeGraphs();
	g_drawlist.clear();
	for(int i=0; i<PLAYERS; i++)
		g_protecg[i].points.clear();
	
	g_lasthint.message = "";
	g_lasthint.graphic = "gui\\transp.png";
}

void SaveHmap(FILE *fp)
{
	fwrite(&g_mapsz.x, sizeof(unsigned char), 1, fp);
	fwrite(&g_mapsz.y, sizeof(unsigned char), 1, fp);

	fwrite(g_hmap.hpt, sizeof(unsigned char), (g_mapsz.x+1)*(g_mapsz.y+1), fp);
	//fwrite(g_hmap.own, sizeof(int), g_mapsz.x*g_mapsz.y, fp);
}

void ReadHmap(FILE *fp, ecbool live, ecbool downafter)
{
	unsigned char widthx=0, widthy=0;

	fread(&widthx, sizeof(unsigned char), 1, fp);
	fread(&widthy, sizeof(unsigned char), 1, fp);

	//alloc hmap TO DO
	if(!downafter)
		g_hmap.alloc(widthx, widthy);

	for(int y=0; y<=widthy; y++)
		for(int x=0; x<=widthx; x++)
		{
			unsigned char h;
			fread(&h, sizeof(unsigned char), 1, fp);
			g_hmap.setheight(x, y, h);
		}
    
    g_hmap.lowereven();
	g_hmap.remesh();

	if(!downafter)
	{
		AllocGrid(widthx, widthy);

		//AllocPathGrid((widthx+1)*TILE_SIZE-1, (widthy+1)*TILE_SIZE-1);
		AllocPathGrid((g_mapsz.x)*TILE_SIZE, (g_mapsz.y)*TILE_SIZE);
		//CalcMapView();
	}

	if(downafter || live)
	{
		for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
		{
			for(unsigned char tx=0; tx<g_mapsz.x; ++tx)
			{
				for(unsigned char ty=0; ty<g_mapsz.y; ++ty)
				{
					CdTile* ctile = GetCd(ctype, tx, ty, ecfalse);

					if(ctile->on && ctile->depth)
						UpDraw(ctile, ctype, tx, ty);
				}
			}
		}
	}
}

void SaveFol(FILE *fp)
{
	for(int i=0; i<FOLIAGES; i++)
	{
#if 0
		ecbool on;
		unsigned char type;
		Vec3f pos;
		float yaw;
#endif

		Foliage *f = &g_fl[i];

		fwrite(&f->on, sizeof(ecbool), 1, fp);

		if(!f->on)
			continue;

		fwrite(&f->type, sizeof(unsigned char), 1, fp);
		fwrite(&f->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&f->drawpos, sizeof(Vec2f), 1, fp);
		fwrite(&f->yaw, sizeof(float), 1, fp);
	}
}

void ReadFol(FILE *fp, ecbool live)
{
	Foliage df;

	for(int i=0; i<FOLIAGES; i++)
	{
		Foliage *f = &g_fl[i];

		if(live)
			f = &df;

		fread(&f->on, sizeof(ecbool), 1, fp);

		if(!f->on)
			continue;

		fread(&f->type, sizeof(unsigned char), 1, fp);
		fread(&f->cmpos, sizeof(Vec2i), 1, fp);
		fread(&f->drawpos, sizeof(Vec2f), 1, fp);
		fread(&f->yaw, sizeof(float), 1, fp);

#if 1
		g_drawlist.push_back(Dl());
		Dl* d = &*g_drawlist.rbegin();
		d->dtype = DEPTH_FOL;
		d->index = i;
		f->depth = d;
		UpDraw(f);
#endif
	}
}

void SaveDeps(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
#if 0
		ecbool on;
		ecbool occupied;
		int restype;
		int amount;
		Vec2i tpos;
		Vec3f drawpos;
#endif

		Deposit *d = &g_deposit[i];

		fwrite(&d->on, sizeof(ecbool), 1, fp);

		if(!d->on)
			continue;

		fwrite(&d->occupied, sizeof(ecbool), 1, fp);
		fwrite(&d->restype, sizeof(int), 1, fp);
		fwrite(&d->amount, sizeof(int), 1, fp);
		fwrite(&d->tpos, sizeof(Vec2i), 1, fp);
		fwrite(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void ReadDeps(FILE *fp)
{
	for(int i=0; i<DEPOSITS; i++)
	{
		Deposit *d = &g_deposit[i];

		fread(&d->on, sizeof(ecbool), 1, fp);

		if(!d->on)
			continue;

		fread(&d->occupied, sizeof(ecbool), 1, fp);
		fread(&d->restype, sizeof(int), 1, fp);
		fread(&d->amount, sizeof(int), 1, fp);
		fread(&d->tpos, sizeof(Vec2i), 1, fp);
		fread(&d->drawpos, sizeof(Vec3f), 1, fp);
	}
}

void SaveUnits(FILE *fp)
{
	for(int i=0; i<MOVERS; i++)
	{
#if 0
		ecbool on;
		int type;
		int stateowner;
		int corpowner;
		int unitowner;

		/*
		The f (floating-point) position vectory is used for drawing.
		*/
		Vec3f fpos;

		/*
		The real position is stored in integers.
		*/
		Vec2i cmpos;
		Vec3f facing;
		Vec2f rotation;

		std::list<Vec2i> path;
		Vec2i goal;
#endif

		Mv *u = &g_mv[i];

		fwrite(&mv->on, sizeof(ecbool), 1, fp);

		if(!mv->on)
			continue;

		fwrite(&mv->type, sizeof(int), 1, fp);
#if 0
		fwrite(&mv->stateowner, sizeof(int), 1, fp);
		fwrite(&mv->corpowner, sizeof(int), 1, fp);
		fwrite(&mv->unitowner, sizeof(int), 1, fp);
#else
		fwrite(&mv->owner, sizeof(int), 1, fp);
#endif

		fwrite(&mv->drawpos, sizeof(Vec3f), 1, fp);
		fwrite(&mv->cmpos, sizeof(Vec2i), 1, fp);
		fwrite(&mv->rotation, sizeof(Vec3f), 1, fp);

		int pathsz = mv->path.size();

		fwrite(&pathsz, sizeof(int), 1, fp);

		for(std::list<Vec2i>::iterator pathiter = mv->path.begin(); pathiter != mv->path.end(); pathiter++)
			fwrite(&*pathiter, sizeof(Vec2i), 1, fp);

		fwrite(&mv->goal, sizeof(Vec2i), 1, fp);

#if 0
		int target;
		int target2;
		ecbool targetu;
		ecbool underorder;
		int fuelstation;
		int belongings[RESOURCES];
		int hp;
		ecbool passive;
		Vec2i prevpos;
		int taskframe;
		ecbool pathblocked;
		int jobframes;
		int supplier;
		int reqamt;
		int targtype;
		int home;
		int car;
		//std::vector<TransportJob> bids;

		float frame[2];

		Vec2i subgoal;
#endif

		fwrite(&mv->target, sizeof(int), 1, fp);
		fwrite(&mv->target2, sizeof(int), 1, fp);
		fwrite(&mv->targetu, sizeof(ecbool), 1, fp);
		fwrite(&mv->underorder, sizeof(ecbool), 1, fp);
		fwrite(&mv->fuelstation, sizeof(int), 1, fp);
		fwrite(mv->belongings, sizeof(int), RESOURCES, fp);
		fwrite(&mv->hp, sizeof(int), 1, fp);
		fwrite(&mv->passive, sizeof(ecbool), 1, fp);
		fwrite(&mv->prevpos, sizeof(Vec2i), 1, fp);
		fwrite(&mv->taskframe, sizeof(int), 1, fp);
		fwrite(&mv->pathblocked, sizeof(ecbool), 1, fp);
		fwrite(&mv->jobframes, sizeof(int), 1, fp);
		fwrite(&mv->supplier, sizeof(int), 1, fp);
		fwrite(&mv->reqamt, sizeof(int), 1, fp);
		fwrite(&mv->targtype, sizeof(int), 1, fp);
		fwrite(&mv->home, sizeof(int), 1, fp);
		fwrite(&mv->car, sizeof(int), 1, fp);
		fwrite(&mv->frame, sizeof(float), 2, fp);
		fwrite(&mv->subgoal, sizeof(Vec2i), 1, fp);

#if 0
	unsigned char mode;
	int pathdelay;
	unsigned __int64 lastpath;

	ecbool threadwait;

	// used for debugging - don't save to file
	ecbool collided;

	unsigned char cdtype;	//conduit type for mode (going to construction)
	int driver;
	//short framesleft;
	int cyframes;	//cycle frames (unit cycle of consumption and work)
	int opwage;	//transport driver wage
	//std::list<TransportJob> bids;	//for trucks
#endif

		fwrite(&mv->mode, sizeof(unsigned char), 1, fp);
		fwrite(&mv->pathdelay, sizeof(int), 1, fp);
		fwrite(&mv->lastpath, sizeof(unsigned __int64), 1, fp);
		fwrite(&mv->threadwait, sizeof(ecbool), 1, fp);
		fwrite(&mv->collided, sizeof(ecbool), 1, fp);
		fwrite(&mv->cdtype, sizeof(signed char), 1, fp);
		fwrite(&mv->driver, sizeof(int), 1, fp);
		fwrite(&mv->cyframes, sizeof(int), 1, fp);
		fwrite(&mv->opwage, sizeof(int), 1, fp);

#if 0
	int cargoamt;
	int cargotype;
	int cargoreq;	//req amt
#endif

		fwrite(&mv->cargoamt, sizeof(int), 1, fp);
		fwrite(&mv->cargotype, sizeof(int), 1, fp);
		fwrite(&mv->cargoreq, sizeof(int), 1, fp);

		int ntpath = mv->tpath.size();
		fwrite(&ntpath, sizeof(int), 1, fp);
		//mv->tpath.clear();
		for(std::list<Vec2s>::iterator ti=mv->tpath.begin(); ti!=mv->tpath.end(); ti++)
			fwrite(&*ti, sizeof(Vec2s), 1, fp);

		fwrite(&mv->exputil, sizeof(int), 1, fp);

		fwrite(&mv->forsale, sizeof(ecbool), 1, fp);
		fwrite(&mv->price, sizeof(int), 1, fp);
		
		fwrite(&mv->incomerate, sizeof(int), 1, fp);

		int nhists = mv->cyclehist.size();
		
		fwrite(&nhists, sizeof(int), 1, fp);
		
		for(std::list<TrCycleHist>::iterator ch=mv->cyclehist.begin(); ch!=mv->cyclehist.end(); ch++)
		{
			fwrite(&*ch, sizeof(TrCycleHist), 1, fp);
		}

		fwrite(&mv->mazeavoid, sizeof(ecbool), 1, fp);
	}
}

void ReadUnits(FILE *fp, ecbool live)
{
	Mv du;

	for(int i=0; i<MOVERS; i++)
	{
		Mv *u = &g_mv[i];

		if(live)
			u = &du;

		fread(&mv->on, sizeof(ecbool), 1, fp);

		if(!mv->on)
			continue;

		fread(&mv->type, sizeof(int), 1, fp);

#ifdef TRANSPORT_DEBUG
		if(mv->type == MV_LABOURER || i==10 || i == 11)
		//if(i!=0)
			mv->on = ecfalse;
#endif

#if 0
		fread(&mv->stateowner, sizeof(int), 1, fp);
		fread(&mv->corpowner, sizeof(int), 1, fp);
		fread(&mv->unitowner, sizeof(int), 1, fp);
#else
		fread(&mv->owner, sizeof(int), 1, fp);
#endif

		fread(&mv->drawpos, sizeof(Vec3f), 1, fp);
		fread(&mv->cmpos, sizeof(Vec2i), 1, fp);
		fread(&mv->rotation, sizeof(Vec3f), 1, fp);

#ifdef RANDOM8DEBUG
		//mv->drawpos = Vec3f(mv->cmpos.x, g_hmap.accheight(mv->cmpos.x, mv->cmpos.y), mv->cmpos.y);
#endif

		int pathsz = 0;

		fread(&pathsz, sizeof(int), 1, fp);
		mv->path.clear();

		for(int pathindex=0; pathindex<pathsz; pathindex++)
		{
			Vec2i waypoint;
			fread(&waypoint, sizeof(Vec2i), 1, fp);
			mv->path.push_back(waypoint);
		}

		fread(&mv->goal, sizeof(Vec2i), 1, fp);

		fread(&mv->target, sizeof(int), 1, fp);
		fread(&mv->target2, sizeof(int), 1, fp);
		fread(&mv->targetu, sizeof(ecbool), 1, fp);
		fread(&mv->underorder, sizeof(ecbool), 1, fp);
		fread(&mv->fuelstation, sizeof(int), 1, fp);
		fread(mv->belongings, sizeof(int), RESOURCES, fp);
		fread(&mv->hp, sizeof(int), 1, fp);
		fread(&mv->passive, sizeof(ecbool), 1, fp);
		fread(&mv->prevpos, sizeof(Vec2i), 1, fp);
		fread(&mv->taskframe, sizeof(int), 1, fp);
		fread(&mv->pathblocked, sizeof(ecbool), 1, fp);
		fread(&mv->jobframes, sizeof(int), 1, fp);
		fread(&mv->supplier, sizeof(int), 1, fp);
		fread(&mv->reqamt, sizeof(int), 1, fp);
		fread(&mv->targtype, sizeof(int), 1, fp);
		fread(&mv->home, sizeof(int), 1, fp);
		fread(&mv->car, sizeof(int), 1, fp);
		fread(&mv->frame, sizeof(float), 2, fp);
		fread(&mv->subgoal, sizeof(Vec2i), 1, fp);

		fread(&mv->mode, sizeof(unsigned char), 1, fp);
		fread(&mv->pathdelay, sizeof(int), 1, fp);
		fread(&mv->lastpath, sizeof(unsigned __int64), 1, fp);
		fread(&mv->threadwait, sizeof(ecbool), 1, fp);
		fread(&mv->collided, sizeof(ecbool), 1, fp);
		fread(&mv->cdtype, sizeof(signed char), 1, fp);
		fread(&mv->driver, sizeof(int), 1, fp);
		fread(&mv->cyframes, sizeof(int), 1, fp);
		fread(&mv->opwage, sizeof(int), 1, fp);

		fread(&mv->cargoamt, sizeof(int), 1, fp);
		fread(&mv->cargotype, sizeof(int), 1, fp);
		fread(&mv->cargoreq, sizeof(int), 1, fp);

#if 1
		int ntpath = 0;
		fread(&ntpath, sizeof(int), 1, fp);
		mv->tpath.clear();
		for(int ti=0; ti<ntpath; ti++)
		{
			Vec2s tpos;
			fread(&tpos, sizeof(Vec2s), 1, fp);
			mv->tpath.push_back(tpos);
		}
#endif
		
		fread(&mv->exputil, sizeof(int), 1, fp);

		fread(&mv->forsale, sizeof(ecbool), 1, fp);
		fread(&mv->price, sizeof(int), 1, fp);

		fread(&mv->incomerate, sizeof(int), 1, fp);
		
		int nhists = 0;
		
		fread(&nhists, sizeof(int), 1, fp);
		
		for(int hi=0; hi<nhists; ++hi)
		{
			TrCycleHist ch;
			fread(&ch, sizeof(TrCycleHist), 1, fp);
			mv->cyclehist.push_back(ch);
		}
		
		fread(&mv->mazeavoid, sizeof(ecbool), 1, fp);

		g_drawlist.push_back(Dl());
		Dl* d = &*g_drawlist.rbegin();
		mv->depth = d;
		d->dtype = DEPTH_U;
		d->index = i;
		UpDraw(u);

		//mv->fillcollider();
	}

	du.home = -1;
	du.mode = UMODE_NONE;
	du.driver = -1;
}

void SaveCapSup(CapSup* cs, FILE* fp)
{
#if 0
	unsigned char rtype;
	int amt;
	int src;
	int dst;
#endif

	fwrite(&cs->rtype, sizeof(unsigned char), 1, fp);
	fwrite(&cs->amt, sizeof(int), 1, fp);
	fwrite(&cs->src, sizeof(int), 1, fp);
	fwrite(&cs->dst, sizeof(int), 1, fp);
}

void ReadCapSup(CapSup* cs, FILE* fp)
{
	fread(&cs->rtype, sizeof(unsigned char), 1, fp);
	fread(&cs->amt, sizeof(int), 1, fp);
	fread(&cs->src, sizeof(int), 1, fp);
	fread(&cs->dst, sizeof(int), 1, fp);
}

void SaveManufJob(ManufJob* mj, FILE* fp)
{
#if 0
	int mvtype;
	int owner;
#endif

	fwrite(&mj->mvtype, sizeof(int), 1, fp);
	fwrite(&mj->owner, sizeof(int), 1, fp);
}

void ReadManufJob(ManufJob* mj, FILE* fp)
{
	fread(&mj->mvtype, sizeof(int), 1, fp);
	fread(&mj->owner, sizeof(int), 1, fp);
}

void SaveBls(FILE *fp)
{
	for(int i=0; i<BUILDINGS; i++)
	{
#if 0
	ecbool on;
	int type;
	int owner;

	Vec2i tpos;	//position in tiles
	Vec3f drawpos;	//drawing position in centimeters

	ecbool finished;

	short pownetw;
	short crpipenetw;
	std::list<short> roadnetw;
#endif

		Bl *b = &g_bl[i];

		fwrite(&b->on, sizeof(ecbool), 1, fp);

		if(!b->on)
			continue;

		fwrite(&b->type, sizeof(int), 1, fp);
#if 0
		fwrite(&b->stateowner, sizeof(int), 1, fp);
		fwrite(&b->corpowner, sizeof(int), 1, fp);
		fwrite(&b->unitowner, sizeof(int), 1, fp);
#else
		fwrite(&b->owner, sizeof(int), 1, fp);
#endif

		fwrite(&b->tpos, sizeof(Vec2i), 1, fp);
		fwrite(&b->drawpos, sizeof(Vec3f), 1, fp);

		fwrite(&b->finished, sizeof(ecbool), 1, fp);

#if 1
		fwrite(&b->pownetw, sizeof(short), 1, fp);
		fwrite(&b->crpipenetw, sizeof(short), 1, fp);
		int nroadnetw = b->roadnetw.size();
		fwrite(&nroadnetw, sizeof(short), 1, fp);
		for(std::list<short>::iterator rnetit = b->roadnetw.begin(); rnetit != b->roadnetw.end(); rnetit++)
			fwrite(&*rnetit, sizeof(short), 1, fp);
#else
		fwrite(b->netw, sizeof(short), CD_TYPES, fp);
		for(unsigned char ci=0; ci<CD_TYPES; ++ci)
		{
			int nl = b->netwlist[ci].size();
			fwrite(&nl, sizeof(int), 1, fp);
			for(std::list<Widget*>::iterator nit=b->netwlist[ci].begin(); nit!=b->netwlist[ci].end(); nit++)
				fwrite(&*nit, sizeof(short), 1, fp);
		}
#endif

#if 0
	int stocked[RESOURCES];
	int inuse[RESOURCES];

	EmCntr emitterco[MAX_B_EMITTERS];

	VertexArray drawva;

	int conmat[RESOURCES];
	ecbool inoperation;

	int price[RESOURCES];	//price of produced goods
	int propprice;	//price of this property

	std::list<int> occupier;
	std::list<int> worker;
	int conwage;
	int opwage;
	int cydelay;	//the frame delay between production cycles, when production target is renewed
	short prodlevel;	//production target level of max RATIO_DENOM
	short cymet;	//used to keep track of what was produced this cycle, out of max of prodlevel
	unsigned int lastcy;	//last simframe of last production cycle
	std::list<CapSup> capsup;	//capacity suppliers

	int manufprc[MV_TYPES];
	std::list<ManufJob> manufjob;
	short transporter[RESOURCES];
#endif

		fwrite(b->stocked, sizeof(int), RESOURCES, fp);
		fwrite(b->inuse, sizeof(int), RESOURCES, fp);

		fwrite(b->conmat, sizeof(int), RESOURCES, fp);
		fwrite(&b->inoperation, sizeof(ecbool), 1, fp);
		fwrite(b->price, sizeof(int), RESOURCES, fp);
		fwrite(&b->propprice, sizeof(int), 1, fp);

		fwrite(&b->forsale, sizeof(ecbool), 1, fp);
		fwrite(&b->demolish, sizeof(ecbool), 1, fp);

		unsigned char noc = b->occupier.size();
		fwrite(&noc, sizeof(unsigned char), 1, fp);
		for(std::list<int>::iterator ocit=b->occupier.begin(); ocit!=b->occupier.end(); ocit++)
			fwrite(&*ocit, sizeof(int), 1, fp);

		unsigned char nwk = b->worker.size();
		fwrite(&nwk, sizeof(unsigned char), 1, fp);
		for(std::list<int>::iterator wkit=b->worker.begin(); wkit!=b->worker.end(); wkit++)
			fwrite(&*wkit, sizeof(int), 1, fp);

		fwrite(&b->conwage, sizeof(int), 1, fp);
		fwrite(&b->opwage, sizeof(int), 1, fp);
		fwrite(&b->cydelay, sizeof(int), 1, fp);
		fwrite(&b->prodlevel, sizeof(short), 1, fp);
		fwrite(&b->cymet, sizeof(short), 1, fp);
		fwrite(&b->lastcy, sizeof(unsigned __int64), 1, fp);

		int ncs = b->capsup.size();
		fwrite(&ncs, sizeof(int), 1, fp);
		for(std::list<CapSup>::iterator ncit=b->capsup.begin(); ncit!=b->capsup.end(); ncit++)
			SaveCapSup(&*ncit, fp);

		fwrite(b->manufprc, sizeof(int), MV_TYPES, fp);

		int nmj = b->manufjob.size();
		fwrite(&nmj, sizeof(int), 1, fp);
		for(std::list<ManufJob>::iterator mjit=b->manufjob.begin(); mjit!=b->manufjob.end(); mjit++)
			SaveManufJob(&*mjit, fp);

		fwrite(b->transporter, sizeof(short), RESOURCES, fp);

		fwrite(b->varcost, sizeof(int), RESOURCES, fp);
		fwrite(&b->fixcost, sizeof(int), 1, fp);
		
		fwrite(&b->demolition, sizeof(ecbool), 1, fp);

#if 0
		int hp;
#endif

		fwrite(&b->hp, sizeof(int), 1, fp);
		
		int nhists = b->cyclehist.size();
		
		fwrite(&nhists, sizeof(int), 1, fp);
		
		for(std::list<CycleHist>::iterator ch=b->cyclehist.begin(); ch!=b->cyclehist.end(); ch++)
		{
			fwrite(&*ch, sizeof(CycleHist), 1, fp);
		}
	}
}

void ReadBls(FILE *fp, ecbool live)
{
	Bl db;

	for(int i=0; i<BUILDINGS; i++)
	{

		//Log("\t read bl"<<i);
		//

		Bl *b = &g_bl[i];

		if(live)
			b = &db;

		fread(&b->on, sizeof(ecbool), 1, fp);

		if(!b->on)
			continue;

		//Log("\t\t on"<<i);
		//
		
		//no need to sort, read in order
		g_onbl.push_back((unsigned short)i);

		fread(&b->type, sizeof(int), 1, fp);
#if 0
		fread(&b->stateowner, sizeof(int), 1, fp);
		fread(&b->corpowner, sizeof(int), 1, fp);
		fread(&b->unitowner, sizeof(int), 1, fp);
#else
		fread(&b->owner, sizeof(int), 1, fp);
#endif

		fread(&b->tpos, sizeof(Vec2i), 1, fp);
		fread(&b->drawpos, sizeof(Vec3f), 1, fp);

#if 0
		BlType* t = &g_bltype[b->type];
		Vec2i tmin;
		Vec2i tmax;
		tmin.x = b->tpos.x - t->width.x/2;
		tmin.y = b->tpos.y - t->width.y/2;
		tmax.x = tmin.x + t->width.x;
		tmax.y = tmin.y + t->width.y;
		b->drawpos = Vec3f(b->tpos.x*TILE_SIZE, Lowest(tmin.x, tmin.y, tmax.x, tmax.y), b->tpos.y*TILE_SIZE);
		if(t->foundation == FD_SEA)
			b->drawpos.y = WATER_LEVEL;
		if(t->width.x % 2 == 1)
			b->drawpos.x += TILE_SIZE/2;
		if(t->width.y % 2 == 1)
			b->drawpos.y += TILE_SIZE/2;
#endif

		fread(&b->finished, sizeof(ecbool), 1, fp);

		//Log("\t\t netws..."<<i);
		//

#if 1
		fread(&b->pownetw, sizeof(short), 1, fp);
		fread(&b->crpipenetw, sizeof(short), 1, fp);
		short nroadnetw = -1;
		fread(&nroadnetw, sizeof(short), 1, fp);
		for(int rni=0; rni<nroadnetw; rni++)
		{
			short roadnetw = -1;
			fread(&roadnetw, sizeof(short), 1, fp);
			b->roadnetw.push_back(roadnetw);
		}
#else
		fread(b->netw, sizeof(short), CD_TYPES, fp);
		for(unsigned char ci=0; ci<CD_TYPES; ++ci)
		{
			int nl = 0;
			fread(&nl, sizeof(int), 1, fp);
			for(int nli=0; nli<nl; ++nli)
			{
				short ni;
				fread(&ni, sizeof(short), 1, fp);
				b->netwlist[ci].push_back(ni);
			}
		}
#endif

		fread(b->stocked, sizeof(int), RESOURCES, fp);
		fread(b->inuse, sizeof(int), RESOURCES, fp);

		fread(b->conmat, sizeof(int), RESOURCES, fp);
		fread(&b->inoperation, sizeof(ecbool), 1, fp);
		fread(b->price, sizeof(int), RESOURCES, fp);
		fread(&b->propprice, sizeof(int), 1, fp);
		
		fread(&b->forsale, sizeof(ecbool), 1, fp);
		fread(&b->demolish, sizeof(ecbool), 1, fp);

		//Log("\t\t ocs..."<<i);
		//

		unsigned char noc = b->occupier.size();
		fread(&noc, sizeof(unsigned char), 1, fp);
		for(int oci=0; oci<noc; oci++)
		{
			int oc;
			fread(&oc, sizeof(int), 1, fp);
			b->occupier.push_back(oc);
		}

		unsigned char nwk = b->worker.size();
		fread(&nwk, sizeof(unsigned char), 1, fp);
		for(int wki=0; wki<nwk; wki++)
		{
			int wk;
			fread(&wk, sizeof(int), 1, fp);
			b->worker.push_back(wk);
		}

		fread(&b->conwage, sizeof(int), 1, fp);
		fread(&b->opwage, sizeof(int), 1, fp);
		fread(&b->cydelay, sizeof(int), 1, fp);
		fread(&b->prodlevel, sizeof(short), 1, fp);
		fread(&b->cymet, sizeof(short), 1, fp);
		fread(&b->lastcy, sizeof(unsigned __int64), 1, fp);

		int ncs = 0;
		fread(&ncs, sizeof(int), 1, fp);
		for(int ci=0; ci<ncs; ci++)
		{
			CapSup cs;
			ReadCapSup(&cs, fp);
			b->capsup.push_back(cs);
		}

		fread(b->manufprc, sizeof(int), MV_TYPES, fp);

		int nmj = 0;
		fread(&nmj, sizeof(int), 1, fp);
		for(int mji=0; mji<nmj; mji++)
		{
			ManufJob mj;
			ReadManufJob(&mj, fp);
			b->manufjob.push_back(mj);
		}

		fread(b->transporter, sizeof(short), RESOURCES, fp);
		
		fread(b->varcost, sizeof(int), RESOURCES, fp);
		fread(&b->fixcost, sizeof(int), 1, fp);
		
		fread(&b->demolition, sizeof(ecbool), 1, fp);

		fread(&b->hp, sizeof(int), 1, fp);

		int nhists = 0;
		
		fread(&nhists, sizeof(int), 1, fp);
		
		for(int hi=0; hi<nhists; ++hi)
		{
			CycleHist ch;
			fread(&ch, sizeof(CycleHist), 1, fp);
			b->cyclehist.push_back(ch);
		}
		
		g_drawlist.push_back(Dl());
		Dl* d = &*g_drawlist.rbegin();
		b->depth = d;
		d->dtype = DEPTH_BL;
		d->index = i;
		UpDraw(b);

		//b->fillcollider();
	}

	db.capsup.clear();
	db.worker.clear();
	db.occupier.clear();
}

void SaveView(FILE *fp)
{
	Py* py = &g_py[g_localP];
	//fwrite(&g_cam, sizeof(Camera), 1, fp);

	Vec2i scroll = g_scroll + Vec2i(g_width,g_height)/2;

	fwrite(&scroll, sizeof(Vec2i), 1, fp);
	fwrite(&g_zoom, sizeof(float), 1, fp);
}

void ReadView(FILE *fp)
{
	Py* py = &g_py[g_localP];
	//fread(&g_cam, sizeof(Camera), 1, fp);
	fread(&g_scroll, sizeof(Vec2i), 1, fp);
	fread(&g_zoom, sizeof(float), 1, fp);

	g_scroll = g_scroll - Vec2i(g_width,g_height)/2;
}

void ReadGraph(FILE* fp, Graph* g)
{
	fread(&g->cycles, sizeof(unsigned int), 1, fp);
	fread(&g->startframe, sizeof(unsigned __int64), 1, fp);

	unsigned int c;
	fread(&c, sizeof(unsigned int), 1, fp);

	for(unsigned int ci=0; ci<c; ++ci)
	{
		float cf;
		fread(&cf, sizeof(float), 1, fp);
		g->points.push_back(cf);
	}
}

void SaveGraph(FILE* fp, Graph* g)
{
	fwrite(&g->cycles, sizeof(unsigned int), 1, fp);
	fwrite(&g->startframe, sizeof(unsigned __int64), 1, fp);

	unsigned int c = g->points.size();
	fwrite(&c, sizeof(unsigned int), 1, fp);

	for(std::list<float>::iterator cf=g->points.begin(); cf!=g->points.end(); ++cf)
		fwrite(&*cf, sizeof(float), 1, fp);
}

void ReadGraphs(FILE* fp)
{
	for(unsigned char gi=0; gi<GRAPHS; ++gi)
		ReadGraph(fp, &g_graph[gi]);
	for(unsigned char gi=0; gi<PLAYERS; ++gi)
		ReadGraph(fp, &g_protecg[gi]);
}

void SaveGraphs(FILE* fp)
{
	for(unsigned char gi=0; gi<GRAPHS; ++gi)
		SaveGraph(fp, &g_graph[gi]);
	for(unsigned char gi=0; gi<PLAYERS; ++gi)
		SaveGraph(fp, &g_protecg[gi]);
}

void SaveCd(FILE* fp)
{
	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int i=0; i<g_mapsz.x*g_mapsz.y; i++)
		{
#if 0
	ecbool on;
	unsigned char conntype;
	ecbool finished;
	unsigned char owner;
	int conmat[RESOURCES];
	short netw;	//network
	VertexArray drawva;
	//ecbool inaccessible;
	short transporter[RESOURCES];
	Vec3f drawpos;
	//int maxcost[RESOURCES];
	int conwage;
#endif

			CdTile* ctile = &ct->cdtiles[(int)ecfalse][i];

			fwrite(&ctile->on, sizeof(ecbool), 1, fp);

			if(!ctile->on)
				continue;

			fwrite(&ctile->conntype, sizeof(char), 1, fp);
			fwrite(&ctile->finished, sizeof(ecbool), 1, fp);
			fwrite(&ctile->owner, sizeof(unsigned char), 1, fp);
			fwrite(ctile->conmat, sizeof(int), RESOURCES, fp);
			fwrite(&ctile->netw, sizeof(short), 1, fp);
			fwrite(ctile->transporter, sizeof(short), RESOURCES, fp);
			fwrite(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fwrite(&ctile->conwage, sizeof(int), 1, fp);
			fwrite(&ctile->selling, sizeof(ecbool), 1, fp);
		}
	}
}

void ReadCd(FILE* fp, ecbool live)
{
	CdTile dctile;

	for(unsigned char ctype=0; ctype<CD_TYPES; ctype++)
	{
		CdType* ct = &g_cdtype[ctype];

		for(int i=0; i<g_mapsz.x*g_mapsz.y; i++)
		{
			CdTile* ctile = &ct->cdtiles[(int)ecfalse][i];

			if(live)
				ctile = &dctile;

			fread(&ctile->on, sizeof(ecbool), 1, fp);

			if(!ctile->on)
				continue;

			fread(&ctile->conntype, sizeof(char), 1, fp);
			fread(&ctile->finished, sizeof(ecbool), 1, fp);
			fread(&ctile->owner, sizeof(unsigned char), 1, fp);
			fread(ctile->conmat, sizeof(int), RESOURCES, fp);
			fread(&ctile->netw, sizeof(short), 1, fp);
			fread(ctile->transporter, sizeof(short), RESOURCES, fp);
			fread(&ctile->drawpos, sizeof(Vec3f), 1, fp);
			fread(&ctile->conwage, sizeof(int), 1, fp);
			fread(&ctile->selling, sizeof(ecbool), 1, fp);

			g_drawlist.push_back(Dl());
			Dl* d = &*g_drawlist.rbegin();
			d->dtype = DEPTH_CD;
			d->cdtype = ctype;
			d->plan = ecfalse;
			d->index = i;
			ctile->depth = d;
			UpDraw(ctile, d->cdtype, i % g_mapsz.x, i / g_mapsz.x);	//cse fix
		}
	}
}

void ReadPys(FILE* fp)
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

#if 0
	ecbool on;
	ecbool ai;

	int local[RESOURCES];	// used just for counting; cannot be used
	int global[RESOURCES];
	int resch[RESOURCES];	//resource changes/deltas
	int truckwage;	//truck driver wage per second
	int transpcost;	//transport cost per second

#endif

		fread(&py->on, sizeof(ecbool), 1, fp);

		if(!py->on)
			continue;

		fread(&py->ai, sizeof(ecbool), 1, fp);
		fread(py->local, sizeof(int), RESOURCES, fp);
		fread(py->global, sizeof(int), RESOURCES, fp);
		fread(py->resch, sizeof(int), RESOURCES, fp);
		fread(&py->truckwage, sizeof(int), 1, fp);
		fread(&py->transpcost, sizeof(int), 1, fp);
		//fread(&py->gnp, sizeof(unsigned int), 1, fp);
		fread(&py->gnp, sizeof(unsigned __int64), 1, fp);
		//fread(&py->util, sizeof(unsigned int), 1, fp);
		fread(&py->util, sizeof(unsigned __int64), 1, fp);

#if 0
	float color[4];
	RichText name;
#endif

		fread(py->color, sizeof(float), 4, fp);

		int slen = 0;
		fread(&slen, sizeof(int), 1, fp);
		char* name = new char[slen];
		fread(name, sizeof(char), slen, fp);
		py->name = RichText(name);
		delete [] name;
		
		fread(&py->parentst, sizeof(int), 1, fp);
		fread(&py->insttype, sizeof(signed char), 1, fp);
		fread(&py->instin, sizeof(signed char), 1, fp);
		fread(&py->protectionism, sizeof(ecbool), 1, fp);
		fread(&py->extariffratio, sizeof(int), 1, fp);
		fread(&py->imtariffratio, sizeof(int), 1, fp);

		fread(&py->lastthink, sizeof(unsigned __int64), 1, fp);
	}
}

void SavePys(FILE* fp)
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];

		fwrite(&py->on, sizeof(ecbool), 1, fp);

		if(!py->on)
			continue;

		fwrite(&py->ai, sizeof(ecbool), 1, fp);
		fwrite(py->local, sizeof(int), RESOURCES, fp);
		fwrite(py->global, sizeof(int), RESOURCES, fp);
		fwrite(py->resch, sizeof(int), RESOURCES, fp);
		fwrite(&py->truckwage, sizeof(int), 1, fp);
		fwrite(&py->transpcost, sizeof(int), 1, fp);
		fwrite(&py->gnp, sizeof(unsigned __int64), 1, fp);
		fwrite(&py->util, sizeof(unsigned __int64), 1, fp);
		fwrite(py->color, sizeof(float), 4, fp);

		std::string name = py->name.rawstr();
		int slen = name.length() + 1;
		fwrite(&slen, sizeof(int), 1, fp);
		fwrite(name.c_str(), sizeof(char), slen, fp);
		
		fwrite(&py->parentst, sizeof(int), 1, fp);
		fwrite(&py->insttype, sizeof(signed char), 1, fp);
		fwrite(&py->instin, sizeof(signed char), 1, fp);
		fwrite(&py->protectionism, sizeof(ecbool), 1, fp);
		fwrite(&py->extariffratio, sizeof(int), 1, fp);
		fwrite(&py->imtariffratio, sizeof(int), 1, fp);

		fwrite(&py->lastthink, sizeof(unsigned __int64), 1, fp);
	}
}

#if 0
void ReadGr(FILE* fp)
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

#if 0
		std::list<float> points;
		unsigned int startframe;
		unsigned int cycles;
#endif

		fread(&g->startframe, sizeof(unsigned int), 1, fp);
		fread(&g->cycles, sizeof(unsigned int), 1, fp);

		unsigned int np = 0;
		fread(&np, sizeof(unsigned int), 1, fp);

		for(int pi=0; pi<np; pi++)
		{
			float p;
			fread(&p, sizeof(float), 1, fp);
			g->points.push_back(p);
		}
	}
}

void SaveGr(FILE* fp)
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];

		fwrite(&g->startframe, sizeof(unsigned int), 1, fp);
		fwrite(&g->cycles, sizeof(unsigned int), 1, fp);

		unsigned int np = g->points.size();
		fwrite(&np, sizeof(unsigned int), 1, fp);

		for(std::list<Widget*>::iterator pit=g->points.begin(); pit!=g->points.end(); pit++)
		{
			fwrite(&*pit, sizeof(float), 1, fp);
		}
	}
}
#endif

void ReadJams(FILE* fp, ecbool live)
{
	//return;

	TileNode dtn;

	for(short x=0; x<g_mapsz.x; x++)
		for(short y=0; y<g_mapsz.y; y++)
		{
			int tin = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];

			if(live)
				tn = &dtn;

			fread(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

void SaveJams(FILE* fp)
{
	for(short x=0; x<g_mapsz.x; x++)
		for(short y=0; y<g_mapsz.y; y++)
		{
			int tin = x + y * g_mapsz.x;
			TileNode* tn = &g_tilenode[tin];
			fwrite(&tn->jams, sizeof(unsigned char), 1, fp);
		}
}

void ReadVis(FILE* fp, ecbool live)
{
	VisTile dvt;

	for(short x=0; x<g_mapsz.x; x++)
		for(short y=0; y<g_mapsz.y; y++)
		{
			VisTile* v = &g_vistile[ x + y * g_mapsz.x ];

			if(live)
				v = &dvt;

			fread(v->explored, sizeof(v->explored), 1, fp);
			fread(v->vis, sizeof(unsigned char), PLAYERS, fp);
		}
}

void SaveVis(FILE* fp)
{
	for(short x=0; x<g_mapsz.x; x++)
		for(short y=0; y<g_mapsz.y; y++)
		{
			VisTile* v = &g_vistile[ x + y * g_mapsz.x ];
			fwrite(v->explored, sizeof(v->explored), 1, fp);
			fwrite(v->vis, sizeof(unsigned char), PLAYERS, fp);
		}
}

void ReadBords(FILE* fp, ecbool live)
{
	if(live)
	{
		fseek(fp, sizeof(signed char) * g_mapsz.x * g_mapsz.y, SEEK_CUR);
		return;
	}

	fread(g_border, sizeof(signed char), g_mapsz.x * g_mapsz.y, fp);
}

void SaveBords(FILE* fp)
{
	fwrite(g_border, sizeof(signed char), g_mapsz.x * g_mapsz.y, fp);
}

//consistency check
void ConsistCh()
{
	for(int bi=0; bi<BUILDINGS; bi++)
	{
		Bl* b = &g_bl[bi];

		if(!b->on)
			continue;

		for(int ri=0; ri<RESOURCES; ri++)
		{
			int ui = b->transporter[ri];

			if(ui < 0)
				continue;

			Mv* mv = &g_mv[ui];

			ecbool fail = ecfalse;
			unsigned int flag = 0;

			if(!mv->on)
			{
				flag |= 1;
				fail = ectrue;
			}

			if(mv->type != MV_TRUCK)
			{
				flag |= 2;
				fail = ectrue;
			}

			if(mv->mode != UMODE_ATDEMB &&
				mv->mode != UMODE_GODEMB &&
				mv->mode != UMODE_GOSUP &&
				mv->mode != UMODE_ATSUP)
			{
				flag |= 4;
				fail = ectrue;
			}

			if(mv->target != bi)
			{
				flag |= 8;
				fail = ectrue;
			}

			if(!fail)
				continue;

			BlType* bt = &g_bltype[b->type];

			char msg[1280];
			sprintf(msg, "Consistency check failed #%u:\ntransporter to %s umode=%d ucargotype=%d ucargoamt=%d", flag, bt->name, (int)mv->mode, mv->cargotype, mv->cargoamt);
			InfoMess("csfail", msg);
		}
	}
}

ecbool SaveMap(const char* relative)
{
	char fullpath[SFH_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "wb");

	if(!fp)
		return ecfalse;

	char tag[] = MAP_TAG;
	int version = MAP_VERSION;

	fwrite(&tag, sizeof(tag), 1, fp);
	fwrite(&version, sizeof(version), 1, fp);

	fwrite(&g_simframe, sizeof(g_simframe), 1, fp);

	SaveView(fp);
	SaveGraphs(fp);
	SavePys(fp);
	SaveHmap(fp);
	SaveDeps(fp);
	SaveFol(fp);
	SaveBls(fp);
	SaveUnits(fp);
	SaveCd(fp);
	//SaveGr(fp);
	SaveJams(fp);
	SaveVis(fp);
	SaveBords(fp);
	SaveTriggers(fp);
	SaveHint(&g_lasthint, fp);

	fclose(fp);

	return ectrue;
}

//downafter: was the map downloaded and loaded here after the live sim data was loaded?
//live: basically downafter, but also works if map switched during play
ecbool LoadMap(const char* relative, ecbool live, ecbool downafter)
{
	if(!live)
		FreeMap();

	char fullpath[SFH_MAX_PATH+1];
	FullWritePath(relative, fullpath);

	FILE *fp = NULL;

	fp = fopen(fullpath, "rb");

	if(!fp)
		return ecfalse;

	char realtag[] = MAP_TAG;
	int version = MAP_VERSION;
	char tag[ sizeof(realtag) ];

	fread(&tag, sizeof(tag), 1, fp);

	if(memcmp(tag, realtag, sizeof(tag)) != 0)
	{
		ErrMess("Error", "Incorrect header tag in map file.");
		fclose(fp);
		return ecfalse;
	}

	fread(&version, sizeof(version), 1, fp);

	if(version != MAP_VERSION)
	{
		fclose(fp);
		char msg[128];
		sprintf(msg, "Map file version (%i) doesn't match %i.", version, MAP_VERSION);
		ErrMess("Error", msg);
		return ecfalse;
	}

	if(!live)
		fread(&g_simframe, sizeof(g_simframe), 1, fp);
	else
		fseek(fp, sizeof(unsigned __int64), SEEK_CUR);

	Log("read view");
	
	ReadView(fp);
	Log("read pys");

	ReadGraphs(fp);
	Log("read g");
	
	ReadPys(fp);
	Log("read hmap");
	
	ReadHmap(fp, live, downafter);
	Log("read deps");
	
	ReadDeps(fp);
	Log("read fl");
	
	ReadFol(fp, live);
	Log("read bls");
	
	ReadBls(fp, live);
	Log("read mv");
	
	ReadUnits(fp, live);
	Log("read ctile");
	
	ReadCd(fp, live);
	Log("fill col grd");

	//ReadGr(fp);
	ReadJams(fp, live);
	ReadVis(fp, live);
	
	ReadBords(fp, live);

	FillColliderGrid();
	Log("loaded m");
	

	//ReadTriggers(fp);
	//ReadLastHint(fp);

	int numt;
	fread(&numt, sizeof(int), 1, fp);
    
	Trigger* prev = NULL;
	Trigger* t;
	std::vector<int> triggerrefs;
    
	for(int i=0; i<numt; i++)
	{
		t = new Trigger();
        
		if(i==0)
			g_scripthead = t;
        
		if(prev != NULL)
			prev->next = t;
        
		t->prev = prev;
        
		ReadTrigger(t, &triggerrefs, fp);
        
		prev = t;
	}
    
	int refnum = 0;
	for(t=g_scripthead; t; t=t->next)
	{
		for(int i=0; i<t->effects.size(); i++)
		{
			Effect* e = &t->effects[i];
			e->trigger = GetTrigger(triggerrefs[refnum]);
			refnum++;
		}
	}
    
	ReadHint(&g_lasthint, fp);

	fseek(fp, 0, SEEK_END);
	g_mapfsz = ftell(fp);

	fclose(fp);

	ConsistCh();

	g_mapcheck = CheckSum(fullpath);

	return ectrue;
}
