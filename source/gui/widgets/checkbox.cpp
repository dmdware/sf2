










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

CheckBox::CheckBox() : Widget()
{
	parent = NULL;
	type = WIDGET_CHECKBOX;
	name = "";
	text = "";
	font = MAINFONT16;
	reframefunc = NULL;
	ldown = ecfalse;
	rgba[0] = 1;
	rgba[1] = 1;
	rgba[2] = 1;
	rgba[3] = 1;
	selected = 0;
	changefunc = NULL;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/accept.png", ectrue, ecfalse);
	reframe();
}

CheckBox::CheckBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), int sel, float r, float g, float b, float a, void (*change)()) : Widget()
{
	parent = parent;
	type = WIDGET_CHECKBOX;
	name = n;
	text = t;
	font = f;
	reframefunc = reframef;
	ldown = ecfalse;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	selected = sel;
	changefunc = change;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/accept.png", ectrue, ecfalse);
	reframe();
}

int CheckBox::square()
{
	return (int)g_font[font].gheight;
}

void CheckBox::draw()
{
	DrawImage(g_texture[frametex].texname,  pos[0], pos[1], pos[0]+square(), pos[1]+square(), 0,0,1,1, crop);

	if(selected > 0)
		DrawImage(g_texture[filledtex].texname, pos[0], pos[1], pos[0]+square(), pos[1]+square(), 0,0,1,1, crop);

	DrawShadowedText(font, pos[0]+square()+5, pos[1], &text);
}

void CheckBox::inev(InEv* ie)
{
	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] &&
		                g_mouse.x <= pos[2] &&
		                g_mouse.y <= pos[3])
		{
			over = ectrue;
			ie->intercepted = ectrue;
			return;
		}
		else
		{
			over = ecfalse;
			return;
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(over)
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(over && ldown)
		{
			if(selected <= 0)
				selected = 1;
			else
				selected  = 0;

			if(changefunc != NULL)
				changefunc();

			ldown = ecfalse;

			ie->intercepted = ectrue;
			return;
		}

		ldown = ecfalse;
	}
}

