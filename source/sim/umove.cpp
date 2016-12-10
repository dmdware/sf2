











#include "unit.h"
#include "../render/shader.h"
#include "mvtype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "umove.h"
#include "../render/transaction.h"
#include "simdef.h"
#include "simflow.h"
#include "../phys/trace.h"
#include "../phys/collision.h"
#include "../path/collidertile.h"
#include "../path/pathdebug.h"
#include "../sim/bltype.h"
#include "../sim/building.h"
#include "../path/jpspath.h"
#include "../path/jpspartpath.h"
#include "../path/pathnode.h"
#include "../path/partialpath.h"
#include "../path/mazepath.h"
#include "labourer.h"
#include "../math/fixmath.h"
#include "build.h"
#include "../path/pathjob.h"
#include "../path/tilepath.h"
#include "../path/astarpath.h"
#include "../path/anypath.h"
#include "../path/fillbodies.h"
#include "../render/fogofwar.h"
#include "border.h"
#include "../gui/layouts/messbox.h"
#include "../math/vec3f.h"

//not engine
#include "../gui/layouts/chattext.h"

//TODO circular unit radius
ecbool UnitCollides(Mv* mv, Vec2i cmpos, int mvtype)
{
	MvType* t = &g_mvtype[mvtype];
	int cmminx = cmpos.x - t->size.x/2;
	int cmminy = cmpos.y - t->size.x/2;
	int cmmaxx = cmminx + t->size.x - 1;
	int cmmaxy = cmminy + t->size.x - 1;
	
	if(cmminx < 0 || cmminy < 0 || cmmaxx >= g_mapsz.x * TILE_SIZE || cmmaxy >= g_mapsz.y * TILE_SIZE)
		return ectrue;

	//int ui = u-g_mv;
	//if(((g_simframe+ui)%1001!=1))
	//	return ecfalse;

	int cx = cmpos.x / PATHNODE_SIZE;
	int cy = cmpos.y / PATHNODE_SIZE;

#if 1
	//new

	//might still collide becuase in Standable there is
#if 0
	if(roaded)
	{
		CdTile* ctile = GetCd(CD_ROAD,
			nx / (TILE_SIZE/PATHNODE_SIZE),
			ny / (TILE_SIZE/PATHNODE_SIZE),
			ecfalse);

		return ctile->on && ctile->finished;
	}
#endif

	//so.... better check

	if(t->roaded)
	{
		int pathoff = PathOff(t->size.x);

		CdTile* cdtile = GetCd(CD_ROAD, 
			(cmpos.x /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			(cmpos.y /* /PATHNODE_SIZE*PATHNODE_SIZE */ - pathoff)/PATHNODE_SIZE / (TILE_SIZE/PATHNODE_SIZE), 
			ecfalse);

		if(!cdtile->on || !cdtile->finished)
			return ectrue;
	}
#else	//old check...

	//ColliderTile* cell = ColliderAt(cx, cy);

	//if(!t->seaborne && !(cell->flags & FLAG_HASLAND))
	//	return ectrue;

#if 0
	if(t->roaded && !(cell->flags & FLAG_HASROAD))
		return ectrue;
#else
	if(t->roaded)
	{
		CdTile* cdtile = GetCd(CD_ROAD, cmpos.x / TILE_SIZE, cmpos.y / TILE_SIZE, ecfalse);

		if(!cdtile->on || !cdtile->finished)
			return ectrue;
	}
#endif

	
#endif

	//if(cell->flags & FLAG_ABRUPT)
	//	return ectrue;

	int nminx = cmminx / PATHNODE_SIZE;
	int nminy = cmminy / PATHNODE_SIZE;
	int nmaxx = cmmaxx / PATHNODE_SIZE;
	int nmaxy = cmmaxy / PATHNODE_SIZE;

	int uin = -1;
	
	if(u)
		uin = u - g_mv;

	for(int x=nminx; x<=nmaxx; x++)
		for(int y=nminy; y<=nmaxy; y++)
		{
			int ni = PATHNODEINDEX(x, y);
			//cell = ColliderAt(x, y);

#if 0
			if(cell->flags & FLAG_ABRUPT)
				return ectrue;	//corpc fix

			if(cell->foliage != USHRT_MAX)
				return ectrue;

			if(cell->building >= 0)
			{
				Bl* b = &g_bl[cell->building];
				BlType* t2 = &g_bltype[b->type];

				int tminx = b->tpos.x - t2->width.x/2;
				int tminy = b->tpos.y - t2->width.y/2;
				int tmaxx = tminx + t2->width.x;
				int tmaxz = tminy + t2->width.y;

				int minx2 = tminx*TILE_SIZE;
				int miny2 = tminy*TILE_SIZE;
				int maxx2 = tmaxx*TILE_SIZE - 1;
				int maxy2 = tmaxz*TILE_SIZE - 1;

				if(cmminx <= maxx2 && cmminy <= maxy2 && cmmaxx >= minx2 && cmmaxy >= miny2)
				{


					return ectrue;
				}
			}
			
			if(cell->unit >= 0 && 
				cell->unit != uin && 
				!g_mv[cell->unit].hidden())
				return ectrue;
#else
			//if (g_collider.on(ni))
			PathNode* n = g_pathnode + ni;
			if(n->flags & PATHNODE_BLOCKED)
				return ectrue;
#endif

		}

	return ecfalse;
}

void CheckPath(Mv* mv)
{
	MvType* t = &g_mvtype[mv->type];
	
#ifdef RANDOM8DEBUG
	if(u - g_mv == thatunit)
	{
		Log("unitmove u=thatunit dgoal="<<(mv->goal.x-mv->cmpos.x)<<","<<(mv->goal.y-mv->cmpos.y)<<" ");
		Log("unitmove u=thatunit dsubgoal="<<(mv->subgoal.x-mv->cmpos.x)<<","<<(mv->subgoal.y-mv->cmpos.y)<<" ");
	}
#endif

	//Signs that we need a new path
	if( /*mv->cmpos != mv->goal && */	//corpd fix
	   ( mv->path.size() <= 0 ||
		(*mv->path.rbegin() != mv->goal
#ifdef HIERPATH
		&&
			(mv->tpath.size() == 0 //||
			//*mv->tpath.rbegin() != Vec2s(mv->goal.x/TILE_SIZE,mv->goal.y/TILE_SIZE)	//doesn't work for corner-placed conduits
			//let's hope tpath will always be reset when new path is needed.
			)
#endif
			) ) )
	{
		
#if 0
			if(u - g_mv == 205)
			{
				InfoMess("t","t");
			}
#endif

#if 1
		if(t->military)
		//if(1)
		{
			if(g_simframe - mv->lastpath < mv->pathdelay)
			{
				return;
			}

			//mv->pathdelay += 1;
			//mv->pathdelay *= 2;
			//mv->pathdelay += 50;
			mv->pathdelay = (mv->pathdelay + 50) % PATH_DELAY;
			mv->lastpath = g_simframe;

#if 1
			const Vec2i noff = (mv->goal - mv->cmpos) / PATHNODE_SIZE;
			int nodesdist = PATHHEUR( noff );

			PartialPath(mv->type, mv->mode,
			            mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
						&mv->path, &mv->subgoal,
			            u, NULL, NULL,
			            mv->goal.x, mv->goal.y,
			            mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
			            //nodesdist*nodesdist*1);
			TILE_SIZE*4/PATHNODE_SIZE, ecfalse, ectrue);	//enough to move 3 tiles around a corner, filled with obstacles

#if 0
			if(mv->path.size() <= 0)
			{
				mv->path.push_back( mv->goal );
				mv->subgoal = mv->goal;
			}
#endif
#elif 1
			mv->path.clear();
			mv->path.push_back( mv->goal );
			mv->subgoal = mv->goal;
#elif 0
			JPSPath(mv->type, mv->mode,
			            mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, &mv->path, &mv->subgoal,
			            u, NULL, NULL,
			            mv->goal.x, mv->goal.y,
			            mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y);
#else
			JPSPartPath(mv->type, mv->mode,
			            mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, &mv->path, &mv->subgoal,
			            u, NULL, NULL,
			            mv->goal.x, mv->goal.y,
			            mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
			            nodesdist*4);
#endif

#if 0
			RichText rtext("ppathf");
			NewTransx(mv->cmpos + Vec3f(0,t->size.y,0), &rtext);
#endif
		}
		//else if not military
		else //if(!mv->pathblocked)
#endif
		{

			if(g_simframe - mv->lastpath < mv->pathdelay)
			{
				return;
			}

			
#if 0
			if(u - g_mv == 205)
			{
				InfoMess("t","t");
			}
#endif

			//mv->pathdelay += 1;
			//mv->pathdelay *= 2;
			//mv->pathdelay += 10;
			mv->pathdelay = (mv->pathdelay + PATH_DELAY/3) % PATH_DELAY;
			mv->lastpath = g_simframe;
			
			//if(Trapped()

			///MvType* mvt = &g_mvtype[mv->type];
			//int nodesdist = PATHHEUR( (mv->goal - mv->cmpos) / PATHNODE_SIZE );
			///int nodesdist = MAXPATHN * mvt->cmspeed;

#ifdef HIERPATH
			if(mv->tpath.size() == 0 // ||
				//*mv->tpath.rbegin() != Vec2s(mv->goal.x/TILE_SIZE,mv->goal.y/TILE_SIZE)	//not ectrue for conduits on tile corners
					)
			{
				mv->path.clear();
				mv->tpath.clear();
				TilePath(mv->type, mv->mode,
					mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
					&mv->tpath,
					u, NULL, NULL,
					mv->goal.x, mv->goal.y,
					mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,	//corpd fix
					//10000
					MAX_U_TPATH);

#if 0
				if(mv->tpath.size() == 0 && t->size.x > 60)
					TilePath(mv->type, mv->mode,
					mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
					&mv->tpath,
					u, NULL, NULL,
					mv->goal.x, mv->goal.y,
					mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,	//corpd fix
					10000,
					ectrue);
#endif

#if 0
				if(u - g_mv == 36)
				{
					char msg[128];
					sprintf(msg, "u36 trepath goal=%d,%d tpath.sz=%d", mv->goal.x, mv->goal.y, (int)mv->tpath.size());
					RichText rm = RichText(msg);
					AddNotif(&rm);
				}
#endif

				//RichText rm = RichText("CONSTANTLY REPATHING");
				//AddNotif(&rm);
				//if(mv->tpath.size() == 0)
				//	InfoMess("tpf", "tpf");
				//	AddNotif(&RichText("STILL NO PATH"));
			}
#endif

#if 0
			if(!FullPath(0,
				mv->type, mv->mode,
				mv->cmpos.x, mv->cmpos.y, mv->target, mv->target, mv->target2, mv->path, mv->subgoal,
				u, NULL, NULL,
				mv->goal.x, mv->goal.y,
				mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y))
#elif !defined(HIERPATH)
#if 0
			if(AnyPath(mv->type, mv->mode,
				mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
				u, NULL, NULL,
				mv->goal.x, mv->goal.y,
				mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1))
#endif
#if 0
				JPSPath(mv->type, mv->mode,
				mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
				&mv->path, &mv->subgoal,
				u, NULL, NULL,
				mv->goal.x, mv->goal.y,
				mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
				0, 0, g_pathdim.x-1, g_pathdim.y-1, ecfalse, ectrue);
#elif 0
				AStarPath(mv->type, mv->mode,
				mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
				&mv->path, &mv->subgoal,
				u, NULL, NULL,
				mv->goal.x, mv->goal.y,
				mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
				100000,
				0, 0, g_pathdim.x-1, g_pathdim.y-1, ecfalse, ectrue);
#else
				MvType* mvt = &g_mvtype[mv->type];
			//int nodesdist = PATHHEUR( (mv->goal - mv->cmpos) / PATHNODE_SIZE ) * 1 * TILE_SIZE / PATHNODE_SIZE;
			//int nodesdist = MAXPATHN + MAXPATHN * (g_simframe % 30 == 0) + MAXPATHN * (g_simframe % 20 == 0) /* * mvt->cmspeed */;
			int nodesdist = MAXPATHN;

			//if ((u - g_mv) == 457)
			//	InfoMess("p", "o");

			//To avoid slow-downs.
			//MazePath(mv->type, mv->mode,
			PartialPath(mv->type, mv->mode,
				mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
				&mv->path, &mv->subgoal,
				u, NULL, NULL,
				mv->goal.x, mv->goal.y,
				mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
				//nodesdist*nodesdist*1,
				//TILE_SIZE*TILE_SIZE*20/PATHNODE_SIZE/PATHNODE_SIZE, 
				/*nodesdist*/ TILE_SIZE*1/PATHNODE_SIZE,
				ectrue, /*ectrue*/ ectrue);
			//Enough to move 19 tiles around a corner, filled with obstacles.

			
			//2016/05/04 trucks without possible path now give up
			if(!mv->path.size() &&
				!((g_simframe / 3) % 20) )
			{
				//AddChat(&RichText("ga"));
				//AddNotif(&RichText("ga"));
				ResetMode(u);
			}

#if 0
			if(u - g_mv == 205)
			{
				ecbool f = mv->path.size() > 0;

				char m[123];
				sprintf(m, "f %d", (int)f);
				InfoMess(m,m);
			}

#endif
			//if ((u - g_mv) == 457)
			{
			//	if(mv->path.size())
			//		InfoMess("p", "ss");
			//	else
			//		InfoMess("p", "ff");
			}
#endif
#else	//else def HIERPATH
			if(mv->tpath.size() > 0)
			{
#ifdef HIERDEBUG
				g_applog<<"tp sz > 0"<<std::endl;
#endif

				Vec2s tpos = *mv->tpath.begin();

#ifdef HIERDEBUG
				g_applog<<"tpos pop "<<tpos.x<<","<<tpos.y<<std::endl;
#endif

				unsigned int cmtminx = tpos.x * TILE_SIZE;
				unsigned int cmtminy = tpos.y * TILE_SIZE;
				unsigned int cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
				unsigned int cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

				MvType* mvt = &g_mvtype[ mv->type ];

				unsigned int ucmminx = mv->cmpos.x - mvt->size.x/2;
				unsigned int ucmminy = mv->cmpos.y - mvt->size.x/2;
				unsigned int ucmmaxx = ucmminx + mvt->size.x - 1;
				unsigned int ucmmaxy = ucmminy + mvt->size.x - 1;

#ifdef HIERDEBUG
				g_applog<<"\tucmminx <= cmtmaxx = "<<(ucmminx <= cmtmaxx)<<std::endl;
				g_applog<<"\tucmminy <= cmtmaxy = "<<(ucmminy <= cmtmaxy)<<std::endl;
				g_applog<<"\tucmmaxx >= cmtminx = "<<(ucmmaxx >= cmtminx)<<std::endl;
				g_applog<<"\tucmmaxy >= cmtminy = "<<(ucmmaxy >= cmtminy)<<std::endl;
#endif
				

				if(/* mv->cmpos/TILE_SIZE == Vec2i(tpos.x,tpos.y) */
					ucmminx <= cmtmaxx &&
					ucmminy <= cmtmaxy &&
					ucmmaxx >= cmtminx &&
					ucmmaxy >= cmtminy)
				{
					mv->tpath.erase( mv->tpath.begin() );

					if(mv->tpath.size() > 0)
					{
#ifdef HIERDEBUG
						g_applog<<"start in tpos first"<<std::endl;
#endif

						mv->tpath.erase( mv->tpath.begin() );
						//tpos = *mv->tpath.begin();

						//Next tile goal bounds
						tpos = *mv->tpath.begin();

#ifdef HIERDEBUG
						g_applog<<"tpos pop next "<<tpos.x<<","<<tpos.y<<std::endl;
#endif

						cmtminx = tpos.x * TILE_SIZE;
						cmtminy = tpos.y * TILE_SIZE;
						cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
						cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

						Vec2i cmsubgoal = Vec2i(tpos.x,tpos.y) * TILE_SIZE + Vec2i(1,1)*TILE_SIZE/2;	//corpd fix xp

						//Bounds around current unit cmpos
						tpos = Vec2s( mv->cmpos.x / TILE_SIZE, mv->cmpos.y / TILE_SIZE );

#define IN_PATHNODES (TILE_SIZE/PATHNODE_SIZE)

#if 0
						unsigned short nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1);
						unsigned short nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1);
						unsigned short nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1);
						unsigned short nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1);
#elif 1
						unsigned short nminx = imax(0, (tpos.x) * IN_PATHNODES - 1);
						unsigned short nminy = imax(0, (tpos.y) * IN_PATHNODES - 1);
						unsigned short nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES);
						unsigned short nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES);
#else
						unsigned short nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1 - mvt->size.x/PATHNODE_SIZE/2);
						unsigned short nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1 - mvt->size.x/PATHNODE_SIZE/2);
						unsigned short nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1 + mvt->size.x/PATHNODE_SIZE/2);
						unsigned short nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1 + mvt->size.x/PATHNODE_SIZE/2);
