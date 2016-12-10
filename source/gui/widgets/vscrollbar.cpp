










#include "vscrollbar.h"
#include "../../sim/player.h"
#include "../../render/shader.h"
#include "../../debug.h"

VScroll::VScroll() : Widget()
{
	parent = NULL;
	type = WIDGET_VSCROLLBAR;
	name = "";
	opened = ecfalse;
	selected = 0;
	scroll[1] = 0;
	mousescroll = ecfalse;
	ldown = ecfalse;
	ldownbar = ecfalse;
	ldownup = ecfalse;
	ldowndown = ecfalse;
	domain = 1;
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	changefunc = NULL;
	reframefunc = NULL;
}

VScroll::VScroll(Widget* parent, const char* n) : Widget()
{
	parent = parent;
	type = WIDGET_VSCROLLBAR;
	name = n;
	opened = ecfalse;
	selected = 0;
	scroll[1] = 0;
	mousescroll = ecfalse;
	ldown = ecfalse;
	ldownbar = ecfalse;
	ldownup = ecfalse;
	ldowndown = ecfalse;
	domain = 0.5f;
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	changefunc = NULL;
	reframefunc = NULL;
	//reframe();
}

void VScroll::reframe()
{
	Widget::reframe();

	if(parent)
	{
		float* parp = parent->pos;

		//must be bounded by the parent's frame

		pos[0] = fmax(parp[0], pos[0]);
		pos[0] = fmin(parp[2], pos[0]);
		pos[2] = fmax(parp[0], pos[2]);
		pos[2] = fmin(parp[2], pos[2]);
		pos[1] = fmax(parp[1], pos[1]);
		pos[1] = fmin(parp[3], pos[1]);
		pos[3] = fmax(parp[1], pos[3]);
		pos[3] = fmin(parp[3], pos[3]);

		pos[1] = fmin(pos[1], pos[3]);
		pos[0] = fmin(pos[0], pos[2]);
	}

	int w = (int)( pos[2]-pos[0] );

	uppos[0] = pos[0];
	uppos[1] = pos[1];
	uppos[2] = pos[2];
	uppos[3] = pos[1]+w;

	downpos[0] = pos[0];
	downpos[1] = pos[3]-w;
	downpos[2] = pos[2];
	downpos[3] = pos[3];

	barpos[0] = pos[0];
	barpos[1] = uppos[3] + (downpos[1]-uppos[3])*scroll[1];
	barpos[2] = pos[2];
	//barpos[3] = barpos[1] + fmax(w, domain*(downpos[1]-uppos[3]));
	barpos[3] = barpos[1] + domain*(downpos[1]-uppos[3]);

	if(parent)
	{
		float* parp = parent->pos;

		//must be bounded by the parent's frame

		uppos[0] = fmax(parp[0], uppos[0]);
		uppos[0] = fmin(parp[2], uppos[0]);
		uppos[2] = fmax(parp[0], uppos[2]);
		uppos[2] = fmin(parp[2], uppos[2]);
		uppos[1] = fmax(parp[1], uppos[1]);
		uppos[1] = fmin(parp[3], uppos[1]);
		uppos[3] = fmax(parp[1], uppos[3]);
		uppos[3] = fmin(parp[3], uppos[3]);

		downpos[0] = fmax(parp[0], downpos[0]);
		downpos[0] = fmin(parp[2], downpos[0]);
		downpos[2] = fmax(parp[0], downpos[2]);
		downpos[2] = fmin(parp[2], downpos[2]);
		downpos[1] = fmax(parp[1], downpos[1]);
		downpos[1] = fmin(parp[3], downpos[1]);
		downpos[3] = fmax(parp[1], downpos[3]);
		downpos[3] = fmin(parp[3], downpos[3]);

		//bar must be vertically between the two arrow buttons
		barpos[0] = fmax(parp[0], barpos[0]);
		barpos[0] = fmin(parp[2], barpos[0]);
		barpos[2] = fmax(parp[0], barpos[2]);
		barpos[2] = fmin(parp[2], barpos[2]);

		barpos[1] = fmax(parp[1], barpos[1]);
		barpos[1] = fmin(parp[3], barpos[1]);
		barpos[3] = fmax(pos[1], barpos[3]);
		barpos[3] = fmin(pos[3], barpos[3]);

		barpos[1] = fmax(barpos[1], uppos[3]);
		barpos[1] = fmin(barpos[1], downpos[1]);
		barpos[3] = fmax(barpos[3], uppos[3]);
		barpos[3] = fmin(barpos[3], downpos[1]);
	}
}

