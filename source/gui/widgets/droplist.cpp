










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
#include "../gui.h"

DropList::DropList() : Widget()
{
	parent = NULL;
	type = WIDGET_DROPLIST;
	name = "";
	font = MAINFONT8;
	opened = ecfalse;
	selected = 0;
	scroll[1] = 0;
	mousescroll = ecfalse;
	ldown = ecfalse;
	changefunc = NULL;
	reframefunc = NULL;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/filled.jpg", ectrue, ecfalse);
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	//CreateTex(downtex, "gui/down.jpg", ectrue, ecfalse);
	reframe();
}

DropList::DropList(Widget* parent, const char* n, int f, void (*reframef)(Widget* w), void (*change)()) : Widget()
{
	parent = parent;
	type = WIDGET_DROPLIST;
	name = n;
	font = f;
	opened = ecfalse;
	selected = 0;
	scroll[1] = 0;
	mousescroll = ecfalse;
	ldown = ecfalse;
	changefunc = change;
	reframefunc = reframef;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/filled.jpg", ectrue, ecfalse);
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	//CreateTex(downtex, "gui/down.jpg", ectrue, ecfalse);
	reframe();
}

void DropList::erase(int which)
{
	options.erase( options.begin() + which );
	if(selected == which)
		selected = -1;

	if(scroll[1] + rowsshown() > options.size())
		scroll[1] = options.size() - (float)rowsshown();

	if(scroll[1] < 0)
		scroll[1] = 0;
}

int DropList::rowsshown()
{
	int rows = MAX_OPTIONS_SHOWN;

	if(rows > options.size())
		rows = options.size();

	return rows;
}

int DropList::square()
{
	return (int)g_font[font].gheight;
}

float DropList::scrollspace()
{
	return g_font[font].gheight*(rowsshown())-square();
}

//TODO draw using lines

void DropList::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[frametex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	if(!opened)
		DrawImage(g_texture[uptex].texname, pos[2]-square(), pos[1], pos[2], pos[1]+square(), 0, 1, 1, 0, crop);

	if(options.size() <= 0)
		return;

	if(selected < 0)
	{
		//if(options.size() <= 0)
		//	return;

		//DrawShadowedText(font, pos[0]+3, pos[1], options[0].c_str());

		return;
	}

	if(selected >= (int)options.size())
		return;

	DrawShadowedTextF(font, pos[0]+3, pos[1], crop[0], crop[1], crop[2], crop[3], &options[selected], NULL, -1);
}

