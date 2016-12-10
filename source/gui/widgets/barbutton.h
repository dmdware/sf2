










#ifndef BARBUTTON_H
#define BARBUTTON_H

#include "../widget.h"
#include "button.h"

struct BarBut : public Button
{
public:
	float healthbar;

	BarBut(Widget* parent, unsigned int sprite, float bar, void (*reframef)(Widget* w), void (*click)(), void (*overf)(), void (*out)());

	void draw();
};

#endif