void VScroll::draw()
{
	glUniform4f(g_sh[g_curS].slot[SSLOT_COLOR], 1, 1, 1, 0.8f);
	DrawImage(g_texture[ uptex ].texname, uppos[0], uppos[1], uppos[2], uppos[3], 0, 0, 1, 1, crop);
	DrawImage(g_texture[ uptex ].texname, downpos[0], downpos[1], downpos[2], downpos[3], 0, 1, 1, 0, crop);

	Py* py = &g_py[g_localP];

	EndS();

	UseS(SHADER_COLOR2D);
	glUniform1f(g_sh[g_curS].slot[SSLOT_WIDTH], (float)g_currw);
	glUniform1f(g_sh[g_curS].slot[SSLOT_HEIGHT], (float)g_currh);

	float midcolor[] = {0.7f,0.7f,0.7f,0.8f};
	float lightcolor[] = {0.8f,0.8f,0.8f,0.8f};
	float darkcolor[] = {0.5f,0.5f,0.5f,0.8f};

	if(over)
	{
		for(int i=0; i<3; i++)
		{
			midcolor[i] = 0.8f;
			lightcolor[i] = 0.9f;
			darkcolor[i] = 0.6f;
		}
	}

	DrawSquare(midcolor[0], midcolor[1], midcolor[2], midcolor[3], barpos[0], barpos[1], barpos[2], barpos[3], crop);

	DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], barpos[2], barpos[1], barpos[2], barpos[3]-1, crop);
	DrawLine(lightcolor[0], lightcolor[1], lightcolor[2], lightcolor[3], barpos[0], barpos[1], barpos[2]-1, barpos[1], crop);

	DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], barpos[0]+1, barpos[3], barpos[2], barpos[3], crop);
	DrawLine(darkcolor[0], darkcolor[1], darkcolor[2], darkcolor[3], barpos[2], barpos[1]+1, barpos[2], barpos[3], crop);

	EndS();
	CHECKGLERROR();
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

void VScroll::frameupd()
{
	if(ldownup)
	{
		float dy = (float)-g_drawfrinterval * domain;

		float origscroll = scroll[1];

		scroll[1] += (float)dy;

		int w = (int)( pos[2]-pos[0] );

		barpos[1] = uppos[3] + (downpos[1]-uppos[3])*scroll[1];
		//barpos[3] = barpos[1] + fmax(w, domain*(downpos[1]-uppos[3]));
		barpos[3] = barpos[1] + domain*(downpos[1]-uppos[3]);

		float overy = barpos[3] - downpos[1];

		if(overy > 0)
		{
			barpos[1] -= overy;
			barpos[3] -= overy;
			scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
		}

		float undery = uppos[3] - barpos[1];

		if(undery > 0)
		{
			barpos[1] += undery;
			barpos[3] += undery;
			scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
		}
	}

	else if(ldowndown)
	{
		float dy = (float)g_drawfrinterval * domain;

		float origscroll = scroll[1];

		scroll[1] += (float)dy;

		int w = (int)( pos[2]-pos[0] );

		barpos[1] = uppos[3] + (downpos[1]-uppos[3])*scroll[1];
		//barpos[3] = barpos[1] + fmax(w, domain*(downpos[1]-uppos[3]));
		barpos[3] = barpos[1] + domain*(downpos[1]-uppos[3]);

		float overy = barpos[3] - downpos[1];

		if(overy > 0)
		{
			barpos[1] -= overy;
			barpos[3] -= overy;
			scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
		}

		float undery = uppos[3] - barpos[1];

		if(undery > 0)
		{
			barpos[1] += undery;
			barpos[3] += undery;
			scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
		}
	}
}