#endif

#undef IN_PATHNODES

#if 0
						JPSPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							mv->goal.x, mv->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							nminx, nminy, nmaxx, nmaxy, ectrue, ecfalse);
#elif 0
						AStarPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							mv->goal.x, mv->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							100000,
							nminx, nminy, nmaxx, nmaxy, ectrue, ecfalse);
#else
						//Better to use PartialPath, setting a node search limit,
						//instead of bounds, so it's able to get around corners.
						PartialPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							cmsubgoal.x, cmsubgoal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							//nodesdist*nodesdist*1);
							TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, ecfalse, ecfalse,
							ecfalse);
#endif
					}
				}
				//else not yet at next tile pos bounds/goal
				else
				{
#ifdef HIERDEBUG
					g_applog<<"not start in pop tpath, mv->tp.sz()="<<mv->tpath.size()<<std::endl;
#endif
					tpos = *mv->tpath.begin();

					//Next tile goal bounds
					cmtminx = tpos.x * TILE_SIZE;
					cmtminy = tpos.y * TILE_SIZE;
					cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
					cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

					Vec2i cmsubgoal = Vec2i(tpos.x,tpos.y) * TILE_SIZE + Vec2i(1,1)*TILE_SIZE/2;	//corpd fix xp

#if 0
					char cmfail[128];
					sprintf(cmfail, "cm %d,%d->%d,%d", cmtminx, cmtminy, cmtmaxx, cmtmaxy);
					RichText cmfailr = RichText(cmfail);
					AddNotif(&cmfailr);
#endif

					//Bounds around current unit cmpos
					tpos = Vec2s( mv->cmpos.x / TILE_SIZE, mv->cmpos.y / TILE_SIZE );

#define IN_PATHNODES (TILE_SIZE/PATHNODE_SIZE)

					unsigned short nminx = imax(0, (tpos.x-1) * IN_PATHNODES - 1);
					unsigned short nminy = imax(0, (tpos.y-1) * IN_PATHNODES - 1);
					unsigned short nmaxx = imin(g_pathdim.x, (tpos.x+1) * IN_PATHNODES + 1);
					unsigned short nmaxy = imin(g_pathdim.y, (tpos.y+1) * IN_PATHNODES + 1);

#undef IN_PATHNODES

					//Need to append cmgoal to path, but also bound JPS search
					if(mv->tpath.size() <= 1)
#if 0
						JPSPath(mv->type, mv->mode,
						mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
						&mv->path, &mv->subgoal,
						u, NULL, NULL,
						mv->goal.x, mv->goal.y,
						mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
						nminx, nminy, nmaxx, nmaxy, ectrue, ectrue);
#elif 0
						AStarPath(mv->type, mv->mode,
						mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
						&mv->path, &mv->subgoal,
						u, NULL, NULL,
						mv->goal.x, mv->goal.y,
						mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
						100000,
						nminx, nminy, nmaxx, nmaxy, ectrue, ectrue);
#else

						PartialPath(mv->type, mv->mode,
						mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
						&mv->path, &mv->subgoal,
						u, NULL, NULL,
						mv->goal.x, mv->goal.y,
						mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
						//nodesdist*nodesdist*1);
						TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, ectrue, ecfalse, ectrue);
#endif
					else
					{
#if 0
						JPSPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							mv->goal.x, mv->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							nminx, nminy, nmaxx, nmaxy, ectrue, ecfalse);
						elif 0
							AStarPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							mv->goal.x, mv->goal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							100000,
							nminx, nminy, nmaxx, nmaxy, ectrue, ecfalse);
