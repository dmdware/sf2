










#include "pathnode.h"
#include "../math/vec2i.h"
#include "../math/3dmath.h"
#include "pathjob.h"
#include "../sim/utype.h"
#include "collidertile.h"
#include "../algo/binheap.h"
#include "../utils.h"
#include "../debug.h"
#include "../sim/unit.h"
#include "cheaplist.h"

#ifdef HASHPOOL
StackPool g_pathmem;
//HashMap g_pathmap;
std::unordered_map<int32_t, PathNode*> g_pathmap;
#else
PathNode* g_pathnode = NULL;
#endif

StackPool g_stackpool;

Vec2i g_pathdim(0,0);

#ifdef CHEAPLIST
CheapList g_openlist[2];
#elif defined(HOTQUEUE)
HotQueue g_openlist[2];
#elif defined(FIBHEAP)
FibHeap g_openlist[2];
#else
BinHeap g_openlist[2];
#endif


#ifdef GPATHFLAGS
BitSet g_pathflags;
#endif

#if 0
PathNode::PathNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, PathNode* prev, int32_t rund, int32_t stepD)
{
	//this->nx = nx;
	//this->ny = ny;
	int32_t G = rund + stepD;
	this->rund = G;
	//float G = Dot(Vec3f(startx-x,0,startz-z));
	//multiply by 2 to give granularity
	//(needed for diagonal moves).
	int32_t H = PATHHEUR(Vec2i(endx-nx,endz-ny)) * 2;
	//int32_t H = Manhattan(Vec2i(endx-nx,endz-ny)) * 2;
	//int32_t G = iabs(startx-x) + iabs(startz-z);
	//int32_t H = iabs(endx-x) + iabs(endz-z);
	cost = G + H;
	//cost = H;
	prev = prev;
	//tried = false;
}
#endif

#if 0
PathNode::PathNode(int32_t startx, int32_t startz, int32_t endx, int32_t endz, int32_t nx, int32_t ny, PathNode* prev, int32_t rund, int32_t stepD, unsigned char expan)
{
	this->nx = nx;
	this->ny = ny;
	int32_t G = rund + stepD;
	this->rund = G;
	//float G = Dot(Vec3f(startx-x,0,startz-z));
	//float H = MAG_VEC3F(Vec2i(endx-nx,endz-ny));
	int32_t H = Manhattan(Vec2i(endx-nx,endz-ny));
	//int32_t G = iabs(startx-x) + iabs(startz-z);
	//int32_t H = iabs(endx-x) + iabs(endz-z);
	cost = G + H;
	//cost = H;
	prev = prev;
	//tried = false;
	expansion = expan;
}
#endif

/*
The center path node position is obtained by dividing the unit's cm position by PATHNODE_SIZE.
To compactly pack the unit in the least number of path nodes, an offset might be subtracted before
getting the path node position, so that cm width aligns perfectly with least number of path nodes used.

There are two different uses here: 1.) turning a cm-position-centered unit into a path node position,
and 2.) reconstructing the cm goal positions from a path node path plot.

Use case (2) is used in checking if a path node-area is standable, and in Standable2 (where the cm-accurate
position is used), (1) is used.

For an exactly 2-path-node-wide unit, centered on (1000,1000) cm, the unit is already evenly
distributed. It is important how the area that the unit occupies is calculated from a
given cm position. First, from the cm position, half the width (2000/2=1000) is subtracted,
and because this is an integer division, any remainder is rounded down.
Then, from this minimum point (0,0), the full width is added (+2000=(2000,2000)), so
the unit might be jutting by 1 unit in the upper-bound direction if the width is odd in cm.
This means it occupies that area [0,0]->[2000,2000] inclusively, but it doesn't collide
with the unit at [2000,2000]->[4000,4000] even though it should be touching at the point
(2000,2000) because the intersection test tests for a_max_x>b_min_x as opposed to
a_max_x>=b_min_x. This is all necessary for the math to work.

To use this, just subtract "pathoff" from cm position before dividing by
PATHNODE_SIZE to get path node position. Then, to reconstruct, add "pathoff"
after multiplying by PATHNODE_SIZE.

In reality, if cmwidth is not evenly divisible by PATHNODE_SIZE, there can
be any number of ways to fit it in the path nodes, but we will give only one result.
The disadvantage of this might be that a starting node might be detected as colliding
when there might in fact be a way to place the unit.
*/
#if 0
int32_t PathOff(int32_t cmwidth)
{
	int32_t rem, div, mul, res;

	/*
	Example width = 3005
	Desired result = 1500
	*/

	div = cmwidth / PATHNODE_SIZE;	//3
	mul = div * PATHNODE_SIZE;	//3000
	rem = cmwidth - mul;	//5
	res = (cmwidth - rem) / 2;	//1500

	return res;
}
#else
//just align cm min end to 0 boundary between pathnodes so Standable optimization can be used
int32_t PathOff(int32_t cmwidth)
{
	/*
	Example width = 3005
	Desired result = 1502

	cmwidth = 3005
	cmwidth>>1 = 1502
	(cmwidth>>1)/PATHNODE_SIZE = 30
	((cmwidth>>1)/PATHNODE_SIZE)*PATHNODE_SIZE+1502 = 
	*/
	return cmwidth>>1;
}
#endif

