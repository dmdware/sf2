










#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include "../widget.h"

struct TextBlock : public Widget
{
public:

	TextBlock()
	{
		parent = NULL;
		type = WIDGET_TEXTBLOCK;
		name = "";
		text = "";
		font = 0;
		reframefunc = NULL;
		ldown = ecfalse;
		rgba[0] = 1;
		rgba[1] = 1;
		rgba[2] = 1;
		rgba[3] = 1;
	}

	TextBlock(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), float r=1, float g=1, float b=1, float a=1) : Widget()
	{
		parent = parent;
		type = WIDGET_TEXTBLOCK;
		name = n;
		text = t;
		font = f;
		reframefunc = reframef;
		ldown = ecfalse;
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
		rgba[3] = a;
		reframe();
	}

	void draw();
	//ecbool lbuttonup(ecbool moved);
	//ecbool lbuttondown();
	//ecbool mousemove();
	void changevalue(const char* newv);
	int square();
};

#endif
