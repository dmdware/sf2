










#include "collidertile.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "../sim/unit.h"
#include "../sim/utype.h"
#include "../sim/building.h"
#include "../sim/bltype.h"
#include "../render/heightmap.h"
#include "../math/hmapmath.h"
#include "../phys/collision.h"
#include "../render/water.h"
#include "../utils.h"
#include "../render/shader.h"
#include "../sim/selection.h"
#include "../sim/simdef.h"
#include "../phys/trace.h"
#include "../algo/binheap.h"
#include "pathnode.h"
#include "../math/vec2i.h"
#include "pathdebug.h"
#include "pathjob.h"
#include "../debug.h"
#include "../sim/conduit.h"
#include "../sim/transport.h"
#include "../phys/collision.h"
#include "fillbodies.h"
#include "tilepath.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"
#include "../render/drawsort.h"
#include "../sim/simflow.h"

//ColliderTile *g_collider = NULL;
//BitSet g_collider;

#if 0
ColliderTile::ColliderTile()
{
#if 0
	bool hasroad;
	bool hasland;
	bool haswater;
	bool abrupt;	//abrupt incline?
	std::list<int32_t> units;
	std::list<int32_t> foliage;
	int32_t building;
#endif

	//hasroad = false;
	//hasland = false;
	//haswater = false;
	//abrupt = false;
	flags = 0;
	building = -1;
#ifdef TILESIZECOLLIDER
	units.clear();
	foliages.clear();
#else
	//for(int32_t i=0; i<MAX_COLLIDER_UNITS; i++)
	//	units[i] = -1;
	unit = -1;
	foliage = USHRT_MAX;
#endif
}
#endif

 int calls = 0;

void FreePathGrid()
{
	//Log("free path gr");

	//Log("calls %d", calls);

#if 0
	if(g_collider)
	{
		delete [] g_collider;
		g_collider = NULL;
	}
#else
	//g_collider.clearall();
	//g_collider.~BitSet();
#endif

	g_pathdim = Vec2i(0,0);

#ifndef HASHPOOL
	if(g_pathnode)
	{
		delete [] g_pathnode;
		g_pathnode = NULL;
	}
#else
	g_pathmem.freesysmem();
	//g_pathmap.free();
	g_pathmap.clear();
#endif

#ifdef GPATHFLAGS
	g_pathflags.resize(0);
#endif

	g_stackpool.freesysmem();

	g_openlist[0].free();
	g_openlist[1].free();

	if(g_tilepass)
	{
		delete [] g_tilepass;
		g_tilepass = NULL;
	}

	if(g_tregs)
	{
		delete [] g_tregs;
		g_tregs = NULL;
	}

	if(g_tilenode)
	{
		delete [] g_tilenode;
		g_tilenode = NULL;
	}
}

void AllocPathGrid(int32_t cmwx, int32_t cmwy)
{
	FreePathGrid();
	g_pathdim.x = cmwx / PATHNODE_SIZE;
	g_pathdim.y = cmwy / PATHNODE_SIZE;
	//g_collider = new ColliderTile [ g_pathdim.x * g_pathdim.y ];
	//g_collider.resize(g_pathdim.x * g_pathdim.y);

	Log("path gr allc %d,%d\r\n", g_pathdim.x, g_pathdim.y);

	int32_t cwx = g_pathdim.x;
	int32_t cwy = g_pathdim.y;

#ifndef HASHPOOL
	g_pathnode = new PathNode [ cwx * cwy ];
	if(!g_pathnode) OUTOFMEM();
	
	for(int32_t x=0; x<cwx; x++)
		for(int32_t y=0; y<cwy; y++)
		{
			PathNode* n = PATHNODEAT(x, y);
			//n->nx = x;
			//n->ny = z;
			//n->opened = false;
			//n->closed = false;
#ifndef GPATHFLAGS
			n->flags = 0;
#endif
		}
#else
	//g_pathmem.allocsys(cwx*cwy, sizeof(PathNode));
	g_pathmem.allocsys(MAXPATHN * sizeof(PathNode));
#endif

#ifdef GPATHFLAGS
	g_pathflags.resize(PATHNODE_FLAGBITS * g_pathdim.x * g_pathdim.y);
#endif

	
	//g_pathmem.allocsys(MAX_MAP * MAX_MAP * (TILE_SIZE/PATHNODE_SIZE) * (TILE_SIZE/PATHNODE_SIZE) * sizeof(PathNode));
	//g_stackpool.allocsys(1024 * 1024 * 500);	//500 MB

	//g_openlist[0].alloc( cwx * cwy );
	//g_openlist[1].alloc( cwx * cwy );

	g_tilepass = new TileRegs [ (cmwx / TILE_SIZE) * (cmwy / TILE_SIZE) ];
	if(!g_tilepass) OUTOFMEM();

	g_tregs = new unsigned char [ g_pathdim.x * g_pathdim.y ];
	if(!g_tregs) OUTOFMEM();

	g_tilenode = new TileNode [ (cmwx / TILE_SIZE) * (cmwy / TILE_SIZE) ];
	if(!g_tilenode) OUTOFMEM();

	//g_lastpath = g_simframe;
}

