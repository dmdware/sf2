










#ifndef TRUCKMGR_H
#define TRUCKMGR_H

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

class TruckMgr : public Win
{
public:
	TruckMgr(Widget* parent, const char* n, void (*reframef)(Widget* w));

	//void draw();
	//void drawover();
	//void reframe();
	//void inev(InEv* ie);
	//void frameupd();
	void regen(Selection* sel);
};

#endif
