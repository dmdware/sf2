










#include "../gui.h"
#include "../../texture.h"

#include "pane.h"
#include "../../debug.h"

Pane::Pane() : Widget()
{
	type = WIDGET_WINDOW;
	parent = NULL;
	opened = ecfalse;
}

void Pane::chcall(Widget* ch, int type, void* data)
{
	if(ch == &vscroll)
	{

	}
	//else if(ch == &hscroll)
	{
	}
}

Pane::Pane(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Widget()
{
	type = WIDGET_WINDOW;
	reframefunc = reframef;
	parent = parent;
	name = n;
	opened = ecfalse;
	scroll[0] = 0;
	scroll[1] = 0;

	const float alpha = 0.9f;

	CHECKGLERROR();

	vscroll = VScroll(this, "vscroll");

	// only call this when window is created
	if(reframefunc)
		reframefunc(this);
	else //default size and position
	{
		// to do
	}
	
	if(reframefunc)
		reframefunc(this);

	reframe();
}

void Pane::reframe()
{
	//if(reframefunc)
	//	reframefunc(this);

	vscroll.pos[0] = pos[2] - 3 - 10;
	vscroll.pos[1] = pos[1] + 27;
	vscroll.pos[2] = pos[2] - 3;
	vscroll.pos[3] = pos[3] - 27 - 10;
	vscroll.reframe();

	for(std::list<Widget*>::iterator w=sub.begin(); w!=sub.end(); w++)
		(*w)->reframe();
}

void Pane::draw()
{
	//vscroll.draw();

	Widget::draw();
}

void Pane::drawover()
{
	//vscroll.drawover();

	Widget::drawover();
}

void Pane::inev(InEv* ie)
{
	Widget::inev(ie);

	//vscroll.inev(ie);

	if(ldown)
	{
		if(ie->type == INEV_MOUSEMOVE ||
		                ( (ie->type == INEV_MOUSEDOWN || ie->type == INEV_MOUSEUP) && ie->key == MOUSE_LEFT) )
			ie->intercepted = ectrue;

		if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
			ldown = ecfalse;

		if(ie->type == INEV_MOUSEMOVE)
		{
			int dx = g_mouse.x - mousedown[0];
			int dy = g_mouse.y - mousedown[1];
			mousedown[0] = g_mouse.x;
			mousedown[1] = g_mouse.y;
		}
	}

	if(over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			mousedown[0] = g_mouse.x;
			mousedown[1] = g_mouse.y;
			ldown = ectrue;
			ie->intercepted = ectrue;
			tofront();	//can't change list order, breaks iterators
		}
	}

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(ldown)
		{
			ie->intercepted = ectrue;
			ie->curst = g_curst;
			return;
		}

		if(!ie->intercepted &&
		                g_mouse.x >= pos[0]-64 &&
		                g_mouse.y >= pos[1]-64 &&
		                g_mouse.x <= pos[2]+64 &&
		                g_mouse.y <= pos[3]+64)
		{
			over = ectrue;

			if(g_mousekeys[MOUSE_MIDDLE])
				return;

			//ie->intercepted = ectrue;
		}
		else
		{
			//cursor out of window area?
			if(!ie->intercepted)
			{
				if(over)
				{
					//ie->intercepted = ectrue;
					//ie->curst = CU_DEFAULT;
				}
			}
			//event intercepted but cursor in window rectangle (maybe covered up by something else)?
			else
			{
				// to do: this will be replaced by code in other
				//widgets that will set the cursor
				//g_curst = CU_DEFAULT;
			}

			over = ecfalse;
		}
	}
}

void Pane::subframe(float* fr)
{
	memcpy(fr, pos, sizeof(float)*4);
}