void FillColliderGrid()
{
	const int32_t cwx = g_pathdim.x;
	const int32_t cwy = g_pathdim.y;

	//Log("path gr %dcwx,%dcwy", cwx, cwy);

	for(int32_t x=0; x<cwx; x++)
		for(int32_t y=0; y<cwy; y++)
		{
			int32_t cmx = x*PATHNODE_SIZE;
			int32_t cmy = y*PATHNODE_SIZE;
			//ColliderTile* cell = ColliderAt(x, y);

			//Log("cell "<<x<<","<<y<<" cmpos="<<cmx<<","<<cmy<<" y="<<g_hmap.accheight(cmx, cmy));

			//if(AtLand(cmx, cmy))
			{
				//cell->hasland = true;
				//cell->flags |= FLAG_HASLAND;
				//Log("land "<<(cmx/TILE_SIZE)<<","<<(cmy/TILE_SIZE)<<" flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(uint32_t)cell->flags);
				//Log("land");
			}
#if 0
			else
			{
				//cell->hasland = false;
				cell->flags &= ~FLAG_HASLAND;
			}
#endif

#if 0
			if(AtWater(cmx, cmy))
				cell->haswater = true;
			else
				cell->haswater = false;
#endif

#if 0
			if(TileUnclimable(cmx, cmy) && (cell->flags & FLAG_HASLAND))
			{
				//cell->abrupt = true;
				cell->flags |= FLAG_ABRUPT;
			}
			else
#endif
			{
				//cell->abrupt = false;
				//cell->flags &= ~FLAG_ABRUPT;
			}

			int32_t tx = cmx/TILE_SIZE;
			int32_t ty = cmy/TILE_SIZE;

			CdTile* r = GetCd(CD_ROAD, tx, ty, false);

#if 0
			//if(r->on /* && r->finished */ )
			if(r->on && r->finished)
			{
				//cell->hasroad = true;
				cell->flags |= FLAG_HASROAD;
			}
			else
			{
				//cell->hasroad = false;
				cell->flags &= ~FLAG_HASROAD;
			}
#endif
		}

#if 1
	for(int32_t x=0; x<LARGEST_UNIT_NODES; x++)
		for(int32_t y=0; y<cwy; y++)
		{
			//ColliderTile* cell = ColliderAt(x, y);
			//cell->flags |= FLAG_ABRUPT;
			//g_applog<<"abrupt "<<x<<"/"<<cwx<<","<<y<<"/"<<cwy<<std::endl;
			int32_t i = PATHNODEINDEX(x, y);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;
			//Log("ON xyi %d,%d,%d nxy %d,%d check %d", x, y, i, i%g_pathdim.x, i/g_pathdim.x, (int32_t)g_collider.on(i));
		}

	for(int32_t x=cwx-LARGEST_UNIT_NODES-1; x<cwx; x++)
		for(int32_t y=0; y<cwy; y++)
		{
			//ColliderTile* cell = ColliderAt(x, y);
			//cell->flags |= FLAG_ABRUPT;
			//g_applog<<"abrupt "<<x<<"/"<<cwx<<","<<y<<"/"<<cwy<<std::endl;
			int32_t i = PATHNODEINDEX(x, y);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;
			//Log("ON xyi %d,%d,%d nxy %d,%d check %d", x, y, i, i%g_pathdim.x, i/g_pathdim.x, (int32_t)g_collider.on(i));
		}

	for(int32_t x=0; x<cwx; x++)
		for(int32_t y=0; y<LARGEST_UNIT_NODES; y++)
		{
			//ColliderTile* cell = ColliderAt(x, y);
			//cell->flags |= FLAG_ABRUPT;
			//g_applog<<"abrupt "<<x<<"/"<<cwx<<","<<y<<"/"<<cwy<<std::endl;
			int32_t i = PATHNODEINDEX(x,y);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;
			//Log("ON xyi %d,%d,%d nxy %d,%d check %d", x, y, i, i%g_pathdim.x, i/g_pathdim.x, (int32_t)g_collider.on(i));
		}

	for(int32_t x=0; x<cwx; x++)
		for(int32_t y=cwy-LARGEST_UNIT_NODES-1; y<cwy; y++)
		{
			//ColliderTile* cell = ColliderAt(x, y);
			//cell->flags |= FLAG_ABRUPT;
			//g_applog<<"abrupt "<<x<<"/"<<cwx<<","<<y<<"/"<<cwy<<std::endl;
			int32_t i = PATHNODEINDEX(x, y);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;
			//Log("ON xyi %d,%d,%d nxy %d,%d check %d", x, y, i, i%g_pathdim.x, i/g_pathdim.x, (int32_t)g_collider.on(i));
		}
#endif

#if 1
	for(int32_t i=0; i<UNITS; i++)
	{
		Unit* u = &g_unit[i];

		if(!u->on)
			continue;

		if(u->hidden())
			continue;

		
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f1");

		u->fillcollider();

		AddVis(u);
		Explore(u);
	}
#endif

	//Log("fill cb");
	//

	for(int32_t i=0; i<BUILDINGS; i++)
	{
		Building* b = &g_building[i];

		if(!b->on)
			continue;

		//Log("fill cb "<<i<<" c");
		//

		b->fillcollider();

		//Log("fill cb "<<i<<" v");
		//
		//if(b->finished)
		AddVis(b);

		//Log("fill cb "<<i<<" e");
		//

		Explore(b);
	}

	for(int32_t i=0; i<FOLIAGES; i++)
	{
		Foliage* f = &g_foliage[i];

		if(!f->on)
			continue;

		f->fillcollider();
	}

	ResetPathNodes();
#if 1
	FillBodies();
#endif
}

void Foliage::fillcollider()
{
	return;

	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int32_t cmminx = cmpos.x - ft->size.x/2;
	int32_t cmminy = cmpos.y - ft->size.x/2;
	int32_t cmmaxx = cmminx + ft->size.x - 1;
	int32_t cmmaxy = cmminy + ft->size.x - 1;

#if 0
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);
#endif

#ifdef TILESIZECOLLIDER
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//t = tile position
	int32_t tminx = cmminx / TILE_SIZE;
	int32_t tminy = cmminy / TILE_SIZE;
	int32_t tmaxx = cmmaxx / TILE_SIZE;
	int32_t tmaxy = cmmaxy / TILE_SIZE;

	uint16_t fi = this - g_foliage;

	for(int32_t ty = tminy; ty <= tmaxy; ty++)
		for(int32_t tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);
			c->foliages.push_back( fi );
		}

#elif 0
	//c = cell position
	int32_t nminx = imax(0, cmminx / PATHNODE_SIZE);
	int32_t nminy = imax(0, cmminy / PATHNODE_SIZE);
	int32_t nmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int32_t nmaxy = imin(g_pathdim.y-1, cmmaxy / PATHNODE_SIZE);

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);
			c->foliage = this - g_foliage;
		}
#else
	int32_t nx = cmpos.x / PATHNODE_SIZE;
	int32_t ny = cmpos.y / PATHNODE_SIZE;
	//ColliderTile* c = ColliderAt(nx, ny);
	//c->foliage = this - g_foliage;
	int32_t i = PATHNODEINDEX(nx, ny);
	//g_collider.set(i);
	PathNode* n = g_pathnode + i;
	n->flags |= PATHNODE_BLOCKED;