void DropList::drawover()
{
	if(!opened)
		return;

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	Font* f = &g_font[font];

	float drop[4];
	float dropsign;
	int dropedge;

	float drfy[2];	//drop frame box y
	float scary[2];	//scroll area y
	float bary[2];
	float upy[2];
	float downy[2];

	//if(opened)
	{
		if(downdrop)
		{
			drop[0] = pos[0];
			drop[1] = pos[1];
			drop[2] = pos[2];
			drop[3] = pos[3] + rowsshown() * f->gheight;
			dropsign = 1;
			dropedge = 3;
			
			drfy[0] = drop[1]+f->gheight;
			drfy[1] = drop[3];
			scary[0] = drop[1]+f->gheight;
			scary[1] = drop[3];
			upy[0] = drop[1];
			upy[1] = drop[1]+square();
			downy[0] = drop[3]-square();
			downy[1] = drop[3];
			bary[0] = drop[1]+f->gheight+scrollspace()*topratio();
			bary[1] = drop[1]+f->gheight+scrollspace()*bottomratio();
		}
		else
		{
			drop[0] = pos[0];
			drop[1] = pos[1] - rowsshown() * f->gheight;
			drop[2] = pos[2];
			drop[3] = pos[3];
			dropsign = -1;
			dropedge = 1;
			
			drfy[0] = drop[1];
			drfy[1] = drop[3]-f->gheight;
			scary[0] = drop[1]+f->gheight;
			scary[1] = drop[3];
			upy[0] = drop[1];
			upy[1] = drop[1]+square();
			downy[0] = drop[3]-square();
			downy[1] = drop[3];
			bary[0] = drop[1]+f->gheight+scrollspace()*topratio();
			bary[1] = drop[1]+f->gheight+scrollspace()*bottomratio();
		}
	}

	//downdrop = ectrue;

	//if(downdrop)
	{
		//dropdown frame box
		DrawImage(g_texture[frametex].texname, 
			pos[0], drfy[0], pos[2], drfy[1],
			0,0,1,1, g_gui.crop);

		//scroll area box
		DrawImage(g_texture[frametex].texname, 
			pos[2]-square(), scary[0], pos[2], scary[1], 
			0,0,1,1, g_gui.crop);

		//up scroll button
		DrawImage(g_texture[uptex].texname, 
			pos[2]-square(), upy[0], pos[2], upy[1],
			0,0,1,1, g_gui.crop);

		//down scroll button
		DrawImage(g_texture[uptex].texname, 
			pos[2]-square(), downy[0], pos[2], downy[1], 
			0, 1, 1, 0, g_gui.crop);

#if 1
		//scroll bar
		DrawImage(g_texture[filledtex].texname, 
			pos[2]-square(), bary[0], pos[2], bary[1], 
			0,0,1,1, g_gui.crop);

		for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
			//DrawShadowedText(font, pos[0]+3, pos[3]+f->gheight*(i-(int)scroll[1]), &options[i]);
			DrawShadowedTextF(font, pos[0]+3, drfy[0]+f->gheight*(i-(int)scroll[1]), pos[0], drop[1], pos[2], drop[3], &options[i]);
#endif
	}
}