#if 0
Vec2i PathNodePos(PathNode* node)
{
	int i = node - g_pathnode;
	return Vec2i(i % g_pathdim.x, i / g_pathdim.x);
}
#endif

//#include <assert.h>

bool AtGoal(PathJob* pj, PathNode* node)
{
	UType* ut = &g_utype[pj->utype];

#ifndef HASHPOOL
	Vec2i npos = PATHNODEPOS(node);
#else
	Vec2i npos;
	npos.x = node->index % g_pathdim.x;
	npos.y = node->index / g_pathdim.x;
#endif

	int32_t cmposx = npos.x * PATHNODE_SIZE + PathOff(ut->size.x);
	int32_t cmposy = npos.y * PATHNODE_SIZE + PathOff(ut->size.x);

	int32_t cmminx = cmposx - ut->size.x/2;
	int32_t cmminy = cmposy - ut->size.x/2;
	int32_t cmmaxx = cmminx + ut->size.x - 1;
	int32_t cmmaxy = cmminy + ut->size.x - 1;

	if(cmminx <= pj->goalmaxx && cmminy <= pj->goalmaxy && cmmaxx >= pj->goalminx && cmmaxy >= pj->goalminy)
		return true;

	return false;
}

/*
This is very important not to create unwalkable paths.
If you start from the pathnode that is gotten by rounding down the cmpos,
you might be off. This is from experience of mass movement of military units
and seeing repeating movement loops that units fail to break out of.
*/
/*
Once a node is snapped to as the starting node, even if the unit isn't completely aligned to it,
Standable will be used which assumes cmmin is aligned to the 0 margin between tiles.
Therefore this function must use Standable2 to make accurate checks to determine a corner (NW,SW,NE,SE)
that the unit can fully move to without colliding.
Only a single unit can occupy any collider tile now, but if a diagonal move is possible from a 0-aligned position
but there's something in the way from the unit's real position, then it will be stuck forever.
Also, if a unit 2 pathnodes wide, and it's at 90% of pathnode 0, it will reach not only into pathnode 1, but 2 also.
If we assume it will give the same results as a 0-aligned unit, we will have problems.
*/
void SnapToNode(PathJob* pj, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded)
{
#if 1
	//Is this correct? This was my first solution and I changed it to the second one for some time,
	//but seeing an unresolvable truck position that remained trapped even though it could actually move out,
	//changed it back, solving the trapped truck problem.
	//Edit: No, the second method is correct. The only solution to the truck being stuck is to use more pathnode divisions.
	//This is more realistic anyways (using PATHNODE_SIZE of 50 centimeters instead of 125, because a person has about 50 cm
	//of personal space, and a 10x10 meter road tile divides to give 20x20 people worth of room).
	//Edit: the first method must be correct, since for the npos_min we are at npos-1, and for npos_max we are at npos+1.
	Vec2i npos = Vec2i( (pj->cmstartx-pathoff) / PATHNODE_SIZE, (pj->cmstarty-pathoff) / PATHNODE_SIZE );
	//Vec2i npos = Vec2i( (pj->cmstartx) / PATHNODE_SIZE, (pj->cmstarty) / PATHNODE_SIZE );
	//Vec2i npos = Vec2i( (pj->cmstartx-PATHNODE_SIZE/2) / PATHNODE_SIZE, (pj->cmstarty-PATHNODE_SIZE/2) / PATHNODE_SIZE );

	npos.x = imin(g_pathdim.x-1, npos.x);
	npos.y = imin(g_pathdim.y-1, npos.y);

#if 0
	Vec2i npos_min = npos - Vec2i(1,1);

	npos_min.x = imax(0, npos_min.x);
	npos_min.y = imax(0, npos_min.y);

	Vec2i npos_max = npos_min + Vec2i(1,1);
	
	npos_max.x = imin(g_pathdim.x-1, npos_max.x);
	npos_max.y = imin(g_pathdim.y-1, npos_max.y);

	Vec2i npos_nw = Vec2i( npos_min.x, npos_min.y );
	Vec2i npos_ne = Vec2i( npos_max.x, npos_min.y );
	Vec2i npos_sw = Vec2i( npos_min.x, npos_max.y );
	Vec2i npos_se = Vec2i( npos_max.x, npos_max.y );
#if 0
	Vec2i npos_w = Vec2i( npos_min.x, npos.y );
	Vec2i npos_e = Vec2i( npos_max.x, npos.y );
	Vec2i npos_n = Vec2i( npos.x, npos_min.y );
	Vec2i npos_s = Vec2i( npos.x, npos_max.y );
#endif
#endif

#if 0
	PathNode* node_nw = PATHNODEAT(npos_nw.x, npos_nw.y);
	PathNode* node_ne = PATHNODEAT(npos_ne.x, npos_ne.y);
	PathNode* node_sw = PATHNODEAT(npos_sw.x, npos_sw.y);
	PathNode* node_se = PATHNODEAT(npos_se.x, npos_se.y);
#if 0
	PathNode* node_w = PATHNODEAT(npos_w.x, npos_w.y);
	PathNode* node_e = PATHNODEAT(npos_e.x, npos_e.y);
	PathNode* node_n = PATHNODEAT(npos_n.x, npos_n.y);
	PathNode* node_s = PATHNODEAT(npos_s.x, npos_s.y);
#endif
#endif
#ifndef HASHPOOL
	PathNode* node_cen = PATHNODEAT(npos.x, npos.y);
#endif

#if 0
	bool walkable_nw = Standable(npos_nw.x, npos_nw.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool walkable_ne = Standable(npos_ne.x, npos_ne.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool walkable_sw = Standable(npos_sw.x, npos_sw.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
	bool walkable_se = Standable(npos_se.x, npos_se.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);
#if 0
	bool walkable_w = Standable(pj, npos_w.x, npos_w.y);
	bool walkable_e = Standable(pj, npos_e.x, npos_e.y);
	bool walkable_n = Standable(pj, npos_n.x, npos_n.y);
	bool walkable_s = Standable(pj, npos_s.x, npos_s.y);
#endif
#endif 
	bool walkable_cen = Standable(npos.x, npos.y, nsizex, nsizey, sizex, pathoff, igb, igu, roaded);

#if 0
	Vec2i cmpos_nw = Vec2i( npos_nw.x * PATHNODE_SIZE + pathoff, npos_nw.y * PATHNODE_SIZE + pathoff );
	Vec2i cmpos_ne = Vec2i( npos_ne.x * PATHNODE_SIZE + pathoff, npos_ne.y * PATHNODE_SIZE + pathoff );
	Vec2i cmpos_sw = Vec2i( npos_sw.x * PATHNODE_SIZE + pathoff, npos_sw.y * PATHNODE_SIZE + pathoff );
	Vec2i cmpos_se = Vec2i( npos_se.x * PATHNODE_SIZE + pathoff, npos_se.y * PATHNODE_SIZE + pathoff );
#if 0
	Vec2i cmpos_cen = Vec2i(npos.x * PATHNODE_SIZE + pathoff, npos.y * PATHNODE_SIZE + pathoff);
	Vec2i cmpos_w = Vec2i(npos_w.x * PATHNODE_SIZE + pathoff, npos_w.y * PATHNODE_SIZE + pathoff);
	Vec2i cmpos_e = Vec2i(npos_e.x * PATHNODE_SIZE + pathoff, npos_e.y * PATHNODE_SIZE + pathoff);
	Vec2i cmpos_s = Vec2i(npos_s.x * PATHNODE_SIZE + pathoff, npos_s.y * PATHNODE_SIZE + pathoff);
	Vec2i cmpos_n = Vec2i(npos_n.x * PATHNODE_SIZE + PathOff(ut->size.x), npos_n.y * PATHNODE_SIZE + PathOff(ut->size.x));
#endif
#endif

#if 0
	Vec2i off_nw = Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_nw;
	Vec2i off_ne = Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_ne;
	Vec2i off_sw = Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_sw;
	Vec2i off_se = Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_se;
	int32_t dist_nw = MAG_VEC2I( off_nw );
	int32_t dist_ne = MAG_VEC2I( off_ne );
	int32_t dist_sw = MAG_VEC2I( off_sw );
	int32_t dist_se = MAG_VEC2I( off_se );
#if 0
	int32_t dist_cen = MAG_VEC3F(Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_cen);
	int32_t dist_w = MAG_VEC3F(Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_w);
	int32_t dist_e = MAG_VEC3F(Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_e);
	int32_t dist_s = MAG_VEC3F(Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_s);
	int32_t dist_n = MAG_VEC3F(Vec2i(pj->cmstartx, pj->cmstarty) - cmpos_n);
#endif
#endif

	PathNode* startnode = NULL;

	int32_t nearest = -1;

#if 0	//does this cause any problems? corpd fix xp
	if( walkable_cen && walkable_w && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_e && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_s && !startnode )
	{
		startnode = node_cen;
	}
	if( walkable_cen && walkable_n && !startnode )
	{
		startnode = node_cen;
	}
#endif

	//corpd fix xp commented out dist_* < nearest
#if 0
	if( walkable_nw && walkable_ne && walkable_sw && (dist_nw < nearest || !startnode) )
	{
		nearest = dist_nw;
		startnode = node_nw;
		npos = npos_nw;
	}
	if( walkable_ne && walkable_nw && walkable_se && (dist_ne < nearest || !startnode) )
	{
		nearest = dist_ne;
		startnode = node_ne;
		npos = npos_ne;
	}
	if( walkable_sw && walkable_nw && walkable_se && (dist_sw < nearest || !startnode) )
	{
		nearest = dist_sw;
		startnode = node_sw;
		npos = npos_sw;
	}
	if( walkable_se && walkable_ne && walkable_sw && (dist_se < nearest || !startnode) )
	{
		nearest = dist_se;
		startnode = node_se;
		npos = npos_se;
	}

#if 0
	if (walkable_w && (dist_w < nearest || !startnode))
	{
		nearest = dist_w;
		startnode = node_w;
		npos = npos_w;
	}
	if (walkable_e && (dist_e < nearest || !startnode))
	{
		nearest = dist_e;
		startnode = node_e;
		npos = npos_e;
	}
	if (walkable_s && (dist_s < nearest || !startnode))
	{
		nearest = dist_s;
		startnode = node_s;
		npos = npos_s;
	}
	if (walkable_n && (dist_n < nearest || !startnode))
	{
		nearest = dist_n;
		startnode = node_n;
		npos = npos_n;
	}
#endif
#endif

#if 1
	if (walkable_cen /* && (dist_cen < nearest || !startnode) */ )
	{
#ifdef HASHPOOL
		int32_t ni = PATHNODEINDEX(npos.x, npos.y);
		//PathNode* node_cen = (PathNode*)g_pathmap.get(ni);
		PathNode* node_cen;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo maybe assume this will always be true here?
		//if(!node_cen)
		if(nit == g_pathmap.end())
		{
			node_cen = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			node_cen->index = ni;
#ifndef GPATHFLAGS
			node_cen->flags = 0;
#endif
			node_cen->prev = NULL;
			//g_pathmap.add(ni, node_cen);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, node_cen));
		}
		else
			node_cen = (*nit).second;
#endif
		//nearest = dist_cen;
		startnode = node_cen;
		npos = npos;
	}
#endif

	if (startnode)
		goto foundnode;

	return; 

#if 0
	if(!startnode)
	{
		UType* ut = &g_utype[pj->utype];

		{
			int32_t nposx = npos_nw.x;
			int32_t nposy = npos_nw.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + pathoff, nposy * PATHNODE_SIZE + pathoff);
			Vec2i dir = to - from;

			if(dir == Vec2i(0,0))
			{
				startnode = node_nw;
				goto foundnode;
			}

			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / MAG_VEC2I(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_nw < nearest || !startnode) )
			{
				nearest = dist_nw;
				startnode = node_nw;
			}
		}

		{
			int32_t nposx = npos_ne.x;
			int32_t nposy = npos_ne.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + pathoff, nposy * PATHNODE_SIZE + pathoff);
			Vec2i dir = to - from;

			if(dir == Vec2i(0,0))
			{
				startnode = node_ne;
				goto foundnode;
			}

			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / MAG_VEC2I(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_ne < nearest || !startnode) )
			{
				nearest = dist_ne;
				startnode = node_ne;
			}
		}

		{
			int32_t nposx = npos_sw.x;
			int32_t nposy = npos_sw.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + pathoff, nposy * PATHNODE_SIZE + pathoff);
			Vec2i dir = to - from;

			if(dir == Vec2i(0,0))
			{
				startnode = node_sw;
				goto foundnode;
			}

			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / MAG_VEC2I(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_sw < nearest || !startnode) )
			{
				nearest = dist_sw;
				startnode = node_sw;
			}
		}

		{
			int32_t nposx = npos_se.x;
			int32_t nposy = npos_se.y;
			Vec2i from(pj->cmstartx, pj->cmstarty);
			Vec2i to(nposx * PATHNODE_SIZE + pathoff, nposy * PATHNODE_SIZE + pathoff);
			Vec2i dir = to - from;

			if(dir == Vec2i(0,0))
			{
				startnode = node_se;
				goto foundnode;
			}

			//replace by Trace
			Vec2i scaleddir = dir * ut->cmspeed / MAG_VEC2I(dir);
			Vec2i scaleddir2 = dir / 2;
			Vec2i stepto = from + scaleddir;
			Vec2i stepto2 = from + scaleddir2;

			if(Standable2(pj, stepto.x, stepto.y) && Standable2(pj, stepto2.x, stepto2.y) && (dist_se < nearest || !startnode) )
			{
				nearest = dist_se;
				startnode = node_se;
			}
		}

		if(!startnode)
		{
#if 0
			if(pj->thisu == 15)
			{
				static bool did = false;

				if(!did)
				{
					did = true;
					InfoMess("st!", "!st");
				}
			}
#endif
#if 1
			//corpd fix xp
			if( walkable_cen && !startnode )
			{
				startnode = node_cen;
				goto foundnode;
			}
#endif

			return;
		}
	}