#endif
}

void Foliage::freecollider()
{
	return;

	FlType* ft = &g_fltype[type];

	//cm = centimeter position
	int32_t cmminx = cmpos.x - ft->size.x/2;
	int32_t cmminy = cmpos.y - ft->size.x/2;
	int32_t cmmaxx = cmminx + ft->size.x - 1;
	int32_t cmmaxy = cmminy + ft->size.x - 1;

#if 0
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);
#endif

#ifdef TILESIZECOLLIDER
	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//t = tile position
	int32_t tminx = cmminx / TILE_SIZE;
	int32_t tminy = cmminy / TILE_SIZE;
	int32_t tmaxx = cmmaxx / TILE_SIZE;
	int32_t tmaxy = cmmaxy / TILE_SIZE;

	uint16_t fi = this - g_foliage;

	for(int32_t ty = tminy; ty <= tmaxy; ty++)
		for(int32_t tx = tminx; tx <= tmaxx; tx++)
		{
			ColliderTile* c = ColliderAt(tx, ty);

			std::list<Widget*>::iterator fit=c->foliages.begin();
			while(fit!=c->foliages.end())
			{
				if(*fit != fi)
				{
					fit++;
					continue;
				}

				fit = c->foliages.erase(fit);
			}
		}
#elif 0
	//c = cell position
	int32_t nminx = imax(0, cmminx / PATHNODE_SIZE);
	int32_t nminy = imax(0, cmminy / PATHNODE_SIZE);
	int32_t nmaxx = imin(g_pathdim.x-1, cmmaxx / PATHNODE_SIZE);
	int32_t nmaxy = imin(g_pathdim.y-1, cmmaxy / PATHNODE_SIZE);

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			ColliderTile* c = ColliderAt(nx, ny);
			c->foliage = USHRT_MAX;
		}
#else
	int32_t nx = cmpos.x / PATHNODE_SIZE;
	int32_t ny = cmpos.y / PATHNODE_SIZE;
	//ColliderTile* c = ColliderAt(nx, ny);
	//c->foliage = USHRT_MAX;
	int32_t i = PATHNODEINDEX(nx, ny);
	//g_collider.clear(i);
	PathNode* n = g_pathnode + i;
	n->flags &= ~PATHNODE_BLOCKED;
#endif
}

std::list<int> filled;

void Unit::fillcollider()
{
#if 0
	if (filled)
		return;

	filled = true;
#endif

	UType* t = &g_utype[type];
	int32_t ui = this - g_unit;

	//cm = centimeter position
	int32_t cmminx = cmpos.x - t->size.x/2;
	int32_t cmminy = cmpos.y - t->size.x/2;
	int32_t cmmaxx = cmminx + t->size.x - 1;
	int32_t cmmaxy = cmminy + t->size.x - 1;

	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//c = cell position
	int32_t nminx = cmminx / PATHNODE_SIZE;
	int32_t nminy = cmminy / PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx / PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy / PATHNODE_SIZE;

#if 0
	if(this - g_unit == 182 && g_simframe > 118500)
	{
		Log("======== %llu unit 182 fill ========", g_simframe);
	}
#endif

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			//ColliderTile* c = ColliderAt(nx, ny);

			//c->unit = ui;
			int32_t i = PATHNODEINDEX(nx, ny);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;

#if 0		
	if(this - g_unit == 182 && g_simframe > 118500)
	{
		Log(" %llu unit 182 fill %d", g_simframe, i);
		filled.push_back(i);
	}
#endif
		}

#if 0
	if(this - g_unit == 182 && g_simframe > 118500)
	{
		Log("======== ^^^ %llu unit 182 free ========", g_simframe);
	}
#endif
}

void Building::fillcollider()
{
	BlType* t = &g_bltype[type];
	int32_t bi = this - g_building;

	//t = tile position
	int32_t tminx = tpos.x - t->width.x/2;
	int32_t tminz = tpos.y - t->width.y/2;
	int32_t tmaxx = tminx + t->width.x;
	int32_t tmaxz = tminz + t->width.y;

	//cm = centimeter position
	int32_t cmminx = tminx*TILE_SIZE;
	int32_t cmminy = tminz*TILE_SIZE;
	int32_t cmmaxx = tmaxx*TILE_SIZE - 1;
	int32_t cmmaxy = tmaxz*TILE_SIZE - 1;

	//c = cell position
	int32_t nminx = cmminx / PATHNODE_SIZE;
	int32_t nminy = cmminy / PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx / PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			//ColliderTile* c = ColliderAt(nx, ny);
			//c->building = bi;
			int32_t i = PATHNODEINDEX(nx, ny);
			//g_collider.set(i);
			PathNode* n = g_pathnode + i;
			n->flags |= PATHNODE_BLOCKED;
		}
}

void Unit::freecollider()
{
#if 0
	if (!filled)
		return;

	filled = false;
#endif

	UType* t = &g_utype[type];
	int32_t ui = this - g_unit;

	//cm = centimeter position
	int32_t cmminx = cmpos.x - t->size.x/2;
	int32_t cmminy = cmpos.y - t->size.x/2;
	int32_t cmmaxx = cmminx + t->size.x - 1;
	int32_t cmmaxy = cmminy + t->size.x - 1;

	cmminx = imax(cmminx, 0);
	cmminy = imax(cmminy, 0);
	cmmaxx = imin(cmmaxx, g_mapsz.x*TILE_SIZE-1);
	cmmaxy = imin(cmmaxy, g_mapsz.y*TILE_SIZE-1);

	//c = cell position
	int32_t nminx = cmminx / PATHNODE_SIZE;
	int32_t nminy = cmminy / PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx / PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy / PATHNODE_SIZE;

#if 0
	if(this - g_unit == 182 && g_simframe > 118500)
	{
		Log("======== %llu unit 182 free ========", g_simframe);
	}
#endif

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			//c->unit = -1;
			int32_t i = PATHNODEINDEX(nx, ny);
			//g_collider.clear(i);
			PathNode* n = g_pathnode + i;
			n->flags &= ~PATHNODE_BLOCKED;
	
#if 0
	if(this - g_unit == 182 && g_simframe > 118500)
	{
		Log("%llu unit 182 free %d ", g_simframe, i);


		for(std::list<Widget*>::iterator fit=filled.begin(); fit!=filled.end(); ++fit)
			if(*fit == i)
				fit = filled.erase(fit);
	}
#endif
		}

