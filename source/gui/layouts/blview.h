










#ifndef BUILDINGVIEW_H
#define BUILDINGVIEW_H

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

class BlView : public Win
{
public:
	BlView(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Button m_close;
	Button m_set;
	InsDraw m_ucost;
	Button m_graphs;
	bool m_owned;	//is the building/property being displayed owned by local player?

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

void Resize_BV_Cl(Widget* w);
void Resize_BV_Cl_w(Widget* w);
void DrawUCost();
void DrawBlGraphs();

#endif