#else
						/*
						it's okay now to return partial path and that will not increase existing clumps
						as as soon as the unit can now longer move closer, he will have path.size()=0 
						and move randomly, then if he collides, his path will reset, and there won't 
						be a tpath. right?
						oh, since we're capping the end with the cmsubgoal (tile center for next tile goal),
						that possibly causing some inability to get a path since we demand no partials.
						actually, the opposite is the problem; we don't cap the end when we're right at
						the tile edge, so we get size=0 path, right?
						the solution is not to cap the end, since that would lead to pathing to tile centers,
						but to set the cm tile bounds such that the unit is fully inside the tile.
						no, it shouldn't return size=0 path if there's a starting node.
						*/

#if 0
						cmtminx += mvt->size.x;
						cmtminy += mvt->size.x;
						cmtmaxx -= mvt->size.x;
						cmtmaxy -= mvt->size.x;
#elif 0
						cmtminx = cmtminx + mvt->size.x/2;
						cmtminy = cmtminy + mvt->size.x/2;
						cmtmaxx = cmtmaxx - mvt->size.x/2;
						cmtmaxy = cmtmaxy - mvt->size.x/2;
#endif

						//Better to use PartialPath, setting a node search limit,
						//instead of bounds, so it's able to get around corners.
						PartialPath(mv->type, mv->mode,
							mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype, mv->supplier,
							&mv->path, &mv->subgoal,
							u, NULL, NULL,
							//mv->goal.x, mv->goal.y,	//corpd fix
							cmsubgoal.x, cmsubgoal.y,
							cmtminx, cmtminy, cmtmaxx, cmtmaxy,
							//nodesdist*nodesdist*1);
							TILE_SIZE*TILE_SIZE*3/PATHNODE_SIZE/PATHNODE_SIZE, ecfalse, ecfalse /*ectrue*/,
							ecfalse);
						//Enough to move 2 tiles around a corner, filled with obstacles.
