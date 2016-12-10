










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
#include "../../debug.h"

Image::Image()
{
	parent = NULL;
	type = WIDGET_IMAGE;
	tex = 0;
	pos[0] = 0;
	pos[1] = 0;
	pos[2] = 0;
	pos[3] = 0;
	reframefunc = NULL;

	reframe();
}

Image::Image(Widget* parent, const char* nm, const char* filepath, ecbool clamp, void (*reframef)(Widget* w), float r, float g, float b, float a, float texleft, float textop, float texright, float texbottom) : Widget()
{
	parent = parent;
	type = WIDGET_IMAGE;
	name = nm;
	//CreateTex(tex, filepath, ectrue);
	CreateTex(tex, filepath, clamp, ecfalse);
	//CreateTex(tex, filepath, clamp);
	reframefunc = reframef;
	texc[0] = texleft;
	texc[1] = textop;
	texc[2] = texright;
	texc[3] = texbottom;
	ldown = ecfalse;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	pos[0] = 0;
	pos[1] = 0;
	pos[2] = 0;
	pos[3] = 0;
	reframe();
}

void Image::draw()
{
	//glColor4fv(rgba);
	glUniform4fv(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, rgba);
	DrawImage(g_texture[tex].texname, pos[0], pos[1], pos[2], pos[3], texc[0], texc[1], texc[2], texc[3], crop);
	//glColor4f(1,1,1,1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
}

