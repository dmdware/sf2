










#ifndef PANE_H
#define PANE_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"
#include "vscrollbar.h"

struct Pane : public Widget
{
public:

	VScroll vscroll;

	float minsz[2];
	int mousedown[2];

	Pane();
	Pane(Widget* parent, const char* n, void (*reframef)(Widget* w));

	void inev(InEv* ie);
	void draw();
	void drawover();
	void reframe();
	void chcall(Widget* ch, int type, void* data);
	void subframe(float* fr);
};

#endif
