
#ifndef ROLEVIEW_H
#define ROLEVIEW_H

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

class RoleView : public Win
{
public:
	RoleView(Widget* parent, const char* n, void (*reframef)(Widget* w));

	void regen();
};

//TODO move change all spez widgets to layouts


#endif