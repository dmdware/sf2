










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


void Link::draw()
{
	glDisable(GL_TEXTURE_2D);

	float color[] = {1, 1, 1, 1};

	if(over)
	{
		//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
		//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
		color[0] = 0.8f;
		color[1] = 0.8f;
		color[2] = 0.8f;
	}

	DrawShadowedText(font, pos[0], pos[1], &text, color);

	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	//glEnable(GL_TEXTURE_2D);
}

void Link::inev(InEv* ie)
{
	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();
		
#ifdef PLATFORM_MOBILE
		if(over)
			Log("link up over");
		else
			Log("link up out");
#endif
		
		if(over && ldown)
		{
#ifdef PLATFORM_MOBILE
			Log("LINK CLICK");
#endif
			
			if(clickfunc != NULL)
				clickfunc();

			over = ecfalse;
			ldown = ecfalse;

			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}

		over = ecfalse;
		ldown = ecfalse;
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//mousemove();

		if(over)
		{
#ifdef PLATFORM_MOBILE
			Log("down over");
#endif
			
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;	// intercept mouse event
		}
#ifdef PLATFORM_MOBILE
		else
			Log("down out");
#endif
	}
	else if(ie->type == INEV_MOUSEMOVE)
	{
		int texlen = text.texlen();
		int endx = EndX(&text, texlen, font, 0, 0);	//corpd fix
		if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] &&
		                g_mouse.x <= pos[0]+endx &&	//
		                g_mouse.y <= pos[1]+g_font[font].gheight)
		{
#ifdef PLATFORM_MOBILE
			Log("link over move");
#endif
		}
		else
		{
#ifdef PLATFORM_MOBILE
			Log("link out move");
#endif
			over = ecfalse;
		}

		if(!ie->intercepted)
		{
			if(g_mouse.x >= pos[0] && g_mouse.y >= pos[1] &&
			g_mouse.x <= pos[0]+endx &&	//
			                g_mouse.y <= pos[1]+g_font[font].gheight)
			{
				over = ectrue;

				ie->intercepted = ectrue;
			}
			else
			{
				over = ecfalse;
			}
		}
	}
}