#if 0
	if(this - g_unit == 182 && g_simframe > 118500)
	{


		if(filled.size())
		{
			char m[123];
			sprintf(m, "%d not freed!!!", *filled.begin());
			InfoMess("asd",m);

			for(std::list<Widget*>::iterator fit=filled.begin(); fit!=filled.end(); fit++)
				Log("not f %d", *fit);
		}
		Log("======== ^^ %llu unit 182 free ========", g_simframe);
	}
#endif
}

void Building::freecollider()
{
	BlType* t = &g_bltype[type];
	int32_t bi = this - g_building;

	//t = tile position
	int32_t tminx = tpos.x - t->width.x/2;
	int32_t tminy = tpos.y - t->width.y/2;
	int32_t tmaxx = tminx + t->width.x;
	int32_t tmaxy = tminy + t->width.y;

	//cm = centimeter position
	int32_t cmminx = tminx*TILE_SIZE;
	int32_t cmminy = tminy*TILE_SIZE;
	int32_t cmmaxx = tmaxx*TILE_SIZE - 1;
	int32_t cmmaxy = tmaxy*TILE_SIZE - 1;

	//c = cell position
	int32_t nminx = cmminx / PATHNODE_SIZE;
	int32_t nminy = cmminy / PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx / PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy / PATHNODE_SIZE;

	for(int32_t ny = nminy; ny <= nmaxy; ny++)
		for(int32_t nx = nminx; nx <= nmaxx; nx++)
		{
			//ColliderTile* c = ColliderAt(nx, ny);

			//if(c->building == bi)
			//	c->building = -1;

			int32_t i = PATHNODEINDEX(nx, ny);
			//g_collider.clear(i);
			PathNode* n = g_pathnode + i;
			n->flags &= ~PATHNODE_BLOCKED;
		}
}

// Uses cm pos instead of pathnode pos
// Uses cm-accurate intersect checks
bool Standable2(const PathJob* pj, int32_t cmposx, int32_t cmposy)
{
	UType* ut = &g_utype[pj->utype];

#ifdef TILESIZECOLLIDER
	const int32_t tx = cmposx / TILE_SIZE;
	const int32_t ty = cmposy / TILE_SIZE;

	ColliderTile* cell = ColliderAt( tx, ty );
#else
	const int32_t nx = (cmposx - PathOff(ut->size.x)) / PATHNODE_SIZE;
	const int32_t ny = (cmposy - PathOff(ut->size.x)) / PATHNODE_SIZE;

#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

	//ColliderTile* cell = ColliderAt( nx, ny );
#endif

#if 1
	int32_t ni = PATHNODEINDEX(nx, ny);
	//g_collider.on(ni);

	//if(cell->flags & FLAG_ABRUPT)
	//if(g_collider.on(ni))
	{
		//Log("abrupt");
		//return false;
	}

#if 0
	if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{
		//Log("!land flag="<<(cell->flags & FLAG_HASLAND)<<"="<<(uint32_t)cell->flags);
		return false;
	}

	if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		//Log("!sea");
		return false;
	}
#endif
#endif

	int32_t cmminx = cmposx - ut->size.x/2;
	int32_t cmminy = cmposy - ut->size.x/2;
	int32_t cmmaxx = cmminx + ut->size.x - 1;
	int32_t cmmaxy = cmminy + ut->size.x - 1;
	
#if 0
	if(cmminx < 0)
		return false;
	if(cmminy < 0)
		return false;
	if(cmmaxx >= g_mapsz.x*TILE_SIZE)
		return false;
	if(cmmaxy >= g_mapsz.y*TILE_SIZE)
		return false;
#endif

#ifndef TILESIZECOLLIDER
	int32_t nminx = cmminx/PATHNODE_SIZE;
	int32_t nminy = cmminy/PATHNODE_SIZE;
	int32_t nmaxx = cmmaxx/PATHNODE_SIZE;
	int32_t nmaxy = cmmaxy/PATHNODE_SIZE;
#else
	int32_t tminx = cmminx /. TILE_SIZE;
	int32_t tminy = cmminy /. TILE_SIZE;
	int32_t tmaxx = cmmaxx /. TILE_SIZE;
	int32_t tmaxy = cmmaxy /. TILE_SIZE;
#endif

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
	bool bcollided = false;
	bool hasroad = false;

#ifdef TILESIZECOLLIDER
	for(int32_t y=tminy; y<=tmaxy; y++)
		for(int32_t x=tminx; x<=tmaxx; x++)
#else
	for(int32_t y=nminy; y<=nmaxy; y++)
		for(int32_t x=nminx; x<=nmaxx; x++)