#endif

foundnode:

	Vec2i cmpos = Vec2i( npos.x * PATHNODE_SIZE + pathoff, npos.y * PATHNODE_SIZE + pathoff );

	//todo put pathoff in vec2i
	Vec2i startoff = (Vec2i(pj->cmstartx, pj->cmstarty) - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE - (cmpos - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE;
	startnode->rund = PATHHEUR( startoff ) * 2;
	Vec2i runoff = (Vec2i(pj->goalx, pj->goaly) - Vec2i(1, 1)*pathoff) - (cmpos - Vec2i(1,1)*pathoff) /PATHNODE_SIZE;
	startnode->heapkey.cost = startnode->rund + ( PATHHEUR( runoff ) * 2 );
#ifdef HEAPKEYAGE
	startnode->heapkey.age = g_pathage--;
#endif
	startnode->prev = NULL;

	g_openlist[0].setbase(startnode->heapkey);
	g_openlist[0].add(startnode);

	//startnode->opened = true;
#ifndef GPATHFLAGS
	startnode->flags |= PATHNODE_OPENED;
#else
	//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT);
	int32_t ni = startnode - g_pathnode;
	g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED_BIT);
#endif
	g_toclear.push_back(startnode);

#else
	Vec2i npos = Vec2i( (pj->cmstartx) / PATHNODE_SIZE, (pj->cmstarty) / PATHNODE_SIZE );
	PathNode* startnode = PATHNODEAT(npos.x, npos.y);

	Vec2i cmpos = Vec2i( npos.x * PATHNODE_SIZE + PathOff(ut->size.x), npos.y * PATHNODE_SIZE + PathOff(ut->size.x) );

	startnode->rund = MAG_VEC3F( Vec2i(pj->cmstartx, pj->cmstarty) - cmpos );
	startnode->heapkey.cost = startnode->rund + Manhattan( Vec2i(pj->goalx, pj->goaly) - cmpos );
	startnode->prev = NULL;

	g_openlist.insert(startnode);
	//pj->wt->opennode[ startnode - pj->wt->pathnode ] = pj->wt->pathcnt;

	startnode->opened = true;
	g_toclear.push_back(startnode);
#endif

	//startNode._opened = true
	//toClear[startNode] = true
}

