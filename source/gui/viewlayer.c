










#include "gui.h"
#include "../texture.h"

void ViewLayer_init(ViewLayer *vl, Widget* parent, const char* n)
{
	Widget* bw;

	bw = (Widget*)vl;
	Widget_init(bw);
	strcpy(bw->name, n);
	bw->type = WIDGET_VIEWLAYER;
	parent = parent;
	Widget_reframe(bw);
}

void ViewLayer_reframe(ViewLayer *vl)
{
	Widget* bw;

	bw = (Widget*)vl;
	bw->pos[0] = 0;
	bw->pos[1] = 0;
	bw->pos[2] = (float)(g_width-1);
	bw->pos[3] = (float)(g_height-1);
}

void ViewLayer_show(ViewLayer *vl)
{
	Widget *bw;
	bw = (Widget*)vl;
	/* necessary for window widgets: */
	Widget_tofront(bw);	/* can't break list iterator, might shift */
}

