










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
#include "pathjob.h"

// Calculates a path.
void JPSPartPath(int32_t utype, int32_t umode, int32_t cmstartx, int32_t cmstarty, int32_t target, int32_t target2, int32_t targtype,
                 std::list<Vec2i> *path, Vec2i *subgoal, Unit* thisu, Unit* targu, Building* targb,
                 int32_t cmgoalx, int32_t cmgoaly, int32_t cmgoalminx, int32_t cmgoalminy, int32_t cmgoalmaxx, int32_t cmgoalmaxy,
                 int32_t maxsearch)
{
	UType* ut = &g_utype[utype];

	PathJob* pj = new PathJob;
	pj->utype = utype;
	pj->umode = umode;
	pj->cmstartx = cmstartx;
	pj->cmstarty = cmstarty;
	pj->target = target;
	pj->target2 = target2;
	pj->targtype = targtype;
	pj->path = path;
	pj->subgoal = subgoal;
	pj->thisu = thisu ? thisu - g_unit : -1;
	pj->targu = targu ? targu - g_unit : -1;
	pj->targb = targb ? targb - g_building : -1;
	pj->goalx = (cmgoalminx+cmgoalmaxx)/2;
	pj->goaly = (cmgoalminy+cmgoalmaxy)/2;
	pj->goalx = pj->goalx / PATHNODE_SIZE;
	pj->goaly = pj->goaly / PATHNODE_SIZE;
	pj->goalminx = cmgoalminx;
	pj->goalminy = cmgoalminy;
	pj->goalmaxx = cmgoalmaxx;
	pj->goalmaxy = cmgoalmaxy;
	pj->roaded = ut->roaded;
	pj->landborne = ut->landborne;
	pj->seaborne = ut->seaborne;
	pj->airborne = ut->airborne;
	pj->callback = Callback_UnitPath;
	pj->pjtype = PATHJOB_JPSPART;
	pj->maxsearch = maxsearch;
	pj->cmgoal = Vec2i(cmgoalx, cmgoaly);

#if 0
	int32_t sqmax = sqrt(maxsearch);
	int32_t diagw = (sqmax - 1) / 2;

	pj->maxsubdiag = diagw;
	pj->maxsubdiagstraight = diagw;
	pj->maxsubstraight = diagw;
#endif

	// Returns the path from location `<startX, startY>` to location `<endX, endY>`.
	//return function(finder, startNode, endNode, clearance, toClear)

	pj->process();
	delete pj;
}
