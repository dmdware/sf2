










#ifndef TOUCHLISTENER_H
#define TOUCHLISTENER_H

#include "../widget.h"

struct TouchListener : public Widget
{
public:
	TouchListener();
	TouchListener(Widget* parent, const char* name, void (*reframef)(Widget* w), void (*click2)(int p), void (*overf)(int p), void (*out)(), int parm);

	void inev(InEv* ie);
};

#endif
