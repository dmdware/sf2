










#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"
#include "../../sim/player.h"

TouchListener::TouchListener() : Widget()
{
	parent = NULL;
	type = WIDGET_TOUCHLISTENER;
	over = ecfalse;
	ldown = ecfalse;
	reframefunc = NULL;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = NULL;
	overfunc2 = NULL;
	outfunc = NULL;
	param = -1;
	name = "";
}

TouchListener::TouchListener(Widget* parent, const char* name, void (*reframef)(Widget* w), void (*click2)(int p), void (*overf2)(int p), void (*out)(), int parm) : Widget()
{
	parent = parent;
	type = WIDGET_TOUCHLISTENER;
	over = ecfalse;
	ldown = ecfalse;
	reframefunc = reframef;
	clickfunc = NULL;
	overfunc = NULL;
	clickfunc2 = click2;
	overfunc2 = overf2;
	outfunc = out;
	param = parm;
	name = name;
	reframe();
}

void TouchListener::inev(InEv* ie)
{
	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(over && ldown)
		{
			if(clickfunc != NULL)
				clickfunc();

			if(clickfunc2 != NULL)
				clickfunc2(param);

			over = ecfalse;
			ldown = ecfalse;

			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}

		if(ldown)
		{
			ldown = ecfalse;
			ie->intercepted = ectrue;
			return;
		}

		over = ecfalse;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(over)
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}
	}
	else if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
		{
			if(overfunc != NULL)
				overfunc();
			if(overfunc2 != NULL)
				overfunc2(param);

			over = ectrue;

			ie->intercepted = ectrue;
			return;
		}
		else
		{
			if(over && outfunc != NULL)
				outfunc();

			over = ecfalse;
		}
	}
}
