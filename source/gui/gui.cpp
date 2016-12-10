










#include "gui.h"
#include "../render/shader.h"
#include "../texture.h"
#include "font.h"
#include "../math/3dmath.h"
#include "../platform.h"
#include "../window.h"
#include "draw2d.h"
#include "../render/heightmap.h"
#include "../app/appmain.h"
#include "cursor.h"
#include "../sim/player.h"
#include "../debug.h"
#include "../sim/mvtype.h"
#include "../sim/bltype.h"
#include "widgets/spez/lobby.h"

GUI g_gui;

#ifdef USEZOOM
unsigned int g_zoomtex = -1;
ecbool g_zoomdrawframe = ecfalse;
unsigned int g_zoomfbo = -1;
#endif

void GUI::draw()
{
	Py* py = &g_py[g_localP];

	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	CHECKGLERROR();
	Ortho(g_width, g_height, 1, 1, 1, 1);
	CHECKGLERROR();

#if 0
	DrawImage(g_texture[0].texname, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

	Widget::draw();
	CHECKGLERROR();
	Widget::drawover();

	//not engine
	if(g_appmode == APPMODE_PLAY &&
		g_keys[SDL_SCANCODE_TAB])
	{
		Lobby_DrawPyL();
		Lobby_DrawState();
	}

#if 0
	DrawImage(g_depth, g_width - 300, 0, g_width, 300, 0, 1, 1, 0);
#endif

#if 0
	if(g_depth != -1)
		DrawImage(g_depth, 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 0
	if(g_appmode == APPMODE_PLAY)
		DrawImage(g_tiletexs[IN_PRERENDER], 0, 0, 150, 150, 0, 1, 1, 0);
#endif

#if 1
	unsigned int spi = g_cursor[g_curst];
	Sprite* sp = &g_sprite[spi];

	float crop[] = {0,0,(float)g_width-1,(float)g_height-1}; 
	DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3], 0,0,1,1, crop);
#elif 1
	static int si = -100;
	si++;
	if(si >= 0)
	{
		BlType* bt = &g_bltype[BL_CHEMPL];
		Sprite* sp = &g_sprite[bt->sprite[0]];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#elif 0
	static int si = -100;
	si++;
	if(si >= 0)
	{
		InType* tt = &g_intype[0];
		Sprite* sp = &g_sprite[tt->sprite];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#else
	MvType* mvt = &g_mvtype[MV_LABOURER];
	static int si = -100;
	si++;
	if(si >= 0)
	{
		//si = si%(DIRS*mvt->nframes);
		si = si%(mvt->nframes);
		unsigned int spi = mvt->sprite[si/mvt->nframes][si%mvt->nframes];
		//unsigned int spi = mvt->sprite[0][si%mvt->nframes];

		Sprite* sp = &g_sprite[spi];
		DrawImage(g_texture[sp->difftexi].texname, g_mouse.x+sp->offset[0], g_mouse.y+sp->offset[1], g_mouse.x+sp->offset[2], g_mouse.y+sp->offset[3]);
	}
#endif

#ifdef PLATFORM_MOBILE
#if 0
	if(g_zoomdrawframe)
	{
		glFlush();
		glFinish();
		
		Vec2i zoombox[2];
		
		zoombox[0].x = g_mouse.x - ZOOMBOX;
		zoombox[0].y = g_mouse.y - ZOOMBOX;
		zoombox[1].x = g_mouse.x + ZOOMBOX;
		zoombox[1].y = g_mouse.y + ZOOMBOX;
		
		zoombox[0].x = imax(0, zoombox[0].x);
		zoombox[0].y = imax(0, zoombox[0].y);
		zoombox[1].x = imin(g_width-1, zoombox[1].x);
		zoombox[1].y = imin(g_height-1, zoombox[1].y);
		
#if 0
		LoadedTex zoompix;
		zoompix.sizex = zoombox[1].x - zoombox[0].x;
		zoompix.sizey = zoombox[1].y - zoombox[0].y;
		zoompix.channels = 3;
		zoompix.data = (unsigned char*)malloc( sizeof(unsigned char) * 3 * zoompix.sizex * zoompix.sizey );
		
		glReadPixels(zoombox[0].x, zoombox[0].y, zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
		//CreateTex(&zoompix, &zoomtex, ectrue, ecfalse);
		
		glBindTexture(GL_TEXTURE_2D, g_zoomtex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, zoombox[0].x - (g_mouse.x - ZOOMBOX), zoombox[0].y - (g_mouse.y - ZOOMBOX), zoompix.sizex, zoompix.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix.data);
#endif
		
		//DrawImage(g_zoomtex, g_mouse.x - ZOOMBOX*2, g_mouse.y - ZOOMBOX*2, g_mouse.x + ZOOMBOX*2, g_mouse.y + ZOOMBOX*2, 0,1,1,0, crop);
		
		//zoompix.destroy();
		//glDeleteTextures(1, &zoomtex);
		
#if 0
		LoadedTex zoompix2;
		Resample(&zoompix, &zoompix2, Vec2i(zoompix.sizex,zoompix.sizey)*2);
		
		Vec2i raster;
		raster.x = imax(0, g_mouse.x - ZOOMBOX);
		raster.y = imax(0, g_mouse.y - ZOOMBOX);
		
		glRasterPos(raster.x, raster.y);
		
		glDrawPixels(zoompix2.sizex, zoompix2.sizey, GL_RGB, GL_UNSIGNED_BYTE, zoompix2.data);
#endif
	}
#endif
#endif

#if 0
	if(g_appmode == APPMODE_PLAY)
	{
		CdType* ct = &g_cdtype[CD_ROAD];
	int sli = ct->splist[ CONNECTION_EAST ][ 1 ];
	SpList* sl = &g_splist[ sli ];
	int ci = SpriteRef(sl, 0, IN_0000, 0, 0, 0);
	int spi = sl->sprites[ ci ];
	Sprite* sp = &g_sprite[ spi ];
	Texture* difftex = &g_texture[ sp->difftexi ];
	DrawImage(difftex->texname, 0, 0, 150, 150, 0, 0, 1, 1, crop);

	char m[123];
	sprintf(m, "cd sli%d ci%d spi%d sp->difftexi%d difftex->texname%d", sli, ci, spi, sp->difftexi, difftex->texname);
	//InfoMess(m,m);
	}
#endif
	
	CHECKGLERROR();

	EndS();
	CHECKGLERROR();

	UseS(SHADER_COLOR2D);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_sh[SHADER_COLOR2D].slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	//glEnable(GL_DEPTH_TEST);
	//DrawSelector();
	DrawMarquee();

	CHECKGLERROR();
	EndS();
	CHECKGLERROR();

	glEnable(GL_DEPTH_TEST);
}

void GUI::inev(InEv* ie)
{
#ifdef PLATFORM_MOBILE
	if(ie->type == INEV_MOUSEDOWN ||
	   ie->type == INEV_MOUSEUP)
	{
		InEv ie2 = *ie;
		ie2.type = INEV_MOUSEMOVE;
		ie2.intercepted = ecfalse;
		ie2.interceptor = NULL;
		Widget::inev(&ie2);
	}
#endif
	
	Widget::inev(ie);

	if(!ie->intercepted)
	{
		//if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT) Log("mouse up l");

		if(ie->type == INEV_MOUSEMOVE && mousemovefunc) mousemovefunc(ie);
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && lbuttondownfunc) lbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && lbuttonupfunc) lbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_MIDDLE && mbuttondownfunc) mbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_MIDDLE && mbuttonupfunc) mbuttonupfunc();
		else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_RIGHT && rbuttondownfunc) rbuttondownfunc();
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_RIGHT && rbuttonupfunc) rbuttonupfunc();
		else if(ie->type == INEV_MOUSEWHEEL && mousewheelfunc) mousewheelfunc(ie->amount);
		else if(ie->type == INEV_KEYDOWN && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();
		else if(ie->type == INEV_KEYUP && keyupfunc[ie->scancode]) keyupfunc[ie->scancode]();
		//cse fix
		else if(ie->type == INEV_COPY && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();
		else if(ie->type == INEV_PASTE && keydownfunc[ie->scancode]) keydownfunc[ie->scancode]();

		if(ie->type != INEV_MOUSEMOVE && anykeydownfunc) anykeydownfunc(-1);
	}
}

