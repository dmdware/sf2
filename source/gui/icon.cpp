










#include "icon.h"
#include "../texture.h"

Icon g_icon[ICONS];

void DefI(int type, const char* relative, const char* tag)
{
	Icon* i = &g_icon[type];

	i->tag = UStr(tag);
	//QueueTex(&i->tex, relative, ectrue);
	CreateTex(i->tex, relative, ectrue, ecfalse);
	Texture* t = &g_texture[i->tex];
	i->width = t->width;
	i->height = t->height;
}