//place the starting pathnodes to g_openlist[2] for the direction going backwards
//WARNING: if startnode is only 1 and is the same node as in SnapToNode() then there will be an infinite loop when reconstructing the path and following ->prev path
//actually, it might just give the wrong path and the unit will be stuck forever
void SnapToNode2(PathJob* pj, const uint8_t nsizex, const uint8_t nsizey, const uint8_t sizex, const uint8_t pathoff, const Building* igb, const Unit* igu, bool roaded,
				 Unit* thisu)
{
	uint16_t ngoalminx = pj->goalminx/ PATHNODE_SIZE;
	uint16_t ngoalmaxx = pj->goalmaxx/ PATHNODE_SIZE;
	uint16_t ngoalminy = pj->goalminy/ PATHNODE_SIZE;
	uint16_t ngoalmaxy = pj->goalmaxy/ PATHNODE_SIZE;
	
	ngoalminx = imax(ngoalminx-nsizex, 0);
	ngoalminy = imax(ngoalminy-nsizey, 0);
	ngoalmaxx = imin(ngoalmaxx+0, g_pathdim.x-1);
	ngoalmaxy = imin(ngoalmaxy+0, g_pathdim.y-1);

	//pay attention to the +1 and <=/< done to avoid repeating any nodes but avoiding having no nodes for a 1x1 dest rect

	for(uint16_t nx=ngoalminx; nx<=ngoalmaxx; ++nx)
	{
		const uint16_t ny = ngoalminy;

		if(!Standable(nx, ny, nsizex, nsizey, sizex, pathoff, igb, igu, roaded))
			continue;

#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & PATHNODE_OPENED)
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT))
#endif
			continue;