void DropList::inev(InEv* ie)
{
	Font* f = &g_font[font];

	float drop[4];
	float dropsign;
	int dropedge;

	float drfy[2];	//drop frame box y
	float scary[2];	//scroll area y
	float bary[2];
	float upy[2];
	float downy[2];

	if(opened)
	{
		if(downdrop)
		{
			drop[0] = pos[0];
			drop[1] = pos[1];
			drop[2] = pos[2];
			drop[3] = pos[3] + rowsshown() * f->gheight;
			dropsign = 1;
			dropedge = 3;
			
			drfy[0] = drop[1]+f->gheight;
			drfy[1] = drop[3];
			scary[0] = drop[1]+f->gheight;
			scary[1] = drop[3];
			upy[0] = drop[1];
			upy[1] = drop[1]+square();
			downy[0] = drop[3]-square();
			downy[1] = drop[3];
			bary[0] = drop[1]+f->gheight+scrollspace()*topratio();
			bary[1] = drop[1]+f->gheight+scrollspace()*bottomratio();
		}
		else
		{
			drop[0] = pos[0];
			drop[1] = pos[1] - rowsshown() * f->gheight;
			drop[2] = pos[2];
			drop[3] = pos[3];
			dropsign = -1;
			dropedge = 1;
			
			drfy[0] = drop[1];
			drfy[1] = drop[3]-f->gheight;
			scary[0] = drop[1]+f->gheight;
			scary[1] = drop[3];
			upy[0] = drop[1];
			upy[1] = drop[1]+square();
			downy[0] = drop[3]-square();
			downy[1] = drop[3];
			bary[0] = drop[1]+f->gheight+scrollspace()*topratio();
			bary[1] = drop[1]+f->gheight+scrollspace()*bottomratio();
		}
	}

	if(ie->type == INEV_MOUSEWHEEL && !ie->intercepted)
	{
		if(opened)
		{
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}
	}
	// corpd fix
	else if(ie->type == INEV_MOUSEMOVE && (!ie->intercepted || mousescroll))
	{
		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
		{
			g_mouseoveraction =  ectrue;
			//windows/msvs2012 still allowed droplist drop down even when the following line wasn't there wtf
			//but mac didn't
			over = ectrue;
		}

#if 0
		if(opened)
		{
			Font* f = &g_font[font];

			//on dropped list?
			if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[1] + f->gheight*rowsshown())
			{
				g_mouseoveraction =  ectrue;
				over = ectrue;
				ie->intercepted = ectrue;
			}
		}
#endif

		//corpd fix corpc fix
		if(opened)
		{
			for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
			{
				// std::list item?
				if(g_mouse.x >= pos[0] && 
					g_mouse.x <= pos[2]-square() && 
					g_mouse.y >= drfy[0]+f->gheight*(i-(int)scroll[1]) && 
					g_mouse.y <= drfy[0]+f->gheight*(i-(int)scroll[1]+1))
				{
					ie->intercepted = ectrue;
				}
			}

			// scroll bar?
			if(g_mouse.x >= pos[2]-square() && 
				g_mouse.y >= bary[0] && 
				g_mouse.x <= pos[2] &&
				g_mouse.y <= bary[1])
			{
				ie->intercepted = ectrue;
			}

			// up button?
			if(g_mouse.x >= pos[2]-square() &&
				g_mouse.y >= upy[0] &&
				g_mouse.x <= pos[2] &&
				g_mouse.y <= upy[1])
			{
				ie->intercepted = ectrue;
			}

			// down button?
			if(g_mouse.x >= pos[2]-square() &&
				g_mouse.y >= downy[0] &&
				g_mouse.x <= pos[2] &&
				g_mouse.y <= downy[1])
			{
				ie->intercepted = ectrue;
			}
		}
		if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
		{
			ie->intercepted = ectrue;
		}

		if(ldown)
			ie->intercepted = ectrue;

		if(!mousescroll)
			return;

		g_mouseoveraction =  ectrue;

		int dy = g_mouse.y - mousedown[1];
		float topy = scary[dropedge/2]+square()+scrollspace()*topratio();
		float newtopy = topy + dy;

		//topratio = (float)scroll / (float)(options.size());
		//topy = pos[3]+square+scrollspace*topratio
		//topy = pos[3]+square+scrollspace*((float)scroll / (float)(options.size()))
		//topy - pos[3] - square = scrollspace*(float)scroll / (float)(options.size())
		//(topy - pos[3] - square)*(float)(options.size())/scrollspace = scroll

		scroll[1] = (newtopy - scary[dropedge/2] - square())*(float)(options.size())/scrollspace();

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
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
	{
		//InfoMess("dlld", "dlld");

#if 0
		if(over)
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
		}
#endif

		if(opened)
		{
			for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
			{
				// std::list item?
				if(g_mouse.x >= pos[0] && 
					g_mouse.x <= pos[2]-square() && 
					g_mouse.y >= drfy[0]+f->gheight*(i-(int)scroll[1]) && 
					g_mouse.y <= drfy[0]+f->gheight*(i-(int)scroll[1]+1))
				{
					ldown = ectrue;
					ie->intercepted = ectrue;
					return;	// intercept mouse event
				}
			}

			// scroll bar?
			if(g_mouse.x >= pos[2]-square() && 
				g_mouse.y >= bary[0] && 
				g_mouse.x <= pos[2] &&
				g_mouse.y <= bary[1])
			{
				ldown = ectrue;
				mousescroll = ectrue;
				mousedown[1] = g_mouse.y;
				ie->intercepted = ectrue;
				return;	// intercept mouse event
			}

			// up button?
			if(g_mouse.x >= pos[2]-square() && 
				g_mouse.y >= upy[0] && 
				g_mouse.x <= pos[2] && 
				g_mouse.y <= upy[1])
			{
				ldown = ectrue;
				ie->intercepted = ectrue;
				return;
			}

			// down button?
			if(g_mouse.x >= pos[2]-square() && 
				g_mouse.y >= downy[0] && 
				g_mouse.x <= pos[2] && 
				g_mouse.y <= downy[1])
			{
				ldown = ectrue;
				ie->intercepted = ectrue;
				return;
			}

			ie->intercepted = ectrue;
		}

		if(!ie->intercepted)
		{
			if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[3])
			{
				ldown = ectrue;
				ie->intercepted = ectrue;
				return;
			}
		}
		
		//corpd fix xp
		if(!ldown)
		{
			opened = ecfalse;

			return;
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
	{
		//InfoMess("dllu", "dllu");

#if 0
		if(over)
		{
			ie->intercepted = ectrue;
		}
#endif

		if(opened)
		{
#if 0	//wtf msvs still worked but mac didn't (mac version is correct)
			if(!ldown)
			{
				opened = ecfalse;
				return;
			}
#endif
	
			//corpd fix
			//did some other widget intercept?
			if(ie->intercepted && 
				ie->interceptor != this)
				opened = ecfalse;
			
			if(ldown)
			{
				ie->intercepted = ectrue;
				ie->interceptor = this;
				ldown = ecfalse;
			}

			if(mousescroll)
			{
				mousescroll = ecfalse;
				ie->intercepted = ectrue;
				ie->interceptor = this;
				return;	// intercept mouse event
			}

			for(int i=(int)scroll[1]; i<(int)scroll[1]+rowsshown(); i++)
			{
				// std::list item?
				if(g_mouse.x >= pos[0] &&
					g_mouse.x <= pos[2]-square() &&
					g_mouse.y >= drfy[0]+f->gheight*(i-(int)scroll[1]) && 
					g_mouse.y <= drfy[0]+f->gheight*(i-(int)scroll[1]+1))
				{
					selected = i;
					opened = ecfalse;
					if(changefunc != NULL)
						changefunc();

					ie->intercepted = ectrue;
					ie->interceptor = this;
					return;	// intercept mouse event
				}
			}

			// up button?
			if(g_mouse.x >= pos[2]-square() &&
				g_mouse.y >= upy[0] &&
				g_mouse.x <= pos[2] &&
				g_mouse.y <= upy[1])
			{
				scroll[1]--;
				if(scroll[1] < 0)
					scroll[1] = 0;

				ie->intercepted = ectrue;
				ie->interceptor = this;
				return;
			}

			// down button?
			if(g_mouse.x >= pos[2]-square() &&
				g_mouse.y >= downy[0] &&
				g_mouse.x <= pos[2] &&
				g_mouse.y <= downy[1])
			{
				scroll[1]++;
				if(scroll[1]+rowsshown() > options.size())
					scroll[1] = options.size() - rowsshown();

				ie->intercepted = ectrue;
				ie->interceptor = this;
				return;
			}
			
			//corpd fix
			//was it outside of this widget?
			if(!ie->intercepted)
				opened = ecfalse;

			ie->intercepted = ectrue;	// intercept mouse event
			ie->interceptor = this;
		}
		//!opened
		else if(!ie->intercepted)
		{
			if(!ldown)
				return;

			ldown = ecfalse;

			if(g_mouse.x >= pos[2]-square() && g_mouse.y >= pos[1] && g_mouse.x <= pos[2] && g_mouse.y <= pos[1]+square())
			{
				opened = ectrue;
				ie->intercepted = ectrue;
				ie->interceptor = this;

				float* parf = g_gui.crop; //parent->pos;
				//see whether up or down has more room
				if(fabs(parf[1] - pos[1]) > fabs(parf[3] - pos[3]))
					downdrop = ecfalse;
				else
					downdrop = ectrue;

				//Need to bring tree to front so that drop-down list gets 
				//the mouse up event first instead of any item in the background.
				Widget* parw = parent;
				while(parw)
				{
					parw->tofront();
					parw = parw->parent;
				}
				tofront();

				return;
			}
		}
	}
}