#endif
					}

#if 0
					PartialPath(mv->type, mv->mode,
						mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
						&mv->path, &mv->subgoal,
						u, NULL, NULL,
						mv->goal.x, mv->goal.y,
						mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
						//nodesdist*nodesdist*1,
						//TILE_SIZE*TILE_SIZE*20/PATHNODE_SIZE/PATHNODE_SIZE, 
						nodesdist,
						ectrue, /*ectrue*/ ecfalse);
#endif
#if 0
					if(mv->path.size() <= 0)
					{
						RichText failed = RichText("failed 2");
						AddNotif(&failed);
					}
#endif

				}

#ifdef HIERDEBUG
				g_applog<<"s "<<mv->cmpos.x<<","<<mv->cmpos.y<<" (t "<<(mv->cmpos.x/TILE_SIZE)<<","<<(mv->cmpos.y/TILE_SIZE)<<")"<<std::endl;

				g_applog<<"ps "<<mv->path.size()<<"tps = "<<mv->tpath.size()<<std::endl;

				g_applog<<"g "<<mv->goal.x<<","<<mv->goal.y<<" (t "<<(mv->goal.x/TILE_SIZE)<<","<<(mv->goal.y/TILE_SIZE)<<")"<<std::endl;

				for(std::list<Widget*>::iterator pit=mv->path.begin(); pit!=mv->path.end(); pit++)
				{
					g_applog<<"p "<<pit->x<<","<<pit->y<<" (t "<<(pit->x/TILE_SIZE)<<","<<(pit->y/TILE_SIZE)<<")"<<std::endl;
				}

				for(std::list<Widget*>::iterator tit=mv->tpath.begin(); tit!=mv->tpath.end(); tit++)
				{
					g_applog<<"t "<<tit->x<<","<<tit->y<<std::endl;
				}

				if(mv->path.size() == 0)
				{
					g_applog<<"not found subpath ----"<<std::endl;
				}
				else
					g_applog<<"did find subpath ----"<<std::endl;