#endif
		{
			//cell = ColliderAt(x, y);
			
#if 0
			CdTile* ctile = GetCd(CD_ROAD, 
				x * PATHNODE_SIZE / TILE_SIZE, 
				y * PATHNODE_SIZE / TILE_SIZE, 
				false);

			if(ctile->on && ctile->finished)
				hasroad = true;
#endif

#ifdef TILESIZECOLLIDER
			for(std::list<Widget*>::iterator fit=cell->foliages.begin(); fit!=cell->foliages.end(); fit++)
			{
				Foliage* f = &g_foliage[*fit];
				FlType* ft = &g_foltype[f->type];

				int32_t fcmminx = f->cmpos.x - ft->size.x/2;
				int32_t fcmminy = f->cmpos.y - ft->size.y/2;
				int32_t fcmmaxx = fcmminx + ft->size.x;
				int32_t fcmmaxy = fcmminy + ft->size.y;

				if(fcmminx > cmmaxx)
					continue;

				if(fcmminy > cmmaxy)
					continue;

				if(fcmmaxx < cmminx)
					continue;

				if(fcmmaxy < cmminy)
					continue;

				collided = true;
				break;
			}
#else

			//int32_t ni = PATHNODEINDEX(nx, ny);
			int32_t ni = PATHNODEINDEX(x, y);	//corpd fix 2016
			//if (g_collider.on(ni))
			//	collided = true;
			
			PathNode* n = g_pathnode + ni;
			if(n->flags & PATHNODE_BLOCKED)
				collided = true;

			//if(cell->foliage != USHRT_MAX)
			//	collided = true;
#endif

#ifdef TILESIZECOLLIDER
			if(cell->building >= 0)
			{
				collided = true;

				if(cell->building != pj->targb)
				{
					bcollided = true;
					return false;
				}
				else
					ignoredb = true;
			}
#else
#if 0
			//if(cell->building >= 0)
			if(pj->targb >= 0)
			{
				Building* b = &g_building[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int32_t tminx = b->tpos.x - t2->width.x/2;
				int32_t tminy = b->tpos.y - t2->width.y/2;
				int32_t tmaxx = tminx + t2->width.x;
				int32_t tmaxy = tminy + t2->width.y;

				int32_t minx2 = tminx*TILE_SIZE;
				int32_t miny2 = tminy*TILE_SIZE;
				int32_t maxx2 = tmaxx*TILE_SIZE - 1;
				int32_t maxy2 = tmaxy*TILE_SIZE - 1;

				if(cmminx <= maxx2 && cmminy <= maxy2 && cmmaxx >= minx2 && cmmaxy >= miny2)
				{
					//Log("bld");
					collided = true;

					if(cell->building != pj->targb)
					{
						bcollided = true;
						return false;
					}
					else
						ignoredb = true;
				}
			}
#endif
#endif

#ifdef TILESIZECOLLIDER
			for(std::list<Widget*>::iterator uit=cell->units.begin(); uit!=cell->units.end(); uit++)
			{
				int16_t uin = *uit;

				Unit* u = &g_unit[uin];

				if(uin != pj->thisu && uindex != pj->targu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int32_t cmminx2 = u->cmpos.x - t->size.x/2;
					int32_t cmminy2 = u->cmpos.y - t->size.x/2;
					int32_t cmmaxx2 = cmminx2 + t->size.x - 1;
					int32_t cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{
						//Log("u");
						//return false;
						collided = true;

						if(uin == pj->targu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
#else
#if 0
			for(int16_t uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				int16_t uindex = cell->units[uiter];

				if( uindex < 0 )
					continue;

				Unit* u = &g_unit[uindex];

				if(uindex != pj->thisu && uindex != pj->targu && !u->hidden())
				{
#if 1
					UType* t = &g_utype[u->type];

					int32_t cmminx2 = u->cmpos.x - t->size.x/2;
					int32_t cmminy2 = u->cmpos.y - t->size.x/2;
					int32_t cmmaxx2 = cmminx2 + t->size.x - 1;
					int32_t cmmaxy2 = cmminy2 + t->size.x - 1;

					if(cmmaxx >= cmminx2 && cmmaxy >= cmminy2 && cmminx <= cmmaxx2 && cmminy <= cmmaxy2)
					{
						//Log("u");
						//return false;
						collided = true;

						if(uindex == pj->targu)
							ignoredu = true;
					}
#else
					return false;
#endif
				}
			}
#else
#if 0
			if(cell->unit >= 0 && cell->unit != pj->thisu && !g_unit[cell->unit].hidden())
			{
				collided = true;
				
				if(cell->unit != pj->targu)
					ignoredu = true;
				//return true;
			}
#endif
#endif
		}
#endif

	bool arrived = false;

	if(cmminx < pj->goalmaxx &&
		cmminy < pj->goalmaxy &&
		cmmaxx > pj->goalminx &&
		cmmaxy > pj->goalminy)
		arrived = true;

//#ifndef HIERPATH
	//needs to be not set to false for HIERPATH because otherwise can only path to exact cm pos for tile goal (?)
	arrived = false;	//ditto
//#endif

	if (collided)
	{
		if (pj->targb >= 0)
		{
			Building* b = &g_building[pj->targb];
			BlType* t2 = &g_bltype[b->type];

			int32_t tminx = b->tpos.x - t2->width.x / 2;
			int32_t tminy = b->tpos.y - t2->width.y / 2;
			int32_t tmaxx = tminx + t2->width.x;
			int32_t tmaxy = tminy + t2->width.y;

			int32_t cmminx2 = tminx*TILE_SIZE;
			int32_t cmminy2 = tminy*TILE_SIZE;
			int32_t cmmaxx2 = tmaxx*TILE_SIZE - 1;
			int32_t cmmaxy2 = tmaxy*TILE_SIZE - 1;

			if (cmminx <= cmmaxx2 && cmminy <= cmmaxy2 && cmmaxx >= cmminx2 && cmmaxy >= cmminy2)
			{
				//Log("bld");
				//collided = true;
				ignoredb = true;
			}
		}

		if (pj->targu >= 0)
		{
			Unit* igu = &g_unit[pj->targu];
			UType* t2 = &g_utype[igu->type];

			int32_t cmminx2 = igu->cmpos.x - t2->size.x / 2;
			int32_t cmminy2 = igu->cmpos.y - t2->size.x / 2;
			int32_t cmmaxx2 = cmminx2 + t2->size.x - 1;
			int32_t cmmaxy2 = cmminy2 + t2->size.x - 1;

			int32_t nminx2 = cmminx2 / PATHNODE_SIZE;
			int32_t nminy2 = cmminy2 / PATHNODE_SIZE;
			int32_t nmaxx2 = cmmaxx2 / PATHNODE_SIZE;
			int32_t nmaxy2 = cmmaxy2 / PATHNODE_SIZE;

			if (nminx <= nmaxx2 && nminy <= nmaxy2 && nmaxx >= nminx2 && nmaxy >= nminy2)
			{
				//collided = true;
				ignoredu = true;
			}
		}
	}
	
#if 1
	//new

	//might still collide becuase in Standable there is
#if 0
	if(roaded)
	{
		CdTile* ctile = GetCd(CD_ROAD,
			nx / (TILE_SIZE/PATHNODE_SIZE),
			ny / (TILE_SIZE/PATHNODE_SIZE),
			false);

		return ctile->on && ctile->finished;
	}
#endif

	//so.... better check

	if(pj->roaded)
	{
		int32_t pathoff = PathOff(ut->size.x);

		CdTile* ctile = GetCd(CD_ROAD, 
			(cmposx /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			(cmposy /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			false);
		
		hasroad = hasroad || (ctile->on && ctile->finished);
		
		if(!hasroad && !ignoredb && !ignoredu && (!arrived || bcollided))
			return false;
	}
#else	//old check...
	
	if(pj->roaded)
	{
		CdTile* ctile = GetCd(CD_ROAD,
			cmposx / TILE_SIZE,
			cmposy / TILE_SIZE,
			false);

		//if(ctile->on && ctile->finished)
		//	hasroad = true;
		hasroad = hasroad || (ctile->on && ctile->finished);

		//CdTile* cdtile = GetCd(CD_ROAD, cmposx / TILE_SIZE, cmposy / TILE_SIZE, false);
		//if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu && (!arrived || bcollided))
		//Log("!road");
		if(!hasroad && !ignoredb && !ignoredu && (!arrived || bcollided))
			return false;
	}

#endif
	
	//int32_t ui = pj->thisu;
	//if(((g_simframe+ui)%1001!=1))
	//	return true;

	if(collided && 
		!ignoredb && !ignoredu 
#ifndef HIERPATH
		&& (!arrived || bcollided)
#endif
		)
		return false;

	return true;
}

//#define TRANSPORT_DEBUG
//#define POWCD_DEBUG

bool Standable(const int16_t nx, const int16_t ny, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded)
{
#if 1
	//assuming -pathoff aligns cmmin with 0 boundary

	//const uint8_t nsizex = sizex/PATHNODE_SIZE;	//todo move out
	
	int32_t ni = PATHNODEINDEX(nx, ny);
	const int32_t yadv = g_pathdim.x;
	
	int32_t xend = ni + nsizex;
	const int32_t yend = xend + g_pathdim.x * nsizey;

#if 0
	
	std::list<int> list3;

	const int16_t nminx = nx;
	const int16_t nminy = ny;
	const int16_t nmaxx = nx + nsizex;
	const int16_t nmaxy = ny + nsizey;

	while(ni <= yend)
	{
		while(ni <= xend)
		{
			list3.push_back(ni);

			++ni;
		}

		ni += yadv - nsizex - 1;
		xend += yadv;
	}

	std::list<Widget*>::iterator list3it = list3.begin();

	char m[1230] = "";


	for(uint16_t y=nminy; y<=nmaxy; ++y)
	{
		for(uint16_t x=nminx; x<=nmaxx; ++x)
		{
			//sprintf(m, "x%d ni%d", x, ni);
			//InfoMess(m,m);
			int32_t ni = PATHNODEINDEX(x, y);
			
			char add[123];

			if(list3it == list3.end())
			{
				sprintf(add, "\nfault: *list3it(done)!=ni(%d)", ni);
				strcat(m, add);
				InfoMess(m,m);
				goto doneexam;
			}

			if(*list3it != ni)
			{
				sprintf(add, "\nfault: *list3it(%d)!=ni(%d)", *list3it, ni);
				strcat(m, add);
				InfoMess(m,m);
				goto doneexam;
			}

			sprintf(add, "%d,", ni);
			strcat(m, add);

			list3it++;
		}
	}

	if(list3it != list3.end())
	{
		char add[123];
		sprintf(add, "\nfault: *list3it(%d)!=ni(done)", *list3it);
		strcat(m, add);
		InfoMess(m,m);
	}

doneexam:

	ni = PATHNODEINDEX(nx, ny);
	xend = ni + nsizex;
#endif

	while(ni <= yend)
	{
		while(ni <= xend)
		{
			//if(BITSET_ON(g_collider.m_bits, ni))

			PathNode* n = g_pathnode + ni;
			if(n->flags & PATHNODE_BLOCKED)
			{
				goto checkcollision;
			}

			++ni;
		}

		ni += yadv - nsizex - 1;
		xend += yadv;
	}
#endif
#if 0
	
	const int16_t nminx3 = nx;
	const int16_t nminy3 = ny;
	const int16_t nmaxx3 = nx + nsizex;
	const int16_t nmaxy3 = ny + nsizey;

	const int16_t cmposx = nx * PATHNODE_SIZE + pathoff;
	const int16_t cmposy = ny * PATHNODE_SIZE + pathoff;

	const int16_t cmminx = cmposx - (sizex>>1);
	const int16_t cmminy = cmposy - (sizex>>1);
	const int16_t cmmaxx = cmminx + sizex - 1;
	const int16_t cmmaxy = cmminy + sizex - 1;

	const int16_t nminx = cmminx/PATHNODE_SIZE;
	const int16_t nminy = cmminy/PATHNODE_SIZE;
	const int16_t nmaxx = cmmaxx/PATHNODE_SIZE;
	const int16_t nmaxy = cmmaxy/PATHNODE_SIZE;

	if(nminx3 != nminx ||
		nmaxx3 != nmaxx ||
		nminy3 != nminy ||
		nmaxy3 != nmaxy)
	{
		char m[123];
		sprintf(m, "fault nmin nmax x y\n full:(%d,%d,%d,%d) \n faulty:(%d,%d,%d,%d) \n sizex%d",
			nminx, nminy, nmaxx, nmaxy,
			nminx3, nminy3, nmaxx3, nmaxy3,
			(int)sizex);
		InfoMess(m,m);
	}
	
	for(uint16_t y=nminy; y<=nmaxy; ++y)
	{
		for(uint16_t x=nminx; x<=nmaxx; ++x)
		{
			//sprintf(m, "x%d ni%d", x, ni);
			//InfoMess(m,m);
			int32_t ni = PATHNODEINDEX(x, y);
			//if (g_collider.on(ni))
			if(BITSET_ON(g_collider.m_bits, ni))
			{
				goto checkcollision;
			}
		}
	}
#endif

	/*
	Need to comment out arrived for now for the sake of tile hierarchical pathfinding.
	Otherwise, when MazePath computes a path around a corner, it will say it is
	okay to collide with a building corner because we arrive at the next tile.
	But it was originally added here to fix trucks being stuck when they were right
	next to a building but clumped up, unable to deliver or recieve loads.
	Edit: experiencing this again after going back to astar from hierarchical.
	Near one building's corner hide to target building, colliding with other bl's corner.
	So the problem is the calling function that expands path nodes checks for
	goal bounds and immediately returns success if goal touched even though there
	is a collision.
	Edit: no, that was not the problem at all, some other bug like Disembark is making
	the truck change spots.
	*/

	if(roaded)
	{
		CdTile* ctile = GetCd(CD_ROAD,
			nx / (TILE_SIZE/PATHNODE_SIZE),
			ny / (TILE_SIZE/PATHNODE_SIZE),
			false);

		return ctile->on && ctile->finished;
	}

	return true;

checkcollision:

#if 1
	const int16_t nminx = nx;
	const int16_t nminy = ny;
	const int16_t nmaxx = nx + nsizex;
	const int16_t nmaxy = ny + nsizey;
#elif 0
	const int16_t cmposx = nx * PATHNODE_SIZE + pathoff;
	const int16_t cmposy = ny * PATHNODE_SIZE + pathoff;

	const int16_t cmminx = cmposx - (sizex>>1);
	const int16_t cmminy = cmposy - (sizex>>1);
	const int16_t cmmaxx = cmminx + sizex - 1;
	const int16_t cmmaxy = cmminy + sizex - 1;

	const int16_t nminx = cmminx/PATHNODE_SIZE;
	const int16_t nminy = cmminy/PATHNODE_SIZE;
	const int16_t nmaxx = cmmaxx/PATHNODE_SIZE;
	const int16_t nmaxy = cmmaxy/PATHNODE_SIZE;
#endif

	if (igb)
	{
		BlType* t2 = &g_bltype[igb->type];

		int16_t tminx = igb->tpos.x - t2->width.x / 2;
		int16_t tminy = igb->tpos.y - t2->width.y / 2;
		int16_t tmaxx = tminx + t2->width.x;
		int16_t tmaxy = tminy + t2->width.y;
		
		int16_t nminx2 = tminx * (TILE_SIZE/PATHNODE_SIZE);
		int16_t nminy2 = tminy * (TILE_SIZE/PATHNODE_SIZE);
		int16_t nmaxx2 = tmaxx * (TILE_SIZE/PATHNODE_SIZE) - 1;
		int16_t nmaxy2 = tmaxy * (TILE_SIZE/PATHNODE_SIZE) - 1;

		if (nminx <= nmaxx2 && nminy <= nmaxy2 && nmaxx >= nminx2 && nmaxy >= nminy2)
		{
			return true;
		}
	}

	if (igu)
	{
		UType* t2 = &g_utype[igu->type];

		//todo change all /2 to >>1 and *2 to <<1
		int16_t cmminx2 = igu->cmpos.x - t2->size.x / 2;
		int16_t cmminy2 = igu->cmpos.y - t2->size.x / 2;
		int16_t cmmaxx2 = cmminx2 + t2->size.x - 1;
		int16_t cmmaxy2 = cmminy2 + t2->size.x - 1;

		//todo use look-up table for divisions
		int16_t nminx2 = cmminx2 / PATHNODE_SIZE;
		int16_t nminy2 = cmminy2 / PATHNODE_SIZE;
		int16_t nmaxx2 = cmmaxx2 / PATHNODE_SIZE;
		int16_t nmaxy2 = cmmaxy2 / PATHNODE_SIZE;

		if (nminx <= nmaxx2 && nminy <= nmaxy2 && nmaxx >= nminx2 && nmaxy >= nminy2)
		{
			return true;
		}
	}
	
	//int32_t ui = thisu-g_unit;
	//if(((g_simframe+ui)%1001!=1))
	//	return true;

	return false;
}

bool TileStandable(const PathJob* pj, const int32_t nx, const int32_t ny)
{
#if 0
	if(nx < 0 || ny < 0 || nx >= g_pathdim.x || ny >= g_pathdim.y)
		return false;
#endif

#ifdef HIERDEBUG
	if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny == 68*TILE_SIZE/PATHNODE_SIZE)
	{
		Log("\t TileStandable? nxy "<<nx<<","<<ny<<" (t "<<(nx*PATHNODE_SIZE/TILE_SIZE)<<","<<(ny*PATHNODE_SIZE/TILE_SIZE)<<")");
	}
#endif

	//ColliderTile* cell = ColliderAt( nx, ny );

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("stand? u"<<pj->thisu<<" n"<<nx<<","<<ny<<" dn("<<(nx-pj->goalx)<<","<<(ny-pj->goaly)<<")");

		char add[1280];
		sprintf(add, "std? u%d n%d,%d dn%d,%d \r\n", (int32_t)pj->thisu, (int32_t)nx, (int32_t)ny, (int32_t)(nx-pj->goalx), (int32_t)(ny-pj->goaly));
		powcdstr += add;
	}
#endif

#if 1
	//needed for trucks to drive to unfinished road tiles
	Vec2s tpos = Vec2s(nx, ny) * PATHNODE_SIZE / TILE_SIZE;

	//If at goal...
	uint32_t cmminx = tpos.x * TILE_SIZE;
	uint32_t cmminy = tpos.y * TILE_SIZE;
	uint32_t cmmaxx = (tpos.x+1) * TILE_SIZE - 1;
	uint32_t cmmaxy = (tpos.y+1) * TILE_SIZE - 1;
	
#if 0
	{
		const int32_t cmposx = nx * PATHNODE_SIZE + PathOff(ut->size.x);
		const int32_t cmposy = ny * PATHNODE_SIZE + PathOff(ut->size.x);
	
		if(pj->thisu == 36)
		{
			int32_t dtx = (int32_t)(cmposx/TILE_SIZE-pj->goalx);
			int32_t dty = (int32_t)(cmposy/TILE_SIZE-pj->goaly);
			char cm[128];
			sprintf(cm, "36 at dt%d,%d", dtx, dty);

			if(dtx == 0 && dty < 2)
				AddNotif(&RichText(cm));
		}
	}
#endif

	if(cmminx <= pj->goalmaxx &&
		cmminy <= pj->goalmaxy &&
		cmmaxx >= pj->goalminx &&
		cmmaxy >= pj->goalminy)
	{
		
#if 0
	{
		const int32_t cmposx = nx * PATHNODE_SIZE + PathOff(ut->size.x);
		const int32_t cmposy = ny * PATHNODE_SIZE + PathOff(ut->size.x);
		if(pj->thisu == 36)
		{
			int32_t dtx = (int32_t)(cmposx/TILE_SIZE-pj->goalx);
			int32_t dty = (int32_t)(cmposy/TILE_SIZE-pj->goaly);
			char cm[128];
			sprintf(cm, "ARR 36 at dt%d,%d", dtx, dty);

			if(dtx == 0 && dty < 2)
				AddNotif(&RichText(cm));
		}
	}
#endif

		return true;
	}
#endif

#if 1
	//if(cell->flags & FLAG_ABRUPT)
	//	return false;

	//if(pj->landborne && !(cell->flags & FLAG_HASLAND))
	{

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !land");
		char add[1280] = "\t !land \r\n";
		powcdstr += add;
	}
#endif

		//return false;
	}

	//if(pj->seaborne && (cell->flags & FLAG_HASLAND))
	{
		//return false;
	}
#endif

	const UType* ut = &g_utype[pj->utype];

	const int32_t cmposx = nx * PATHNODE_SIZE + PathOff(ut->size.x);
	const int32_t cmposy = ny * PATHNODE_SIZE + PathOff(ut->size.x);

#if 0
	const int32_t cmminx = cmposx - ut->size.x/2;
	const int32_t cmminy = cmposy - ut->size.x/2;
	const int32_t cmmaxx = cmminx + ut->size.x - 1;
	const int32_t cmmaxy = cmminy + ut->size.x - 1;
#endif
	const int32_t nminx = cmminx/PATHNODE_SIZE;
	const int32_t nminy = cmminy/PATHNODE_SIZE;
	const int32_t nmaxx = cmmaxx/PATHNODE_SIZE;
	const int32_t nmaxy = cmmaxy/PATHNODE_SIZE;

#if 0
	if(nminx < 0 || nminy < 0 || nmaxx >= g_pathdim.x || nmaxy >= g_pathdim.y)
	{
		return false;
	}
#endif

	bool ignoredb = false;
	bool ignoredu = false;
	bool collided = false;
	if (pj->targb >= 0)
	{
		Building* b = &g_building[pj->targb];
		BlType* t2 = &g_bltype[b->type];

		int32_t tminx = b->tpos.x - t2->width.x / 2;
		int32_t tminy = b->tpos.y - t2->width.y / 2;
		int32_t tmaxx = tminx + t2->width.x;
		int32_t tmaxy = tminy + t2->width.y;

		int32_t cmminx2 = tminx*TILE_SIZE;
		int32_t cmminy2 = tminy*TILE_SIZE;
		int32_t cmmaxx2 = tmaxx*TILE_SIZE - 1;
		int32_t cmmaxy2 = tmaxy*TILE_SIZE - 1;

		if (cmminx <= cmmaxx2 && cmminy <= cmmaxy2 && cmmaxx >= cmminx2 && cmmaxy >= cmminy2)
		{
			//Log("bld");
			collided = true;
			ignoredb = true;
		}
	}

	if (pj->targu >= 0)
	{
		Unit* igu = &g_unit[pj->targu];
		UType* t2 = &g_utype[igu->type];

		int32_t cmminx2 = igu->cmpos.x - t2->size.x / 2;
		int32_t cmminy2 = igu->cmpos.y - t2->size.x / 2;
		int32_t cmmaxx2 = cmminx2 + t2->size.x - 1;
		int32_t cmmaxy2 = cmminy2 + t2->size.x - 1;

		int32_t nminx2 = cmminx2 / PATHNODE_SIZE;
		int32_t nminy2 = cmminy2 / PATHNODE_SIZE;
		int32_t nmaxx2 = cmmaxx2 / PATHNODE_SIZE;
		int32_t nmaxy2 = cmmaxy2 / PATHNODE_SIZE;

		if (nminx <= nmaxx2 && nminy <= nmaxy2 && nmaxx >= nminx2 && nmaxy >= nminy2)
		{
			collided = true;
			ignoredu = true;
		}
	}

	if(pj->roaded)
	{
		CdTile* cdtile = GetCd(CD_ROAD, cmposx / TILE_SIZE, cmposy / TILE_SIZE, false);
		if((!cdtile->on || !cdtile->finished) && !ignoredb && !ignoredu)
		//Log("!road");
			return false;

#ifdef POWCD_DEBUG
	if(pj->umode == UMODE_GOCDJOB && g_unit[pj->thisu].cdtype == CD_POWL)
	{
		//Log("\t !road");
		char add[1280] = "\t !road \r\n";
		powcdstr += add;
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		Unit* u = &g_unit[pj->thisu];

		RichText rt;
		char t[1280];
		sprintf(t, "!roaded d(%d,%d) u%d", (int32_t)(pj->goalx-nx)*PATHNODE_SIZE/TILE_SIZE, (int32_t)(pj->goaly-ny)*PATHNODE_SIZE/TILE_SIZE, (int32_t)pj->thisu);
		rt.m_part.push_back(t);
		AddNotif(&rt);
	}
#endif

#ifdef TRANSPORT_DEBUG
	if(pj->thisu >= 0 && g_unit[pj->thisu].type == UNIT_TRUCK)
	{
		//Unit* u = &g_unit[thisu];

		RichText rt;
		char t[1280] = "";
		//sprintf(t, "pathfound u%d", (int32_t)thisu);
		rt.m_part.push_back(t);
		//AddNotif(&rt);
		Log("\t !r");
	}
#endif

		//return false;
	}

	if(collided && !ignoredu && !ignoredb)
		return false;

#ifdef HIERDEBUG
	//if(pathnum == 73 && nx == 72*TILE_SIZE/PATHNODE_SIZE && ny
#endif

	return true;
}
