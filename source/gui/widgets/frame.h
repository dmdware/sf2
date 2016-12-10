










#ifndef FRAME_H
#define FRAME_H

#include "../widget.h"

struct Frame : public Widget
{
public:
	Frame(Widget* parent, const char* n, void (*reframef)(Widget* w));

	void draw();
	void drawover();
	void inev(InEv* ie);
	void frameupd();
};

#endif
