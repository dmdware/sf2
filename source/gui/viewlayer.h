










#ifndef VIEWLAYER_H
#define VIEWLAYER_H

#include "widget.h"

struct ViewLayer
{
	Widget base;
};

void ViewLayer_init(ViewLayer *vl, Widget* parent, const char* n);
void ViewLayer_reframe(ViewLayer *vl);
void ViewLayer_show(ViewLayer *vl);

#endif
