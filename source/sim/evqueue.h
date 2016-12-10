











#ifndef EVQUEUE_H
#define EVQUEUE_H

#include "../platform.h"

//sim event
struct SimEv
{
public:
	//simframes:
	unsigned __int64 planframe;
	unsigned __int64 execframe;
	unsigned short evtype;
};

/*
the whole point of these is to avoid looping 
over each unit to update them. each unit 
that must be moved or have anything done is
added to this queue.
 emphasis is on actions that take an hour or
 more to complete, e.g., working off a man-hour
 of work.
*/

#define EV_WORKHOUR_BL			0
#define EV_WORKHOUR_BL_CS		1
#define EV_WORKHOUR_CD_CS		2
#define EV_RESTHOUR				3
#define EV_SHOPHOUR				4
#define EV_WORKHOUR_DV			5
//maybe combine move and upd?
#define EV_MOVEANDUPD			6
//for trucks that are standing still or workers without anything to do:
#define EV_UPD					7	//generic update, check state, unit mode switch block, before/after move in the same simframe?
//are following really hour-based?
#define EV_DROPOFFSUP_BL		8
#define EV_PICKUPSUP			9
#define EV_DROPOFFSUP_CD		10
#define EV_DROPOFFSUP_BL_CS		11

/*
must be in order of when they are to be exec'd
and must be placed deterministically
i.e., placed after all those of the same execframe,
but right before the first greater execframe!
(in sub-order of when they are planned/scheduled.)
basically, their order in the queue must be the 
same as when they would be executed if the 
nomral game loop would execute them each frame.
*/
extern std::list<SimEv> g_simev;

struct Mv;

void PlanEvent(unsigned short type, unsigned __int64 execframe, Mv* mv);
void ExecEvent(SimEv* se);
void ExecEvents();

#endif