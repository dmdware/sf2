










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
#include "viewportw.h"
#include "../../platform.h"
#include "../../window.h"
#include "../../render/shader.h"
#include "../gui.h"
#include "../../debug.h"

void Viewport_init(Viewport *vp,
				   Widget* parent, const char* n, void (*reframef)(Widget* w),
				   void (*drawf)(int p, int x, int y, int w, int h),
				   ecbool (*ldownf)(int p, int x, int y, int w, int h),
				   ecbool (*lupf)(int p, int x, int y, int w, int h),
				   ecbool (*mousemovef)(int p, int x, int y, int w, int h),
				   ecbool (*rdownf)(int p, int relx, int rely, int w, int h),
				   ecbool (*rupf)(int p, int relx, int rely, int w, int h),
				   ecbool (*mousewf)(int p, int d),
				   ecbool (*mdownf)(int p, int relx, int rely, int w, int h),
				   ecbool (*mupf)(int p, int relx, int rely, int w, int h),
				   int parm)
{
	Widget *bw;

	bw = (Widget*)vp;
	Widget_init(bw);

	bw->parent = parent;
	bw->type = WIDGET_VIEWPORT;
	strcpy(bw->name, n);
	bw->reframefunc = reframef;
	bw->ldown = ecfalse;
	bw->param = parm;
	bw->drawfunc = drawf;
	bw->ldownfunc = ldownf;
	bw->lupfunc = lupf;
	bw->mousemovefunc = mousemovef;
	bw->rdownfunc = rdownf;
	bw->rupfunc = rupf;
	bw->mousewfunc = mousewf;
	bw->mdownfunc = mdownf;
	bw->mupfunc = mupf;

	Widget_reframe(bw);
	/* TODO remove repeats? */
}

void Viewport_draw(Viewport *vp)
{
	Widget *bw;
	int w, h;
	int viewport[4];
	Shader *s;

	bw = (Widget*)vp;

	w = (int)( bw->pos[2] - bw->pos[0] );
	h = (int)( bw->pos[3] - bw->pos[1] );

	s = g_sh+g_curS;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(bw->pos[0], g_height-bw->pos[3], w, h);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)w);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)h);

	EndS();

	CHECKGLERROR();

	if(bw->drawfunc != NULL)
		bw->drawfunc(bw->param, bw->pos[0], bw->pos[1], w, h);

	CHECKGLERROR();

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
}

void Viewport_inev(Viewport *vp, InEv* ie)
{
	Widget *bw;
	int relx;
	int rely;
	int w;
	int h;

	bw = (Widget*)vp;

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(g_mouse.x >= bw->pos[0] && 
			g_mouse.x <= bw->pos[2] && 
			g_mouse.y >= bw->pos[1] && 
			g_mouse.y <= bw->pos[3])
		{
			if(!ie->intercepted)
				bw->over = ectrue;
		}
		else
			bw->over = ecfalse;
	}

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(g_mouse.x >= bw->pos[0] && 
			g_mouse.x <= bw->pos[2] && 
			g_mouse.y >= bw->pos[1] && 
			g_mouse.y <= bw->pos[3])
			bw->over = ectrue;

		if(bw->mousemovefunc != NULL)
		{
			relx = g_mouse.x - (int)bw->pos[0];
			rely = g_mouse.y - (int)bw->pos[1];
			w = (int)( bw->pos[2] - bw->pos[0] );
			h = (int)( bw->pos[3] - bw->pos[1] );
			ie->intercepted = bw->mousemovefunc(bw->param, relx, rely, w, h);
		}

		return;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(!bw->over)
			return;

		if(bw->ldownfunc != NULL)
		{
			relx = g_mouse.x - (int)bw->pos[0];
			rely = g_mouse.y - (int)bw->pos[1];
			w = (int)( bw->pos[2] - bw->pos[0] );
			h = (int)( bw->pos[3] - bw->pos[1] );
			ie->intercepted = bw->ldownfunc(bw->param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(!bw->over)
			return;

		if(bw->lupfunc != NULL)
		{
			relx = g_mouse.x - (int)bw->pos[0];
			rely = g_mouse.y - (int)bw->pos[1];
			w = (int)( bw->pos[2] - bw->pos[0] );
			h = (int)( bw->pos[3] - bw->pos[1] );
			ie->intercepted = bw->lupfunc(bw->param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_RIGHT && !ie->intercepted)
	{
		if(!bw->over)
			return;

		if(bw->rdownfunc != NULL)
		{
			relx = g_mouse.x - (int)bw->pos[0];
			rely = g_mouse.y - (int)bw->pos[1];
			w = (int)( bw->pos[2] - bw->pos[0] );
			h = (int)( bw->pos[3] - bw->pos[1] );
			ie->intercepted = bw->rdownfunc(bw->param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_RIGHT && !ie->intercepted)
	{
		if(!bw->over)
			return;

		if(rupfunc != NULL)
		{
			relx = g_mouse.x - (int)bw->pos[0];
			rely = g_mouse.y - (int)bw->pos[1];
			w = (int)( bw->pos[2] - bw->pos[0] );
			h = (int)( bw->pos[3] - bw->pos[1] );
			ie->intercepted = bw->rupfunc(bw->param, relx, rely, w, h);
		}
	}
	else if(ie->type == INEV_MOUSEWHEEL && !ie->intercepted)
	{
		if(!bw->over)
			return;

		if(bw->mousewfunc != NULL)
		{
			ie->intercepted = bw->mousewfunc(bw->param, ie->amount);
		}
	}
}
