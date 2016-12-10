



#include "pathjob.h"
#include "pathnode.h"
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
#include "jpspath.h"
#include "reconstructpath.h"
#include "pathdebug.h"
#include "jpsexpansion.h"
#include "../window.h"
#include "partialpath.h"
#include "partialpath.h"
#include "../sim/transport.h"
#include "astarpath.h"
#include "anypath.h"
#include "tilepath.h"
#include "../gui/layouts/chattext.h"
#include "../gui/layouts/messbox.h"
#include "pathnode.h"
#include "../sim/simflow.h"

int64_t g_lastpath = 0;
std::vector<PathNode*> g_toclear;

#ifdef POWCD_DEBUG
std::string powcdstr;
#endif

#ifdef TSDEBUG
Unit* tracku = NULL;
#endif

bool Trapped(Unit* u, Unit *targu, Vec2i cmpos)
{
	return false;	//TODO
}

bool Trapped(Unit* u, Unit* targu)
{
	if (u && !u->hidden())
		u->freecollider();

	UType* ut = &g_utype[u->type];

	PathJob pj;
	pj.utype = u->type;
	//pj.umode = UMODE_NONE;
	//corpd fix xp
	//pj.umode = u->mode;	//not needed? TODO deep test for effect
	pj.cmstartx = u->cmpos.x;
	pj.cmstarty = u->cmpos.y;
	//pj.target = -1;
	//pj.target2 = -1;
	//pj.targtype = TARG_NONE;
	//pj.path = path;
	//pj.subgoal = subgoal;
	pj.thisu = u - g_unit;
	pj.targu = targu ? (targu - g_unit) : -1;
	//corpd fix xp hope this doesn't break anything
	//pj.targu = -1;	//should set it automatically based on unit mode params	
	//edit: for trucks, incoming driver should always be set or else he'll trap the truck if it's between two other trucks and a bl
	//pj.targu = -1;
	pj.targb = -1;
#if 0
	//pj.goalx = cmgoalx;
	//pj.goaly = cmgoaly;
	pj.goalx = pj.goalx / PATHNODE_SIZE;
	pj.goaly = pj.goaly / PATHNODE_SIZE;
	pj.goalminx = cmgoalminx;
	pj.goalminy = cmgoalminy;
	pj.goalmaxx = cmgoalmaxx;
	pj.goalmaxy = cmgoalmaxy;
#endif
	pj.roaded = ut->roaded;
	pj.landborne = ut->landborne;
	pj.seaborne = ut->seaborne;
	pj.airborne = ut->airborne;
	//pj.callback = Callback_UnitPath;
	//pj.pjtype = PATHJOB_ANYPATH;	//corpd fix xp
	//pj.pjtype = PATHJOB_QUICKPARTIAL;
	//pj.maxsearch = maxsearch;
	//pj.cdtype = cdtype;
	pj.goalx = u->goal.x;
	pj.goaly = u->goal.y;
	pj.goalx = pj.goalx / PATHNODE_SIZE;
	pj.goaly = pj.goaly / PATHNODE_SIZE;
	pj.goalminx = u->goal.x;
	pj.goalminy = u->goal.y;
	pj.goalmaxx = u->goal.x;
	pj.goalmaxy = u->goal.y;
	//pj.capend = true;
	//pj.allowpart = false;

	uint8_t sizex = ut->size.x;
	uint8_t pathoff = PathOff(sizex);
	Building* igb = NULL;
	Unit* igu = targu;

	bool roaded = ut->roaded;
	
#if 1
	uint8_t nsizex = ((pj.cmstartx-(sizex>>1)+sizex-1)/PATHNODE_SIZE-(pj.cmstartx-(sizex>>1))/PATHNODE_SIZE);
	uint8_t nsizey = ((pj.cmstarty-(sizex>>1)+sizex-1)/PATHNODE_SIZE-(pj.cmstarty-(sizex>>1))/PATHNODE_SIZE);
	
	nsizex = (sizex-1)/PATHNODE_SIZE;
	nsizey = (sizex-1)/PATHNODE_SIZE;
#else

#endif

	//ResetPathNodes();
	SnapToNode(&pj, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);

	if (!g_openlist[0].hasmore())
	{
		CleanPath(g_toclear);

#if 0
		RichText rt = RichText("!g_openlist.hasmore trapped");
		AddNotif(&rt);
#endif

#if 0
		if (u - g_unit == 15)
		{
			static bool did = false;

			if (!did)
			{
				did = true;
				//if(!stand_s)
				{
					InfoMess("snap!", "snap!");
				}
			}
		}
#endif

	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f8");

		if (u && !u->hidden())
			u->fillcollider();

		return true;
	}
	
// byte-align structures
#pragma pack(push, 1)
	PathNode* node = (PathNode*)g_openlist[0].delmin();
#pragma pack(pop)

#ifndef HASHPOOL
	Vec2i npos = PATHNODEPOS(node);
#else
	Vec2i npos;
	npos.x = node->index % g_pathdim.x;
	npos.y = node->index / g_pathdim.x;
#endif
	//uint16_t noff = imax(1, ut->size.x / PATHNODE_SIZE);
	uint16_t noff = 1;

	bool stand_n = (npos.y - noff < 0) ? false : Standable(npos.x, npos.y - noff, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool stand_s = (npos.y + noff >= g_pathdim.y) ? false : Standable(npos.x, npos.y + noff, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool stand_e = (npos.x + noff >= g_pathdim.x) ? false : Standable(npos.x + noff, npos.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool stand_w = (npos.x - noff < 0) ? false : Standable(npos.x - noff, npos.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);

	if (!stand_n && !stand_s && !stand_e && !stand_w)
	{
		CleanPath(g_toclear);
		
	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f9");

		if (u && !u->hidden())
			u->fillcollider();

		return true;
	}

	CleanPath(g_toclear);

	//if(u - g_unit == 182 && g_simframe > 118500)
		//Log("f10");

	if (u && !u->hidden())
		u->fillcollider();

	return false;
}

void CleanPath(std::vector<PathNode*> &toclear)
{
#if 0
	Log("used/total %d/%d + %d/%d = %d/%d\n\r", 
		g_openlist[0].used, g_openlist[0].total, 
		g_openlist[1].used, g_openlist[1].total, 
		g_openlist[0].used + g_openlist[1].used, g_openlist[0].total + g_openlist[1].total);
#endif

#if 0
	Log("openlist0,1 topbuck %d,%d\n\r",
		(int)g_openlist[0].topbuck.heap.size(),
		(int)g_openlist[1].topbuck.heap.size() );

	for(int i=0; i<2; i++)
	{
		HotQueue* ol = &g_openlist[i];

		Log("openlist%d bucks ", i);

		for(std::list<Widget*>::iterator bit=ol->bucks.begin(); bit!=ol->bucks.end(); bit++)
		{
			Log("[mc%d]sz%d,", (int)bit->uppercost, (int)bit->unsort.size());
		}
	}

	Log("\n\r");
#endif

	for (std::vector<PathNode*>::iterator niter = toclear.begin(); niter != toclear.end(); niter++)
	{
		PathNode* n = *niter;
		//n->opened = false;
		//n->closed = false;
		
#ifdef GPATHFLAGS
		//is this faster?
#if 0
		g_pathflags.clear(PATHNODE_FLAGBITS * n->index + 0);
		g_pathflags.clear(PATHNODE_FLAGBITS * n->index + 1);
		g_pathflags.clear(PATHNODE_FLAGBITS * n->index + 2);
		g_pathflags.clear(PATHNODE_FLAGBITS * n->index + 3);
#else
		int32_t ni = n - g_pathnode;
		g_pathflags.clear(PATHNODE_FLAGBITS * ni + 0);
		g_pathflags.clear(PATHNODE_FLAGBITS * ni + 1);
		g_pathflags.clear(PATHNODE_FLAGBITS * ni + 2);
		g_pathflags.clear(PATHNODE_FLAGBITS * ni + 3);
#endif
#endif

#ifndef HASHPOOL
#ifndef GPATHFLAGS
		//n->flags = 0;
		n->flags &= PATHNODE_BLOCKED;
#endif
		//n->prev = NULL;
#else
		//free(n);
		//*niter = NULL;
		g_pathmem.freeunit(n);
#endif
	}
	
	toclear.clear();

	g_stackpool.resetunits();

#ifdef HASHPOOL
	g_pathmem.resetunits();

#if 0
	Log("##############\r\n");

	for(int i=0; i<MAXHASH; ++i)
	{
		int c = 0;

		HashVal* hv = g_pathmap.values[i];

		while(hv)
		{
			c++;
			hv = hv->next;
		}

		Log("%d,\r\n", c);
	}
	
	Log("##############\r\n");
	Log("##############\r\n");
	Log("##############\r\n");
#endif

	//g_pathmap.free();
	g_pathmap.clear();
#endif

#ifdef GPATHFLAGS
	//is this faster?
	//g_pathflags.clearall();
#endif

	g_openlist[0].free();
	g_openlist[1].free();

	//ResetPathNodes();
}

//#define TRANSPORT_DEBUG

uint32_t g_pathage;

bool PathJob::process()
{
	ResetPathNodes();//

	UType* ut = &g_utype[utype];
	uint8_t sizex = ut->size.x;
	uint8_t pathoff = PathOff(sizex);
	Building* igb = NULL;
	Unit* igu = NULL;

	if(targb >= 0)
		igb = &g_building[targb];

	if(targu >= 0)
		igu = &g_unit[targu];

	bool roaded = ut->roaded;
	
	uint8_t nsizex = ((cmstartx-(sizex>>1)+sizex-1)/PATHNODE_SIZE-(cmstartx-(sizex>>1))/PATHNODE_SIZE);
	uint8_t nsizey = ((cmstarty-(sizex>>1)+sizex-1)/PATHNODE_SIZE-(cmstarty-(sizex>>1))/PATHNODE_SIZE);
	
	//SnapToNode(this, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);

	nsizex = (sizex-1)/PATHNODE_SIZE;
	nsizey = (sizex-1)/PATHNODE_SIZE;
	
	g_pathage = 0xffffffff;

	SnapToNode(this, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	SnapToNode2(this, nsizex, nsizey, sizex, pathoff, igb, igu, roaded, &g_unit[thisu]);
	

	
#if 0
			if(this->thisu == 205)
			{
				Unit* u = &g_unit[this->thisu];
				//bool f = u->path.size() > 0;
				bool a = g_openlist[0].hasmore();
				bool b = g_openlist[1].hasmore();

				char m[123];
				sprintf(m, "ab %d,%d", (int)a, (int)b);
				InfoMess(m,m);
			}

#endif

	//if(!g_openlist[1].hasmore())
	//	InfoMess("no2","no2");
	
// byte-align structures
#pragma pack(push, 1)
	PathNode* node;
#pragma pack(pop)
	searchdepth = 0;
	closest = 0;
	closestnode = NULL;

	while (g_openlist[0].hasmore() && g_openlist[1].hasmore())
	{
		searchdepth++;

		if (searchdepth > maxsearch && pjtype == PATHJOB_QUICKPARTIAL)
			break;
		
// byte-align structures
#pragma pack(push, 1)
		// Pops the lowest cost-cost node, moves it in the closed std::list
		node = (PathNode*)g_openlist[0].delmin();
		//int32_t i = node - g_pathnode;
#pragma pack(pop)

#ifndef HASHPOOL
		Vec2i npos = PATHNODEPOS(node);
#else
		Vec2i npos;
		npos.x = node->index % g_pathdim.x;
		npos.y = node->index / g_pathdim.x;
#endif

#if 0	//what's endnode? dont have it. only endnode2.
		if(node->flags & (PATHNODE_OPENED2 | PATHNODE_CLOSED2))
		{
			//path found
			
			ReconstructPath2(this, node);

			CleanPath(g_toclear);

			if (callback)
				callback(true, this);

			return true;
		}
		else
#endif
		{
			//node->closed = true;
#ifndef GPATHFLAGS
			node->flags |= PATHNODE_CLOSED;
#else
			//g_pathflags.set(PATHNODE_FLAGBITS * node->index + PATHNODE_CLOSED_BIT);
			int32_t ni = node - g_pathnode;
			g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED_BIT);
#endif

#if 0
			// If the popped node is the endNode, return it
			if (AtGoal(this, node))
			{
				ReconstructPath(this, node);

				CleanPath(g_toclear);

				if (callback)
					callback(true, this);

				return true;
			}
#endif

			if(Expand_QP(this, node, nsizex, nsizey, sizex, pathoff, igb, igu, roaded, PATHNODE_FORWARD))
			{
				//todo check if checking for found path inside node expansion is slower

				CleanPath(g_toclear);

				if (callback)
					callback(true, this);

				return true;
			}
		}
		
		//now backwards
		
// byte-align structures
#pragma pack(push, 1)
		node = (PathNode*)g_openlist[1].delmin();
#pragma pack(pop)
		
#ifndef HASHPOOL
		npos = PATHNODEPOS(node);
#else
		npos.x = node->index % g_pathdim.x;
		npos.y = node->index / g_pathdim.x;
#endif

#if 0
		if(node->flags & (PATHNODE_OPENED | PATHNODE_CLOSED))
		{
			//path found
		}
		else
#endif
		{
			//node->closed = true;
#ifndef GPATHFLAGS
			node->flags |= PATHNODE_CLOSED2;
#else
			//g_pathflags.set(PATHNODE_FLAGBITS * node->index + PATHNODE_CLOSED2_BIT);
			int32_t ni = node - g_pathnode;
			g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED2_BIT);
#endif

#if 0
			// If the popped node is the endNode, return it
			if (AtGoal(this, node))
			{
				ReconstructPath(this, node);

				CleanPath(g_toclear);

				if (callback)
					callback(true, this);

				return true;
			}
#endif

			if(Expand_QP(this, node, nsizex, nsizey, sizex, pathoff, igb, igu, roaded, PATHNODE_BACKWARD))
			{
				CleanPath(g_toclear);

				if (callback)
					callback(true, this);

				return true;
			}
		}
	}

	bool pathfound = false;

	if ((pjtype == PATHJOB_QUICKPARTIAL) &&
		closestnode && allowpart)
	{
		pathfound = true;
		ReconstructPath(this, closestnode);
	}

	CleanPath(g_toclear);

	if (callback)
		callback(pathfound, this);

	return true;
}

void Callback_UnitPath(bool result, PathJob* pj)
{
	int16_t ui = pj->thisu;

	if (ui < 0)
		return;

	Unit* u = &g_unit[ui];

	u->threadwait = false;
	u->pathblocked = !result;
}
