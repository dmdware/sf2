










#ifndef LINK_H
#define LINK_H

#include "../widget.h"

struct Link : public Widget
{
public:
	Link(Widget* parent, const char* name, const RichText t, int f, void (*reframef)(Widget* w), void (*click)()) : Widget()
	{
		parent = parent;
		type = WIDGET_LINK;
		name = name;
		over = ecfalse;
		ldown = ecfalse;
		text = t;
		font = f;
		reframefunc = reframef;
		clickfunc = click;
		reframe();
	}

	void draw();
	void inev(InEv* ie);
};

#endif
