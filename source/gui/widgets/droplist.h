










#ifndef DROPDOWNS_H
#define DROPDOWNS_H

#include "../widget.h"

struct DropList : public Widget
{
public:

	int mousedown[2];

	ecbool downdrop;

	DropList();
	DropList(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)());

	virtual void draw();
	virtual void drawover();
	virtual void inev(InEv* ie);

	int rowsshown();
	int square();
	void erase(int which);

	float topratio()
	{
		return scroll[1] / (float)(options.size());
	}

	float bottomratio()
	{
		return (scroll[1]+rowsshown()) / (float)(options.size());
	}

	float scrollspace();
};

#endif
