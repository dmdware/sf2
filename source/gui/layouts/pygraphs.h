










#ifndef PYGRAPHS_H
#define PYGRAPHS_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../winw.h"
#include "../../../sim/building.h"
#include "../../../sim/player.h"

#define PYGRAPH_FUNDS			0	//expenses and earnings. edit: only funds (single figure)
#define PYGRAPH_TOTLABEXP		1	//total labour expenses
#define PYGRAPH_WORKLABEXP		2	//work labour expenses
#define PYGRAPH_DREXP			3	//driver labour expenses
#define PYGRAPH_CSEXP			4	//construction labour expenses
#define PYGRAPH_REQINEX			5	//requisites inputs expenses
#define PYGRAPH_BLFIN_BEG		6	//expenses and earnings by building type (business type)
#define PYGRAPH_BLFIN_END		(PYGRAPH_BLFIN_BEG+BL_TYPES)
#define PYGRAPH_TYPES			(PYGRAPH_BLFIN_END)

extern char *PYGRAPHSTR[PYGRAPH_TYPES];

class PyGraph
{
public:
	std::list<int32_t> fig;	//figure
	std::list<Vec2i> dubfig;	//double figure

	//bool dubl;	//is it double (expenses and earnings / cycle history) or single (figures)
};

extern PyGraph g_pygraphs[PLAYERS][PYGRAPH_TYPES];

class PyGraphs : public Win
{
public:
	PyGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_close;
	InsDraw m_graphs;
	DropList m_rsel;

	void regen(Selection* sel);
	void regvals(Selection* sel);
	virtual void reframe();
	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};

void RecPyStats();

#endif