#endif

#if 0
				//Try a full path
				if(mv->path.size() <= 0)
				{
					JPSPath(mv->type, mv->mode,
						mv->cmpos.x, mv->cmpos.y,
						mv->target, mv->target2, mv->targtype, mv->cdtype,
						&mv->path, &mv->subgoal,
						u, NULL, NULL,
						mv->goal.x, mv->goal.y,
						mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
						0, 0, g_pathdim.x-1, g_pathdim.y-1,
						ecfalse, ectrue);

					if(mv->path.size() > 0)
					{
						//Necessary for it to not reset path upon
						//arriving at next tile node?
						mv->tpath.clear();
						//mv->tpath.push_back(Vec2s(mv->goal.x/TILE_SIZE, mv->goal.y/TILE_SIZE));
					}
				}
#endif

#if 1	/* this is not a jam, this happens normally during the course of following a tpath
				but having turned it off, I noticed that lag increased as workers weren't as spaced out as before.
				so let's set jams=4 for this unit, and check for that condition specifically to avoid.
				edit: oh well, a small price to pay (only 90fps) for not having the workers die off from inefficient movement.
				edit: oh, it was just because I wasn't checking for tpath.sz>0.
				*/
				//check for jams
				if(mv->tpath.size() > 0 && mv->path.size() <= 0)
				{
					Jamify(u);
					//ResetMode(u);	//TODO works?
					//mv->pathblocked = ectrue;
				}
#endif
			}
			//else after TilePath() tpath.size() == 0
			else
			{
				//Jamify(u);
				//ResetMode(u);	//TODO works?
				//mv->pathblocked = ectrue;
			}
#endif	//end if def HIERPATH

#if 0
			//causes clumps, as mv gather around a bl that might have all entrances blocked off, 
			//but unit will move closer to goal in the meanwhile while there might be a blockage
			if(mv->path.size() <= 0)
			{
				int nodesdist = PATHHEUR( (mv->goal - mv->cmpos) / PATHNODE_SIZE );

				PartialPath(mv->type, mv->mode,
					mv->cmpos.x, mv->cmpos.y, mv->target, mv->target2, mv->targtype, mv->cdtype,
					&mv->path, &mv->subgoal,
					u, NULL, NULL,
					mv->goal.x, mv->goal.y,
					mv->goal.x, mv->goal.y, mv->goal.x, mv->goal.y,
					nodesdist*nodesdist*10, ectrue, ectrue);

				mv->pathblocked = ectrue;
			}
			else
				mv->pathblocked = ecfalse;
#endif

//#ifdef HIERPATH
#if 1	//trucks don't work for some reason
			//TODO need to check for out of map bounds?
			//induce random walking to unclump from the clumped area
			//and just to make it look like the unit doesn't know where to go
			if(mv->path.size() <= 0  && 
				g_mvtype[mv->type].walker  )
			{
				//mv->path.push_back( mv->goal );
				//PlaceUAb(mv->type, mv->cmpos, &mv->subgoal);
				//mv->path.push_back( mv->subgoal );
				//mv->subgoal = mv->goal;

				//set it in a new direction (out of 8 directions) every 3 simframes
				//unsigned int timeoff = (g_simframe % (8 * 3)) / 3;
				unsigned int timeoff = ( (g_simframe + (u-g_mv)) % (8 * 3)) / 3;
				//don't set goal because that would throw off the real target goal
				mv->subgoal /*= mv->goal*/ = mv->cmpos + Vec2i(OFFSETS[timeoff].x,OFFSETS[timeoff].y) * TILE_SIZE / 2;
				//follow the goal after subgoal so that we bump into something and repath automatically
				mv->path.push_back( mv->goal );

				mv->pathdelay += PATH_DELAY * 2;

#ifdef RANDOM8DEBUG
				if(u - g_mv == thatunit)
				{
					g_applog<<"unitmove u=thatunit random8dir"<<std::endl;
				}
#endif
			}
#endif
//#endif
		}

		//goto endcheck;

		return;
	}
}