#endif

		Vec2i cmpos = Vec2i( nx * PATHNODE_SIZE + pathoff, ny * PATHNODE_SIZE + pathoff );

		//todo put pathoff in vec2i to make simpler
		Vec2i startoff = (Vec2i(pj->goalx, pj->goaly) - Vec2i(1, 1)*pathoff) - (cmpos - Vec2i(1,1)*pathoff) /PATHNODE_SIZE;
		Vec2i runoff = (Vec2i(pj->cmstartx, pj->cmstarty) - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE - (cmpos - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE;
		startnode->rund = PATHHEUR( startoff ) * 2;
		startnode->heapkey.cost = startnode->rund + ( PATHHEUR( runoff ) * 2 );
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
		startnode->prev = NULL;

		g_openlist[1].setbase(startnode->heapkey);
		g_openlist[1].add(startnode);

		//startnode->opened = true;
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_OPENED2;
#else
		
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}
	
	for(uint16_t nx=ngoalminx+1; nx<ngoalmaxx; ++nx)
	{
		const uint16_t ny = ngoalmaxy;

		if(!Standable(nx, ny, nsizex, nsizey, sizex, pathoff, igb, igu, roaded))
			continue;

#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & PATHNODE_OPENED)
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT))
#endif
			continue;
#endif

		Vec2i cmpos = Vec2i( nx * PATHNODE_SIZE + pathoff, ny * PATHNODE_SIZE + pathoff );

		//todo put pathoff in vec2i to make simpler
		Vec2i startoff = (Vec2i(pj->goalx, pj->goaly) - Vec2i(1, 1)*pathoff) - (cmpos - Vec2i(1,1)*pathoff) /PATHNODE_SIZE;
		Vec2i runoff = (Vec2i(pj->cmstartx, pj->cmstarty) - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE - (cmpos - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE;
		startnode->rund = PATHHEUR( startoff ) * 2;
		startnode->heapkey.cost = startnode->rund + ( PATHHEUR( runoff ) * 2 );
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
		startnode->prev = NULL;
		
		g_openlist[1].setbase(startnode->heapkey);
		g_openlist[1].add(startnode);

		//startnode->opened = true;
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_OPENED2;
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

	for(uint16_t ny=ngoalminy+1; ny<=ngoalmaxy; ++ny)
	{
		const uint16_t nx = ngoalminx;

		if(!Standable(nx, ny, nsizex, nsizey, sizex, pathoff, igb, igu, roaded))
			continue;

#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & PATHNODE_OPENED)
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT))
#endif
			continue;
#endif

		Vec2i cmpos = Vec2i( nx * PATHNODE_SIZE + pathoff, ny * PATHNODE_SIZE + pathoff );

		//todo put pathoff in vec2i to make simpler
		Vec2i startoff = (Vec2i(pj->goalx, pj->goaly) - Vec2i(1, 1)*pathoff) - (cmpos - Vec2i(1,1)*pathoff) /PATHNODE_SIZE;
		Vec2i runoff = (Vec2i(pj->cmstartx, pj->cmstarty) - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE - (cmpos - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE;
		startnode->rund = PATHHEUR( startoff ) * 2;
		startnode->heapkey.cost = startnode->rund + ( PATHHEUR( runoff ) * 2 );
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
		startnode->prev = NULL;
		
		g_openlist[1].setbase(startnode->heapkey);
		g_openlist[1].add(startnode);

		//startnode->opened = true;
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_OPENED2;
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}
	
	for(uint16_t ny=ngoalminy+1; ny<=ngoalmaxy; ++ny)
	{
		const uint16_t nx = ngoalmaxx;

		if(!Standable(nx, ny, nsizex, nsizey, sizex, pathoff, igb, igu, roaded))
			continue;

#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & PATHNODE_OPENED)
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT))
#endif
			continue;
#endif

		Vec2i cmpos = Vec2i( nx * PATHNODE_SIZE + pathoff, ny * PATHNODE_SIZE + pathoff );

		//todo put pathoff in vec2i to make simpler
		Vec2i startoff = (Vec2i(pj->goalx, pj->goaly) - Vec2i(1, 1)*pathoff) - (cmpos - Vec2i(1,1)*pathoff) /PATHNODE_SIZE;
		Vec2i runoff = (Vec2i(pj->cmstartx, pj->cmstarty) - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE - (cmpos - Vec2i(1, 1)*pathoff)/PATHNODE_SIZE;
		startnode->rund = PATHHEUR( startoff ) * 2;
		startnode->heapkey.cost = startnode->rund + ( PATHHEUR( runoff ) * 2 );
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
		startnode->prev = NULL;
		
		g_openlist[1].setbase(startnode->heapkey);
		g_openlist[1].add(startnode);

		//startnode->opened = true;
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_OPENED2;
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_OPENED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

	//don't search middle nodes

#if 0
	for(uint16_t ny=ngoalminy+1; ny<ngoalmaxy; ++ny)
	{
		for(uint16_t nx=ngoalminx+1; nx<ngoalmaxx; ++nx)
		{
#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
#ifndef GPATHFLAGS
		else if(startnode->flags & (PATHNODE_OPENED | PATHNODE_CLOSED2))
#else
		else if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT) ||
			g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT))
#endif
			continue;
#endif
			
#ifndef GPATHFLAGS
			startnode->flags |= PATHNODE_CLOSED2;
			startnode->heapkey.age = g_pathage--;
#else
			g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT);
