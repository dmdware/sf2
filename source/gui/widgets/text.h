










#ifndef TEXT_H
#define TEXT_H

#include "../widget.h"

struct Text : public Widget
{
public:
	Text()
	{
		parent = NULL;
		type = WIDGET_TEXT;
		name = "";
		//text = "";
		font = MAINFONT8;
		reframefunc = NULL;
		ldown = ecfalse;
		rgba[0] = 1;
		rgba[1] = 1;
		rgba[2] = 1;
		rgba[3] = 1;
		shadow = ecfalse;
	}

	Text(Widget* parent, const char* n, const RichText& t, int f, void (*reframef)(Widget* w), ecbool shdw=ectrue, float r=0.8f, float g=0.8f, float b=0.8f, float a=1) : Widget()
	{
		parent = parent;
		type = WIDGET_TEXT;
		name = n;
		//Log("t.rawstr "<<t.rawstr());
		//

#ifdef USTR_DEBUG
		Log("Text(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), ecbool shdw=ectrue, float r=0.8f, float g=0.8f, float b=0.8f, float a=1) : Widget()");
		
#endif

		text = t;

#ifdef USTR_DEBUG
		Log("text end assign");
		
#endif

		font = f;
		reframefunc = reframef;
		ldown = ecfalse;
		//rgba[0] = rgba[1] = rgba[2] = 0.8f;
		//rgba[3] = 1.0f;
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
		rgba[3] = a;
		shadow = shdw;
		reframe();
	}

	void draw();
};

#endif
