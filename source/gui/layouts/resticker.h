










#ifndef RESTICKER_H
#define RESTICKER_H

#include "../../widget.h"

class ResTicker : public Widget
{
public:
	ResTicker(Widget* parent, const char* n, void (*reframef)(Widget* w));

	Text restext;
	Image leftinnerdiagblur;
	Image rightinnerdiagblur;
	Image innerbottom;
	Image lefthlineblur;
	Image righthlineblur;
	Image whitebg;

	void draw();
	void drawover();
	void reframe();
	void inev(InEv* ie);
	void frameupd();
};

#endif