#endif
			g_toclear.push_back(startnode);
		}
	}
#else
	//only fill inside perimeter
	
	if(ngoalminx+1 >= ngoalmaxx-1)
		goto skip0;

	for(uint16_t ny=ngoalminy+1; ny<ngoalmaxy; ++ny)
	{
		uint16_t nx=ngoalminx+1;
#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & (PATHNODE_OPENED | PATHNODE_CLOSED2))
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT) ||
			g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT))
#endif
			continue;
#endif
		
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_CLOSED2;
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

	skip0:

	if(ngoalmaxx-1 <= ngoalminx+1)
		goto skip1;

	for(uint16_t ny=ngoalminy+2; ny<ngoalmaxy-1; ++ny)
	{
		uint16_t nx=ngoalmaxx-1;
#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & (PATHNODE_OPENED | PATHNODE_CLOSED2))
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT) ||
			g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT))
#endif
			continue;
#endif
		
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_CLOSED2;
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

skip1:

	if(ngoalminy+1 >= ngoalmaxy)
		goto skip2;

	for(uint16_t nx=ngoalminx+2; nx<ngoalmaxx; ++nx)
	{
		uint16_t ny=ngoalminy+1;
#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
		else
			startnode = (*nit).second;
#ifndef GPATHFLAGS
		else if(startnode->flags & (PATHNODE_OPENED | PATHNODE_CLOSED2))
#else
		if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT) ||
			g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT))
