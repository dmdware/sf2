










#include "../gui.h"
#include "../../texture.h"

#include "winw.h"
#include "../../debug.h"

void Click_WinClose(Widget* w)
{
	w->parent->hide();
}

void Click_WinSize(Widget* w)
{
	Win* win = (Win*)w->parent;
	win->fullsize();
}

Win::Win() : Widget()
{
	type = WIDGET_WINDOW;
	parent = NULL;
	opened = ecfalse;
}

void Win::fullsize()
{
	float* parf = parent->pos;	//parent frame
	float outpos[4];
	fillout(outpos);

	if(outpos[0] == parf[0] &&
		outpos[1] == parf[1] &&
		outpos[2] == parf[2] &&
		outpos[3] == parf[3])
	{
		//already fullscreen, make smaller
		memcpy(pos, prevpos, sizeof(float)*4);
	}
	else
	{
		//currently not fullscreen, make fullscreen
		memcpy(prevpos, pos, sizeof(float)*4);

		float border[4];
		border[0] = pos[0] - outpos[0];
		border[1] = pos[1] - outpos[1];
		border[2] = outpos[2] - pos[2];
		border[3] = outpos[3] - pos[3];
		
		pos[0] = border[0] + parf[0];
		pos[1] = border[1] + parf[1];
		pos[2] = parf[2] - border[2];
		pos[3] = parf[3] - border[3];
	}

	reframe();
}

void Win::chcall(Widget* ch, int type, void* data)
{
	if(ch == &vscroll)
	{

	}
	//else if(ch == &hscroll)
	{
	}
}

Win::Win(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Widget()
{
	type = WIDGET_WINDOW;
	reframefunc = reframef;
	parent = parent;
	name = n;
	scroll[0] = 0;
	scroll[1] = 0;
	//hidden = ectrue;
	hidden = ecfalse;

	const float alpha = 0.9f;

	CHECKGLERROR();

	bg_logo_image = Image(this, "", "gui/centerp/pcsgray.png", ectrue, NULL, 1, 1, 1, alpha/8.0f,		0, 0, 1, 1);

	vscroll = VScroll(this, "vscroll");

	title_text = Text(this, "title", RichText(""), FONT_EUROSTILE16, NULL, ectrue, 1, 1, 1, 1.0f);
	
	trclose = Button(this, "close", "gui/cancel.png", RichText(), RichText(), MAINFONT16, BUST_LEFTIMAGE, NULL, NULL, NULL, NULL, NULL, NULL, -1, Click_WinClose);
	trfull = Button(this, "size", "gui/fullsize.png", RichText(), RichText(), MAINFONT16, BUST_LEFTIMAGE, NULL, NULL, NULL, NULL, NULL, NULL, -1, Click_WinSize);

	// only call this when window is created
	if(reframefunc)
		reframefunc(this);
	else //default size and position
	{
		// to do
	}

	reframe();
}

void Win::reframe()
{
#if 1
	if(parent)
	{
		SubCrop(parent->crop, pos, crop);
	}
	else
	{
		crop[0] = 0;
		crop[1] = 0;
		crop[2] = (float)g_width-1;
		crop[3] = (float)g_height-1;
	}
#endif

	scar[0] = pos[0];
	scar[1] = pos[1];
	scar[2] = pos[2];
	scar[3] = pos[3];

	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->reframe();

	//crop[0] = 0;
	//crop[1] = 0;
	//crop[2] = (float)g_width-1;
	//crop[3] = (float)g_height-1;
	//fillout(crop);

	//vscroll.pos[0] = innerright - 3 - 10;
	//vscroll.pos[1] = innertop + 27;
	//vscroll.pos[2] = innerright - 3;
	//vscroll.pos[3] = innerbottom - 27 - 10;
	vscroll.reframe();

	//title_text.pos[0] = inner_top_mid_image.pos[0];
	//title_text.pos[1] = inner_top_mid_image.pos[1];
	//title_text.pos[2] = inner_top_mid_image.pos[2];
	//title_text.pos[3] = inner_top_mid_image.pos[1] + 32;
	title_text.reframe();

	float outpos[4];
	fillout(outpos);

	trclose.pos[0] = outpos[2] - (pos[1]-outpos[1]) * 2;
	trclose.pos[1] = outpos[1];
	trclose.pos[2] = outpos[2] - (pos[1]-outpos[1]) * 1;
	trclose.pos[3] = pos[1];
	trclose.reframe();
	trclose.crop[0] = outpos[0];
	trclose.crop[1] = outpos[1];
	trclose.crop[2] = outpos[2];
	trclose.crop[3] = outpos[3];

	trfull.pos[0] = outpos[2] - (pos[1]-outpos[1]) * 3;
	trfull.pos[1] = outpos[1];
	trfull.pos[2] = outpos[2] - (pos[1]-outpos[1]) * 2;
	trfull.pos[3] = pos[1];
	trfull.reframe();
	trfull.crop[0] = outpos[0];
	trfull.crop[1] = outpos[1];
	trfull.crop[2] = outpos[2];
	trfull.crop[3] = outpos[3];

	float minsz = fmin((pos[2]-pos[0]),(pos[3]-pos[1]));

	bg_logo_image.pos[0] = (pos[0]+pos[2])/2.0f - minsz/2.0f;
	bg_logo_image.pos[1] = (pos[3]+pos[1])/2.0f - minsz/2.0f;
	bg_logo_image.pos[2] = (pos[0]+pos[2])/2.0f + minsz/2.0f;
	bg_logo_image.pos[3] = (pos[3]+pos[1])/2.0f + minsz/2.0f;
	bg_logo_image.reframe();

	scar[2] = scar[0];
	scar[3] = scar[1];
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
	{
		Widget* w = *i;
		scar[2] = fmax(scar[2], w->pos[2]);
		scar[3] = fmax(scar[3], w->pos[3]);
	}
}

void Win::fillout(float* outpos)
{
	Font* f = &g_font[MAINFONT16];
	outpos[0] = pos[0] - 7;
	outpos[1] = pos[1] - 7 - f->gheight;
	outpos[2] = pos[2] + 7;
	outpos[3] = pos[3] + 7;
}

void Win::draw()
{
	EndS();
	UseS(SHADER_COLOR2D);
	Shader* s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(s->slot[SSLOT_COLOR], 1,1,1,1);

	float outpos[4];

	fillout(outpos);

	DrawSquare(0.5f, 0.5f, 0.5f, 0.9f, outpos[0], outpos[1], outpos[2], outpos[3], outpos);
	
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, outpos[0], outpos[1], outpos[2], outpos[1], outpos);	//top line outer
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, outpos[0], outpos[1], outpos[0], outpos[3], outpos);	//left line outer
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, outpos[0], outpos[3], outpos[2], outpos[3], outpos);	//bottom line outer
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, outpos[2], outpos[1], outpos[2], outpos[3], outpos);	//right line outer

	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, pos[0]-1, pos[1]-1, pos[2]+1, pos[1]-1, outpos);	//top line inner
	DrawLine(0.4f, 0.4f, 0.4f, 0.9f, pos[0]-1, pos[1]-1, pos[0]-1, pos[3]+1, outpos);	//left line inner
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, pos[0]-1, pos[3]+1, pos[2]+1, pos[3]+1, outpos);	//bottom line inner
	DrawLine(0.6f, 0.6f, 0.6f, 0.9f, pos[2]+1, pos[1]-1, pos[2]+1, pos[3]+1, outpos);	//right line inner

	EndS();
	Ortho(g_width, g_height, 1, 1, 1, 1);

	//bg_logo_image.draw();

	//vscroll.draw();
	title_text.draw();
	trclose.draw();
	trfull.draw();
	
	Widget::draw();
}

