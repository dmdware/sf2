










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


ListBox::ListBox(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)()) : Widget()
{
	parent = parent;
	type = WIDGET_LISTBOX;
	name = n;
	font = f;
	reframefunc = reframef;
	opened = ecfalse;
	selected = -1;
	scroll[1] = 0;
	mousescroll = ecfalse;
	ldown = ecfalse;
	changefunc = change;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/filled.jpg", ectrue, ecfalse);
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	//CreateTex(downtex, "gui/down.jpg", ectrue, ecfalse);
	reframe();
}

void ListBox::erase(int which)
{
	options.erase( options.begin() + which );
	if(selected == which)
		selected = -1;

	if(scroll[1] + rowsshown() > options.size())
		scroll[1] = options.size() - (float)rowsshown();

	if(scroll[1] < 0)
		scroll[1] = 0;
}

int ListBox::rowsshown()
{
	int rows = (int)( (pos[3]-pos[1])/g_font[font].gheight );

	if(rows > (int)options.size())
		rows = options.size();

	return rows;
}

int ListBox::square()
{
	return (int)g_font[font].gheight;
}

float ListBox::scrollspace()
{
	return (pos[3]-pos[1]-square()*2);
}

void ListBox::draw()
{
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	Font* f = &g_font[font];
	int rows = rowsshown();

	DrawImage(g_texture[frametex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	DrawImage(g_texture[frametex].texname, pos[2]-square(), pos[1], pos[2], pos[3], 0,0,1,1, crop);
	DrawImage(g_texture[uptex].texname, pos[2]-square(), pos[1], pos[2], pos[1]+square(), 0,0,1,1, crop);
	DrawImage(g_texture[uptex].texname, pos[2]-square(), pos[3]-square(), pos[2], pos[3], 0, 1, 1, 0, crop);
	//DrawImage(g_texture[downtex].texname, pos[2]-square(), pos[3]-square(), pos[2], pos[3]);
	DrawImage(g_texture[filledtex].texname, pos[2]-square(), pos[1]+square()+scrollspace()*topratio(), pos[2], pos[1]+square()+scrollspace()*bottomratio(), 0,0,1,1, crop);

	if(selected >= 0 && selected >= (int)scroll[1] && selected < (int)scroll[1]+rowsshown())
	{
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 0.5f);
		DrawImage(g_texture[filledtex].texname, pos[0], pos[1]+(selected-(int)scroll[1])*f->gheight, pos[2]-square(), pos[1]+(selected-(int)scroll[1]+1)*f->gheight, 0,0,1,1, crop);
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
	}

	for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
		DrawShadowedText(font, pos[0]+3, pos[1]+g_font[font].gheight*(i-(int)scroll[1]), &options[i]);
}

void ListBox::inev(InEv* ie)
{
	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(!mousescroll)
			return;

		int dy = g_mouse.y - mousedown[1];

		float topy = pos[3]+square()+scrollspace()*topratio();
		float newtopy = topy + dy;

		//topratio = (float)scroll / (float)(options.size());
		//topy = pos[3]+square+scrollspace*topratio
		//topy = pos[3]+square+scrollspace*((float)scroll / (float)(options.size()))
		//topy - pos[3] - square = scrollspace*(float)scroll / (float)(options.size())
		//(topy - pos[3] - square)*(float)(options.size())/scrollspace = scroll

		scroll[1] = (newtopy - pos[3] - square())*(float)(options.size())/scrollspace();

		if(scroll[1] < 0)
		{
			scroll[1] = 0;
			ie->intercepted = ectrue;
			return;
		}
		else if(scroll[1] + rowsshown() > options.size())
		{
			scroll[1] = options.size() - (float)rowsshown();
			ie->intercepted = ectrue;
			return;
		}

		mousedown[1] = g_mouse.y;

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		Font* f = &g_font[font];

		for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
		{
			int row = i-(int)scroll[1];
			// std::list item?
			if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[1]+f->gheight*row
			                && g_mouse.y <= pos[1]+f->gheight*(row+1))
			{
				ldown = ectrue;
				ie->intercepted = ectrue;
				return;	// intercept mouse event
			}
		}

		// scroll bar?
		if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1]+square()+scrollspace()*topratio() && g_mouse.x <= pos[2] &&
		                g_mouse.y <= pos[1]+square()+scrollspace()*bottomratio())
		{
			ldown = ectrue;
			mousescroll = ectrue;
			mousedown[1] = g_mouse.y;
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}

		// up button?
		if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;
		}

		// down button?
		if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]-square() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(!ldown)
			return;

		ldown = ecfalse;

		if(mousescroll)
		{
			mousescroll = ecfalse;
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}

		Font* f = &g_font[font];

		for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
		{
			int row = i-(int)scroll[1];

			// std::list item?
			if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2]-square() && g_mouse.y >= pos[1]+f->gheight*row
			                && g_mouse.y <= pos[1]+f->gheight*(row+1))
			{
				selected = i;
				if(changefunc != NULL)
					changefunc();

				ie->intercepted = ectrue;
				return;	// intercept mouse event
			}
		}

		// up button?
		if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
		{
			if(rowsshown() < (int)((pos[3]-pos[1])/f->gheight))
			{
				ie->intercepted = ectrue;
				return;
			}

			scroll[1]--;
			if(scroll[1] < 0)
				scroll[1] = 0;

			ie->intercepted = ectrue;
			return;
		}

		// down button?
		if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[3]-square() && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
		{
			scroll[1]++;
			if(scroll[1]+rowsshown() > options.size())
				scroll[1] = options.size() - (float)rowsshown();

			ie->intercepted = ectrue;
			return;
		}

		ie->intercepted = ectrue;	// intercept mouse event
	}
}

