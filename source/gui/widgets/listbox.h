










#ifndef LISTBOX_H
#define LISTBOX_H

#include "../widget.h"

struct ListBox : public Widget
{
public:

	int mousedown[2];

	ListBox(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)());

	void draw();
	void inev(InEv* ie);
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