void Win::drawover()
{
	//vscroll.drawover();

	Widget::drawover();
}

void Win::show()
{
	Widget::show();
	
	//necessary for window widgets:
	tofront();	//can't break list iterator, might shift
}

void Win::inev(InEv* ie)
{
	Widget::inev(ie);

	//vscroll.inev(ie);
	
	trclose.inev(ie);
	trfull.inev(ie);
	
	float outpos[4];
	fillout(outpos);

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

			if(g_curst == CU_MOVE)
			{
				pos[0] += dx;
				pos[1] += dy;
				pos[2] += dx;
				pos[3] += dy;

				if(pos[0] < 0)
				{
					pos[2] -= pos[0] - (float)0;
					pos[0] = (float)0;
				}
				if(pos[2] > (float)g_width)
				{
					pos[0] -= pos[2] - (float)g_width;
					pos[2] = (float)(g_width);
				}
				if(pos[1] < 0)
				{
					pos[3] -= pos[1];
					pos[1] = (float)64;
				}
				if(pos[3] > g_height)
				{
					pos[1] -= pos[3] - g_height;
					pos[3] = (float)(g_height);
				}
				
				memcpy(prevpos, pos, sizeof(float)*4);

				reframe();
			}
			else if(g_curst == CU_RESZT)
			{
				int newh = (int)( pos[3]-pos[1]-dy );
				if(newh < 0) newh = 0;
				pos[1] = pos[3] - newh;
				if(pos[1] < 0) pos[1] = (float)0;
				if(pos[1] > g_height) pos[1] = (float)(g_height);
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZB)
			{
				int newh = (int)( pos[3]-pos[1]+dy );
				if(newh < 0) newh = 0;
				pos[3] = pos[1] + newh;
				if(pos[3] < 0) pos[3] = (float)0;
				if(pos[3] > g_height) pos[3] = (float)(g_height);
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZL)
			{
				int neww = (int)( pos[2]-pos[0]-dx );
				if(neww < 0) neww = 0;
				pos[0] = pos[2] - neww;
				if(pos[0] < 0) pos[0] = (float)0;
				if(pos[0] > g_width) pos[0] = (float)(g_width);
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZR)
			{
				int neww = (int)( pos[2]-pos[0]+dx );
				if(neww < 0) neww = 0;
				pos[2] = pos[0] + neww;
				if(pos[2] < 0) pos[2] = (float)0;
				if(pos[2] > g_width) pos[2] = (float)(g_width);
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZTL)
			{
				int newh = (int)( pos[3]-pos[1]-dy );
				if(newh < 0) newh = 0;
				pos[1] = pos[3] - newh;
				if(pos[1] < 0) pos[1] = (float)0;
				if(pos[1] > g_height) pos[1] = (float)(g_height);

				int neww = (int)( pos[2]-pos[0]-dx );
				if(neww < 0) neww = 0;
				pos[0] = pos[2] - neww;
				if(pos[0] < 0) pos[0] = (float)0;
				if(pos[0] > g_width) pos[0] = (float)(g_width);
				
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZTR)
			{
				int newh = (int)( pos[3]-pos[1]-dy );
				if(newh < 0) newh = 0;
				pos[1] = pos[3] - newh;
				if(pos[1] < 0) pos[1] = (float)0;
				if(pos[1] > g_height) pos[1] = (float)(g_height);

				int neww = (int)( pos[2]-pos[0]+dx );
				if(neww < 0) neww = 0;
				pos[2] = (float)( pos[0] + neww );
				if(pos[2] < 0) pos[2] = (float)0;
				if(pos[2] > g_width) pos[2] = (float)(g_width);
				
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZBL) 
			{
				int newh = (int)( pos[3]-pos[1]+dy );
				if(newh < 0) newh = 0;
				pos[3] = pos[1] + newh;
				if(pos[3] < 0) pos[3] = (float)0;
				if(pos[3] > g_height) pos[3] = (float)(g_height);

				int neww = (int)( pos[2]-pos[0]-dx );
				if(neww < 0) neww = 0;
				pos[0] = pos[2] - neww;
				if(pos[0] < 0) pos[0] = (float)0;
				if(pos[0] > g_width) pos[0] = (float)(g_width);
				
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
			else if(g_curst == CU_RESZBR)
			{
				int newh = (int)( pos[3]-pos[1]+dy );
				if(newh < 0) newh = 0;
				pos[3] = pos[1] + newh;
				if(pos[3] < 0) pos[3] = (float)0;
				if(pos[3] > g_height) pos[3] = (float)(g_height);

				int neww = (int)( pos[2]-pos[0]+dx );
				if(neww < 0) neww = 0;
				pos[2] = pos[0] + neww;
				if(pos[2] < 0) pos[2] = (float)0;
				if(pos[2] > g_width) pos[2] = (float)(g_width);
				
				memcpy(prevpos, pos, sizeof(float)*4);
				reframe();
			}
		}
	}

	if(over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			//InfoMess("win ev", "WE");

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

		if(!ie->intercepted)
		{
			if( g_mouse.x >= outpos[0] &&
			   g_mouse.y >= outpos[1] &&
			   g_mouse.x <= outpos[2] &&
			   g_mouse.y <= outpos[3])
			{
				over = ectrue;
				
				//InfoMess("win evOVER", "OVER");
				
				if(g_mousekeys[MOUSE_MIDDLE])
					return;
				
				//ie->intercepted = ectrue;
				
				if(g_curst == CU_DRAG)
					return;
				
				if(g_mouse.x <= pos[0])
				{
					if(g_mouse.y <= pos[1])
						ie->curst = CU_RESZTL;
					else if(g_mouse.y >= pos[3])
						ie->curst = CU_RESZBL;
					else
						ie->curst = CU_RESZL;
					
					ie->intercepted = ectrue;
				}
				else if(g_mouse.x >= pos[2])
				{
					if(g_mouse.y <= pos[1])
						ie->curst = CU_RESZTR;
					else if(g_mouse.y >= pos[3])
						ie->curst = CU_RESZBR;
					else
						ie->curst = CU_RESZR;
					
					ie->intercepted = ectrue;
				}
				else if(g_mouse.x >= outpos[0] &&
						g_mouse.x <= outpos[2])
				{
					if(g_mouse.y <= outpos[1]+4)
						ie->curst = CU_RESZT;
					else if(g_mouse.y >= pos[3])
						ie->curst = CU_RESZB;
					else if(g_mouse.x >= pos[0] &&
							g_mouse.y >= outpos[1]+4 &&
							g_mouse.x <= pos[2] &&
							g_mouse.y <= pos[1])
						ie->curst = CU_MOVE;
					else
						ie->curst = CU_DEFAULT;
					
					ie->intercepted = ectrue;
				}
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
}

void Win::subframe(float* fr)
{
	memcpy(fr, pos, sizeof(float)*4);
}