void MoveUnit(Mv* mv)
{
	MvType* t = &g_mvtype[mv->type];
	mv->prevpos = mv->cmpos;
	mv->collided = ecfalse;

	if(mv->threadwait)
		return;

	if(mv->type == MV_TRUCK && mv->driver < 0)
		return;

	//Needed so that with large MAXPATHN, we don't spend a lot of time
	//on unreachable paths in a crowd for labourers at the beginning of new game
	if( /* mv->mode == UMODE_NONE && */
	   mv->cmpos == mv->goal &&
	   mv->cmpos == mv->subgoal )	//corpd fix
	{
		//RichText mess = RichText("goal-cmpos < speed");
		//AddNotif(&mess);
		return;
	}

	Vec2i goaloff = mv->goal - mv->cmpos;

	if(mv->underorder && mv->target < 0 && DOT_VEC2I(goaloff) <= PATHNODE_SIZE*PATHNODE_SIZE)
		return;

	CheckPath(u);

	//return;	//temp

	mv->freecollider();
	RemVis(u);

	Vec2i dir = mv->subgoal - mv->cmpos;

	//Arrived at subgoal?
	//if(Dot(mv->subgoal - mv->cmpos) <= t->cmspeed * t->cmspeed)
	if(iabs(dir.x) <= t->cmspeed &&	//necesssary extra checks to make sure dir doesn't overflow when computing Dot (dot product).
		iabs(dir.y) <= t->cmspeed &&
		DOT_VEC2I(dir) <= t->cmspeed * t->cmspeed)
	{
		//TODO put this and below copy in UpDraw?
		//mv->rotation.z = GetYaw(dir.x, dir.y);
		//mv->rotation.z = atanf( (float)dir.x / (float)dir.y );
		mv->rotation.z = atan2( (float)dir.x, (float)dir.y );
		CalcRot(u);

		//PlayAnim(mv->frame[BODY_LOWER], 0, t->nframes, ectrue, 1.0f);

		mv->cmpos = mv->subgoal;

		//dir = Vec2i(0, 0);	//fixed; no overreach collision now.

		if(mv->path.size() >= 2)
		{
			mv->path.erase( mv->path.begin() );
			mv->subgoal = *mv->path.begin();
			dir = mv->subgoal - mv->cmpos;
		}
		else
		{
#ifdef HIERPATH
			//Did we finish the local path? Have tpath?
			if(mv->tpath.size() > 0)
			{

				Vec2s tpos = *mv->tpath.begin();

				unsigned int cmtminx = tpos.x * TILE_SIZE;
				unsigned int cmtminy = tpos.y * TILE_SIZE;
				unsigned int cmtmaxx = (tpos.x+1) * TILE_SIZE - 1;
				unsigned int cmtmaxy = (tpos.y+1) * TILE_SIZE - 1;

				MvType* mvt = &g_mvtype[ mv->type ];

				unsigned int ucmminx = mv->cmpos.x - mvt->size.x/2;
				unsigned int ucmminy = mv->cmpos.y - mvt->size.x/2;
				unsigned int ucmmaxx = ucmminx + mvt->size.x - 1;
				unsigned int ucmmaxy = ucmminy + mvt->size.x - 1;

				if(/* mv->cmpos/TILE_SIZE == Vec2i(tpos.x,tpos.y) */
					ucmminx <= cmtmaxx &&
					ucmminy <= cmtmaxy &&
					ucmmaxx >= cmtminx &&
					ucmmaxy >= cmtminy &&
					mv->tpath.size() > 1)
				{
					mv->path.clear();

					if(*mv->tpath.begin() == Vec2s(mv->cmpos.x/TILE_SIZE,mv->cmpos.y/TILE_SIZE))
						mv->tpath.erase( mv->tpath.begin() );
					//Advance
				}
				else
				{
					ResetPath(u);
				}
			}
			//necessary so mv can get a real path if
			//this was just a temp partial or random 8 move
			else
#endif
			{
				ResetPath(u);
			}
		}
	}
	//corpd fix 2016
	else if(dir.x != 0 || dir.y != 0)
	//if(mag > t->cmspeed)
	{
		//mv->rotation.z = GetYaw(dir.x, dir.y);
		//mv->rotation.z = atanf( (float)dir.x / (float)dir.y );
		mv->rotation.z = atan2( (float)dir.x, (float)dir.y );
		CalcRot(u);
		//PlayAnim(mv->frame[BODY_LOWER], 0, t->nframes, ectrue, 1.0f);

		//actually, instead of all this bit shifting nonsense, why not do this?
		int maxdelta = 1 << 12;
		//get absolute value and see if it exceeds max value
		//we're assuming dir.x can't equal INT_MIN, because that can't give a positive int value.
		if(iabs(dir.x) >= maxdelta ||
			iabs(dir.y) >= maxdelta)
		{
			//pick the max element and divide by it (or something something a bit smaller than it.
			//some multiple smaller so we can still have something when we divide the coordinates by it.)
			int dirmax = imax(iabs(dir.x), iabs(dir.y));
			//sign(dir.x);
			//int hmax = dirmax / 2;	//half of max element
			//int qmax = dirmax >> 2;	//quarter of max element
			int dmax = dirmax >> 8;	//max divided by
			dir.x /= dmax;
			dir.y /= dmax;
			//dividing a vector's components by the same number gives a vector pointing in the same direction but smaller.
			//dividing by (max/256) gives us a maximum vector distance of 256 in the case that the line is straight/axial.
		}

		int mag = MAG_VEC2I(dir);

		if(mag <= 0)
		{
			mag = 1;
		}
		
		Vec2i speeddir = dir * t->cmspeed / mag;

		//Can result in overshooting the target otherwise,
		//and being a few centimeters in collision with building,
		//leading to forever being unable to follow the path.
		if(t->cmspeed >= mag)
			speeddir = dir;

		mv->cmpos = mv->cmpos + speeddir;
	}

	//corpc fix, no more sublevel braces; jumping to subgoal and dir!=0 are both moves that can have collisions.
	if(UnitCollides(u, mv->cmpos, mv->type))
	{
		ecbool ar = CheckIfArrived(u);
		
//	if(u - g_mv == 182 && g_simframe > 118500)
	//	Log("f2");

		//if(ar //|| 
		//(g_simframe+(u-g_mv))%1001==1
		//)
		{
			mv->collided = ectrue;
			mv->cmpos = mv->prevpos;
			mv->path.clear();
			mv->tpath.clear();
			mv->subgoal = mv->cmpos;
			mv->fillcollider();
			AddVis(u);
			Explore(u);
			UpDraw(u);
			ResetPath(u);
		}

		if(ar)
			OnArrived(u);
		
	//	if((g_simframe+(u-g_mv))%1001==1)
		return;
	}

	if(!mv->hidden())
	{
		
	//if(u - g_mv == 182 && g_simframe > 118500)
	//	Log("f3");

		mv->fillcollider();
		AddVis(u);
		Explore(u);
		UpDraw(u);

		//if(t->military)
		if(mv->type != MV_TRUCK)
			MarkTerr(u);
	}

	mv->drawpos.x = mv->cmpos.x;
	mv->drawpos.y = mv->cmpos.y;
//	mv->drawpos.z = g_hmap.accheight2(mv->cmpos.x, mv->cmpos.y);
}