void GUI::reframe()
{
	Py* py = &g_py[g_localP];

	pos[0] = 0;
	pos[1] = 0;
	pos[2] = g_width-1;
	pos[3] = g_height-1;

	Widget::reframe();
}

void SetStatus(const char* status, ecbool logthis)
{
	if(logthis)
	{
		Log(status);
		
	}

#if 0
	Log(status);
	
#endif
	/*
	char upper[1024];
	int i;
	for(i=0; i<strlen(status); i++)
	{
	upper[i] = toupper(status[i]);
	}
	upper[i] = '\0';*/

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	//gui->get("load")->get("status", WIDGET_TEXT)->text = upper;
	ViewLayer* loadingview = (ViewLayer*)gui->get("loading");

	if(!loadingview)
		return;

	Widget* statustext = loadingview->get("status");

	if(!statustext)
		return;

	statustext->text = RichText(UStr(status));
}

ecbool MousePosition()
{
	Py* py = &g_py[g_localP];

	Vec2i old = g_mouse;
	SDL_GetMouseState(&g_mouse.x, &g_mouse.y);
	
#ifdef PLATFORM_MOBILE
	g_mouse.x >>= 1;
	g_mouse.y >>= 1;
#endif

	if(g_mouse.x == old.x && g_mouse.y == old.y)
		return ecfalse;

	return ectrue;
}

void CenterMouse()
{
	Py* py = &g_py[g_localP];
	
	g_mouse.x = g_width/2;
	g_mouse.y = g_height/2;
	
#ifndef PLATFORM_MOBILE
	SDL_WarpMouseInWindow(g_window, g_mouse.x, g_mouse.y);
#else
	//for mobile, half of real screen is full "drawing" screen
	SDL_WarpMouseInWindow(g_window, g_mouse.x*2, g_mouse.y*2);
#endif
}

void Ortho(int width, int height, float r, float g, float b, float a)
{
	CHECKGLERROR();
	Py* py = &g_py[g_localP];
	UseS(SHADER_ORTHO);
	Shader* s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)height);
	glUniform4f(s->slot[SSLOT_COLOR], r, g, b, a);
	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(g_sh[SHADER_ORTHO].slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(g_sh[SHADER_ORTHO].slot[SSLOT_NORMAL]);
	g_currw = width;
	g_currh = height;
	CHECKGLERROR();
}
