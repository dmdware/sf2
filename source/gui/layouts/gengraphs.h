










#ifndef GENGRAPHS_H
#define GENGRAPHS_H

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

#define GENGRAPH_TOTSAT		0	//total satiety
#define GENGRAPH_AVINSAT		1	//average individual satiety
#define GENGRAPH_TOTHFUN		2	//total household funds
#define GENGRAPH_AVINFUN		3	//average individual funds
#define GENGRAPH_POP			4	//population
#define GENGRAPH_TYPES			5

extern const char *GENGRAPHSTR[GENGRAPH_TYPES];

class Stat
{
public:
	std::list<int32_t> fig;	//figure
};

extern Stat g_genstats[GENGRAPH_TYPES];

class GenGraphs : public Win
{
public:
	GenGraphs(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_close;
	InsDraw m_graphs;
	DropList m_rsel;

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
	void regvals(Selection* sel);
	virtual void reframe();
	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);
};

void RecStats();

#endif
