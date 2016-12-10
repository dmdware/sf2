










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
#include "../../sim/player.h"
#include "../../debug.h"

void BarBut_init(BarBut* w, Widget* parent, unsigned int sprite, float bar, void (*reframef)(Widget* w), void (*click)(), void (*overf)(), void (*out)()) : Button()
{
	w->parent = parent;
	w->type = WIDGET_BARBUTTON;
	w->over = ecfalse;
	w->ldown = ecfalse;
	w->tex = sprite;
	CreateTex(&w->bgtex, "gui/buttonbg.png", ectrue, ecfalse);
	CreateTex(&w->bgovertex, "gui/buttonbgover.png", ectrue, ecfalse);
	w->reframefunc = reframef;
	w->healthbar = bar;
	w->clickfunc = click;
	w->overfunc = overf;
	w->outfunc = out;
	Widget_reframe((Widget*)w);
}

void BarBut_draw(BarBut* w)
{
	unsigned int bgovertex, bgtex, tex;
	float *pos;
	float *crop;
	float bar;
	Shader *s;

	bgovertex = w->bgovertex;
	bgtex = w->bgtex;
	tex = w->tex;
	pos = w->pos;
	crop = w->crop;

	if(w->over)
		DrawImage(g_texture[bgovertex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);
	else
		DrawImage(g_texture[bgtex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	DrawImage(g_texture[tex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	EndS();
	UseS(SHADER_COLOR2D);
	s = g_sh+g_curS;
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_currw);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_currh);
	DrawSquare(1, 0, 0, 1, pos[0], pos[3]-5, pos[2], pos[3], crop);
	bar = (pos[2] - pos[0]) * healthbar;
	DrawSquare(0, 1, 0, 1, pos[0], pos[3]-5, pos[0]+bar, pos[3], crop);

	EndS();
	CHECKGLERROR();
	Ortho(g_currw, g_currh, 1, 1, 1, 1);
}