ecbool CheckIfArrived(Mv* mv)
{
	MvType* mvt = &g_mvtype[mv->type];
	int ui = (int)(u-g_mv);

	int ucmminx = mv->cmpos.x - mvt->size.x/2;
	int ucmminy = mv->cmpos.y - mvt->size.x/2;
	int ucmmaxx = ucmminx + mvt->size.x - 1;
	int ucmmaxy = ucmminy + mvt->size.x - 1;

	//switched from fine-grained collisions to pathnode check
#if 1
	Bl* b;
	BlType* bt;
	CdType* ct;
	CdTile* ctile;
	Mv* u2;
	MvType* u2t;

	int btminx;
	int btminy;
	int btmaxx;
	int btmaxy;
	int bcmminx;
	int bcmminy;
	int bcmmaxx;
	int bcmmaxy;
	int ccmposx;
	int ccmposz;
	int ccmminx;
	int ccmminy;
	int ccmmaxx;
	int ccmmaxy;
	int u2cmminx;
	int u2cmminy;
	int u2cmmaxx;
	int u2cmmaxy;

	switch(mv->mode)
	{
#if 1
	case UMODE_GOBLJOB:
	case UMODE_GOCSTJOB:
	case UMODE_GOSHOP:
	case UMODE_GOREST:
	case UMODE_GODEMB:
		b = &g_bl[mv->target];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		//btmaxx = btminx + bt->width.x - 1;
		//btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
		{
			//char msg[1280];
			//sprintf(msg, "arr ui=%d u(%d,%d) b(%d,%d) %d<=%d && %d<=%d && %d<=%d && %d<=%d", ui, mv->cmpos.x/TILE_SIZE, mv->cmpos.y/TILE_SIZE, b->tpos.x, b->tpos.y, ucmminx, bcmmaxx, ucmminy, bcmmaxy, bcmminx, ucmmaxx, bcmminy, ucmmaxy);
			//RichText at("arr");
			//RichText at(msg);
			//AddNotif(&at);
			return ectrue;
		}
		break;
#endif
	case UMODE_GOCDJOB:
	case UMODE_GODEMCD:
		ct = &g_cdtype[mv->cdtype];
		ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);

		ccmposx = mv->target*TILE_SIZE + ct->physoff.x;
		ccmposz = mv->target2*TILE_SIZE + ct->physoff.y;

		ccmminx = ccmposx - TILE_SIZE/2;
		ccmminy = ccmposz - TILE_SIZE/2;
		ccmmaxx = ccmminx + TILE_SIZE;
		ccmmaxy = ccmminy + TILE_SIZE;

		if(ucmminx <= ccmmaxx && ucmminy <= ccmmaxy && ccmminx <= ucmmaxx && ccmminy <= ucmmaxy)
			return ectrue;
		break;
#if 1
	case UMODE_GOSUP:
		b = &g_bl[mv->supplier];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		//btmaxx = btminx + bt->width.x - 1;
		//btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return ectrue;
		break;

	case UMODE_GOREFUEL:
		b = &g_bl[mv->fuelstation];
		bt = &g_bltype[b->type];

		btminx = b->tpos.x - bt->width.x/2;
		btminy = b->tpos.y - bt->width.y/2;
		btmaxx = btminx + bt->width.x - 1;
		btmaxy = btminy + bt->width.y - 1;

		bcmminx = btminx * TILE_SIZE;
		bcmminy = btminy * TILE_SIZE;
		bcmmaxx = bcmminx + bt->width.x*TILE_SIZE - 1;
		bcmmaxy = bcmminy + bt->width.y*TILE_SIZE - 1;

		if(ucmminx <= bcmmaxx && ucmminy <= bcmmaxy && bcmminx <= ucmmaxx && bcmminy <= ucmmaxy)
			return ectrue;
		break;

	case UMODE_GODRIVE:
		u2 = &g_mv[mv->target];
		u2t = &g_mvtype[u2->type];

		u2cmminx = u2->cmpos.x - u2t->size.x/2;
		u2cmminy = u2->cmpos.y - u2t->size.x/2;
		u2cmmaxx = u2cmminx + u2t->size.x - 1;
		u2cmmaxy = u2cmminy + u2t->size.x - 1;
		
		ucmminx = ucmminx / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmminy = ucmminy / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmmaxx = ucmmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		ucmmaxy = ucmmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
		
		u2cmminx = u2cmminx / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmminy = u2cmminy / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmmaxx = u2cmmaxx / PATHNODE_SIZE * PATHNODE_SIZE;
		u2cmmaxy = u2cmmaxy / PATHNODE_SIZE * PATHNODE_SIZE;
		//TODO correct in all cases?
		//InfoMess("dr","d?");

		//AddNotif(&RichText("arrived tr----------"));

		if(ucmminx <= u2cmmaxx && ucmminy <= u2cmmaxy && u2cmminx <= ucmmaxx && u2cmminy <= ucmmaxy)
			return ectrue;
		break;
#endif
	default:
		break;
	};

