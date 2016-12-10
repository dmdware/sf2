










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
#include "pathjob.h"
#include "reconstructpath.h"
#include "../gui/layouts/chattext.h"

void ReconstructPath(PathJob* pj, PathNode* endnode)
{
	pj->path->clear();

	UType* ut = &g_utype[pj->utype];

#if 1
	// Reconstruct the path, following the path steps
	for(PathNode* n = endnode; n; n = n->prev)
	{
#ifndef HASHPOOL
		Vec2i npos = PATHNODEPOS(n);
#else
		Vec2i npos;
		npos.x = n->index % g_pathdim.x;
		npos.y = n->index / g_pathdim.x;
#endif

#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("Reconstruct "<<npos.x<<","<<npos.y<<" pjtype="<<(int32_t)pj->pjtype);
		
	}
#endif

		Vec2i cmpos( npos.x * PATHNODE_SIZE + PathOff(ut->size.x), npos.y * PATHNODE_SIZE + PathOff(ut->size.x) );
		pj->path->push_front(cmpos);
	}
#endif


#ifdef RANDOM8DEBUG
	if(pj->thisu == thatunit)
	{
		Log("Reconstruct start "<<(pj->cmstartx/PATHNODE_SIZE)<<","<<(pj->cmstarty/PATHNODE_SIZE)<<" pjtype="<<(int32_t)pj->pjtype);
		
	}
#endif

#if 1	//necessary for exact point
	//pj->path->push_back(Vec2i(pj->goalx, pj->goaly)*PATHNODE_SIZE + PathOff(ut->size.x));
	if(pj->capend)
		pj->path->push_back(pj->cmgoal);
#endif

	if(pj->path->size() > 0)
		*pj->subgoal = *pj->path->begin();

#if 0
	if(pj->thisu == 457)
	{
		std::list<Widget*>::iterator lastp = pj->path->begin();
		int32_t i=0;
		for(std::list<Widget*>::iterator p=pj->path->begin(); p!=pj->path->end() && i<30; i++, p++)
		{
			char msg[128];

			sprintf(msg, "path[%d] n(%d,%d) cm(%d,%d) d(%d,%d)", i, (int32_t)p->x/PATHNODE_SIZE, (int32_t)p->y/PATHNODE_SIZE,
				(int32_t)p->x, (int32_t)p->y, (int32_t)(p->x-lastp->x), (int32_t)(p->y-lastp->y));

			InfoMess(msg,msg);

			lastp = p;
		}

		char m[123];
		sprintf(m, "subg %d,%d", (int32_t)pj->subgoal->x, (int32_t)pj->subgoal->y);
		InfoMess(m, m);
	}
#endif
}

//endnode going forward
 //endnode2 goes backward
void ReconstructPath2(PathJob* pj, PathNode* endnode, PathNode* endnode2)
{
	pj->path->clear();

	UType* ut = &g_utype[pj->utype];

#if 1
	// Reconstruct the path, following the path steps
	//Follow the path forward from the start, adding to the front of the path list
	for(PathNode* n = endnode; n; n = n->prev)
	{
#ifndef HASHPOOL
		Vec2i npos = PATHNODEPOS(n);
#else
		Vec2i npos;
		npos.x = n->index % g_pathdim.x;
		npos.y = n->index / g_pathdim.x;
#endif

		Vec2i cmpos( npos.x * PATHNODE_SIZE + PathOff(ut->size.x), npos.y * PATHNODE_SIZE + PathOff(ut->size.x) );
		pj->path->push_front(cmpos);
	}
#endif
	
	//todo use custom list implementation to make splicing and just stealing pointers possible to avoiding copying data
	//todo use bit field with 3/4 bits for 8 directions for pathnode->prev pointer, extra bit to show no parent

#if 1
	// Now follow the path backwards, adding to the back of path list
	for(PathNode* n = endnode2; n; n = n->prev)
	{
#ifndef HASHPOOL
		Vec2i npos = PATHNODEPOS(n);
#else
		Vec2i npos;
		npos.x = n->index % g_pathdim.x;
		npos.y = n->index / g_pathdim.x;
#endif

		Vec2i cmpos( npos.x * PATHNODE_SIZE + PathOff(ut->size.x), npos.y * PATHNODE_SIZE + PathOff(ut->size.x) );
		pj->path->push_back(cmpos);
	}
#endif

	//todo use path vector instead of list and reserve size before adding elements

#if 1	//necessary for exact point
	//pj->path->push_back(Vec2i(pj->goalx, pj->goaly)*PATHNODE_SIZE + PathOff(ut->size.x));
	if(pj->capend)
		pj->path->push_back(pj->cmgoal);
#endif

	if(pj->path->size() > 0)
		*pj->subgoal = *pj->path->begin();
}