void VScroll::inev(InEv* ie)
{
	//for(std::list<Widget*>::iterator w=sub.rbegin(); w!=sub.rend(); w++)
	//	(*w)->inev(ie);

	Py* py = &g_py[g_localP];

	//return;

#if 1
	if(ldown)
	{
		if(ie->type == INEV_MOUSEMOVE ||
			( (ie->type == INEV_MOUSEDOWN || ie->type == INEV_MOUSEUP) && ie->key == MOUSE_LEFT) )
			ie->intercepted = ectrue;

		if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
		{
			ldown = ecfalse;
			ldownbar = ecfalse;
			ldownup = ecfalse;
			ldowndown = ecfalse;
		}

		if(ie->type == INEV_MOUSEMOVE && ldownbar)
		{
			if(g_mouse.y < barpos[1] || g_mouse.y > barpos[3])
				return;

			int dx = g_mouse.x - mousedown[0];
			int dy = g_mouse.y - mousedown[1];
			mousedown[0] = g_mouse.x;
			mousedown[1] = g_mouse.y;

			float origscroll = scroll[1];

			scroll[1] += (float)dy / (downpos[1] - uppos[3]);

			int w = (int)( pos[2]-pos[0] );

			barpos[1] = uppos[3] + (downpos[1]-uppos[3])*scroll[1];
			//barpos[3] = barpos[1] + fmax(w, domain*(downpos[1]-uppos[3]));
			barpos[3] = barpos[1] + domain*(downpos[1]-uppos[3]);

			float overy = barpos[3] - downpos[1];

			if(overy > 0)
			{
				barpos[1] -= overy;
				barpos[3] -= overy;
				scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
			}

			float undery = uppos[3] - barpos[1];

			if(undery > 0)
			{
				barpos[1] += undery;
				barpos[3] += undery;
				scroll[1] = (barpos[1] - uppos[3]) / (downpos[1] - uppos[3]);
			}

			if(parent)
			{
				VScroll::ScrollEv sev;
				sev.delta = scroll[1] - origscroll;
				sev.newpos = scroll[1];

				//parent->chcall(this, CHCALL_VSCROLL, (void*)&sev);
			}
		}
	}
#endif

	//return;

	if(over && ie->type == INEV_MOUSEDOWN && !ie->intercepted)
	{
		if(ie->key == MOUSE_LEFT)
		{
			ldown = ectrue;

			if(g_mouse.x >= barpos[0] &&
				g_mouse.y >= barpos[1] &&
				g_mouse.x <= barpos[2] &&
				g_mouse.y <= barpos[3])
			{
				ldownbar = ectrue;
				mousedown[0] = g_mouse.x;
				mousedown[1] = g_mouse.y;
				ie->intercepted = ectrue;

				//ie->curst = CU_RESZT;
			}

			if(g_mouse.x >= uppos[0] &&
				g_mouse.y >= uppos[1] &&
				g_mouse.x <= uppos[2] &&
				g_mouse.y <= uppos[3])
			{
				ldownup = ectrue;
				ie->intercepted = ectrue;

				//ie->curst = CU_RESZT;
			}

			if(g_mouse.x >= downpos[0] &&
				g_mouse.y >= downpos[1] &&
				g_mouse.x <= downpos[2] &&
				g_mouse.y <= downpos[3])
			{
				ldowndown = ectrue;
				ie->intercepted = ectrue;

				//ie->curst = CU_RESZT;
			}
		}
	}

	//return;

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(ldown)
		{
			ie->intercepted = ectrue;
			return;
		}

		//return;

#if 1
		//corpe fix
		if(/* !ie->intercepted && */
			g_mouse.x >= pos[0] &&
			g_mouse.y >= pos[1] &&
			g_mouse.x <= pos[2] &&
			g_mouse.y <= pos[3])
		{
			over = ectrue;

			//if(g_mousekeys[MOUSE_MIDDLE])
			//	return;

			//return;
			ie->intercepted = ectrue;

			//InfoMess("in", "i");

#if 0
			//if(g_curst == CU_RESZT)
			//	return;

			if(g_mouse.x >= barpos[0] &&
				g_mouse.y >= barpos[1] &&
				g_mouse.x <= barpos[2] &&
				g_mouse.y <= barpos[3])
				ie->curst = CU_RESZT;
#endif
		}
		else
		{
			if(!ie->intercepted)
			{
				if(over)
				{
					//g_curst = CU_DEFAULT;
				}
			}
			else
			{
				// to do: this will be replaced by code in other
				//widgets that will set the cursor
				//ie->curst = CU_DEFAULT;
			}

			over = ecfalse;
		}
	}
#endif
}