#endif
			continue;
#endif
		
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_CLOSED2;
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

skip2:

	if(ngoalmaxy-1 <= ngoalminy)
		goto skip3;

	for(uint16_t nx=ngoalminx+2; nx<ngoalmaxx; ++nx)
	{
		uint16_t ny=ngoalmaxy-1;
#ifndef HASHPOOL
		PathNode* startnode = PATHNODEAT(nx,ny);
#else
		int32_t ni = PATHNODEINDEX(nx,ny);
		//PathNode* startnode = (PathNode*)g_pathmap.get(ni);
		PathNode* startnode;
		std::list<Widget*>::iterator nit = g_pathmap.find(ni);

		//todo assume this is always true here?
		//if(!startnode)
		if(nit == g_pathmap.end())
		{
			//todo memory checks here and other places where g_pathmem.alloc is used
			startnode = (PathNode*)g_pathmem.alloc(sizeof(PathNode));
			startnode->index = ni;
#ifndef GPATHFLAGS
			startnode->flags = 0;
#endif
			startnode->prev = NULL;
			//g_pathmap.add(ni, startnode);
			g_pathmap.insert(std::pair<int32_t, PathNode*>(ni, startnode));
		}
#ifndef GPATHFLAGS
		else if(startnode->flags & (PATHNODE_OPENED | PATHNODE_CLOSED2))
#else
		else if(g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_OPENED_BIT) ||
			g_pathflags.on(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT))
#endif
			continue;
#endif
		
#ifndef GPATHFLAGS
		startnode->flags |= PATHNODE_CLOSED2;
#ifdef HEAPKEYAGE
		startnode->heapkey.age = g_pathage--;
#endif
#else
		//g_pathflags.set(PATHNODE_FLAGBITS * startnode->index + PATHNODE_CLOSED2_BIT);
		int32_t ni = startnode - g_pathnode;
		g_pathflags.set(PATHNODE_FLAGBITS * ni + PATHNODE_CLOSED2_BIT);
#endif
		g_toclear.push_back(startnode);
	}

skip3:
	;
#endif
}

//int64_t g_lastpath;

void ResetPathNodes()
{
	return;

#if 0

	StartTimer(TIMER_RESETPATHNODES);

	for(int32_t i = 0; i < g_pathdim.x * g_pathdim.y; i++)
	{
		PathNode* n = &g_pathnode[i];
		n->closed = false;
		n->opened = false;
		n->prev = NULL;
	}
	g_openlist.resetelems();

	StopTimer(TIMER_RESETPATHNODES);

#endif // 0
}