#else

	const int unminx = ucmminx/PATHNODE_SIZE;
	const int unminz = ucmminy/PATHNODE_SIZE;
	const int unmaxx = ucmmaxx/PATHNODE_SIZE;
	const int unmaxz = ucmmaxy/PATHNODE_SIZE;

	for(int z=unminz; z<=unmaxz; z++)
		for(int x=unminx; x<=unmaxx; x++)
		{
			ColliderTile* cell = ColliderAt(x, y);

			if(cell->building >= 0)
			{
				switch(mv->mode)
				{
					case UMODE_GOBLJOB:
					case UMODE_GOCSTJOB:
					case UMODE_GOSHOP:
					case UMODE_GOREST:
					case UMODE_GODEMB:
						if(mv->target == cell->building)
							return ectrue;
						break;
					case UMODE_GOSUP:
						if(mv->supplier == cell->building)
							return ectrue;
						break;
					case UMODE_GOREFUEL:
						if(mv->fuelstation == cell->building)
							return ectrue;
						break;
					default:
						break;
				}
			}


			for(short uiter = 0; uiter < MAX_COLLIDER_UNITS; uiter++)
			{
				short uindex = cell->mv[uiter];

				if( uindex < 0 )
					continue;

				Mv* u2 = &g_mv[uindex];

				if(!u2->hidden())
				{
					switch(mv->mode)
					{
						case UMODE_GODRIVE:
							if(mv->target == uindex)
								return ectrue;
							break;
						default:
							break;
					}
				}
			}
		}

#endif

	return ecfalse;
}

//arrived at transport vehicle
void ArAtTra(Mv* mv)
{
	Mv* u2 = &g_mv[mv->target];
	//mv->freecollider();
	mv->mode = UMODE_DRIVE;
	u2->driver = u - g_mv;
}

void OnArrived(Mv* mv)
{
	mv->pathdelay = 0;
	
	//if(mv->mode != UMODE_NONE)
		//mv->cyframes = 1;	//TODO check if it doesn't interfere with anything. needed to charge minimum of one man-hour.

	if(mv->mode == UMODE_GODRIVE)
		mv->cyframes = DRIVE_WORK_DELAY-1;
	else
		mv->cyframes = WORK_DELAY-1;

	Py* utilpy = NULL;
	Bl* b = NULL;
	CdTile* ctile = NULL;
	Mv* u2 = NULL;

	int ui = u - g_mv;

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset goal track u 2", "reset goal track u 2");
	}
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
#endif

	switch(mv->mode)
	{
	case UMODE_GOBLJOB:

	//if(u - g_mv == 182 && g_simframe > 118500)
		//Log("f14 bljob");

		mv->mode = UMODE_BLJOB;
		mv->freecollider();
		b = &g_bl[mv->target];
		b->worker.push_back(u-g_mv);
		utilpy = &g_py[b->owner];
		utilpy->util += mv->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCSTJOB:
		mv->mode = UMODE_CSTJOB;
		mv->freecollider();
		b = &g_bl[mv->target];
		b->worker.push_back(u-g_mv);
		utilpy = &g_py[b->owner];
		utilpy->util += mv->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOCDJOB:
		mv->mode = UMODE_CDJOB;
		mv->freecollider();
		ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
		utilpy = &g_py[ctile->owner];
		utilpy->util += mv->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GOSHOP:
		mv->mode = UMODE_SHOPPING;
		mv->freecollider();
		b = &g_bl[mv->target];
		utilpy = &g_py[b->owner];
		utilpy->util += mv->exputil / 10000;
		b->occupier.push_back(ui);
		ResetGoal(u);
		break;
	case UMODE_GOREST:
		mv->mode = UMODE_RESTING;
		mv->freecollider();
		b = &g_bl[mv->home];
		utilpy = &g_py[b->owner];
		utilpy->util += mv->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODRIVE:
		ArAtTra(u);
		mv->freecollider();
		RemVis(u);
		u2 = &g_mv[mv->target];
		utilpy = &g_py[u2->owner];
		utilpy->util += mv->exputil / 10000;
		ResetGoal(u);
		break;
	case UMODE_GODEMB:
		if(mv->driver >= 0)
			Disembark(&g_mv[mv->driver]);
		mv->driver = -1;
		mv->mode = UMODE_ATDEMB;
		ResetGoal(u);
		break;
	case UMODE_GODEMCD:
		if(mv->driver >= 0)
			Disembark(&g_mv[mv->driver]);
		mv->driver = -1;
		mv->mode = UMODE_ATDEMCD;
		ResetGoal(u);
		break;
	case UMODE_GOSUP:
		if(mv->driver >= 0)
			Disembark(&g_mv[mv->driver]);
		mv->driver = -1;
		mv->mode = UMODE_ATSUP;
		ResetGoal(u);
		break;
	case UMODE_GOREFUEL:
		if(mv->driver >= 0)
			Disembark(&g_mv[mv->driver]);
		mv->driver = -1;
		mv->mode = UMODE_REFUELING;
		ResetGoal(u);
		break;
	default:
		break;
	};

	//if(type == TRUCK && UnitSelected(this))
	//	RedoLeftPanel();

	///RecheckSelection();
}
