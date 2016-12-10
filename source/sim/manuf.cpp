













#include "manuf.h"
#include "building.h"
#include "bltype.h"
#include "unit.h"
#include "mvtype.h"
#include "player.h"
#include "../net/client.h"

void OrderMan(int mvtype, int bi, int player)
{
	ManufJob mj;
	Bl* b = &g_bl[bi];
	mj.owner = player;
	mj.mvtype = mvtype;
	b->manufjob.push_back(mj);
	b->trymanuf();
	b->manufjob.clear();
}
