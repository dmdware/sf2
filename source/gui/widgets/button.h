










#ifndef BUTTON_H
#define BUTTON_H

#include "../widget.h"

/* styles */
#define BUST_CORRODE			0
#define BUST_LINEBASED		1
#define BUST_LEFTIMAGE		2

struct Button
{
	Widget base;

	char style;
};

Button();
Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip, int f, int style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int p), void (*overf)(), void (*overf2)(int p), void (*out)(), int parm, void (*click3)(Widget* w));

virtual void draw();
virtual void drawover();
virtual void inev(InEv* ie);

#endif
