













#include "hint.h"

Hint g_lasthint;

Hint::Hint()
{
	reset();
}

void Hint::reset()
{
	message = "";
	graphic = "gui\\transp.png";
	gwidth = 100;
	gheight = 100;
}

void GMessage(const char* text)
{
#if 0
	Widget *gui = (Widget*)&g_gui;
	Widget* gmessage = gui->get("game message")->get("message");
	gmessage->text = text;
	g_lasthint.message = text;
	g_lasthint.graphic = "gui/transp.png";
	Widget* graphic = gui->get("game message")->get("graphic");
	//graphic->tex = g_transparency;
    CreateTex(graphic->tex, "gui/transp.png", ectrue);
	Click_LastHint();
#endif
}

void GMessageG(const char* tex, int w, int h)
{
#if 0
	Widget *gui = (Widget*)&g_gui;
	Widget* graphic = gui->get("game message")->get("graphic");
	CreateTex(graphic->tex, tex, ectrue);
	graphic->pos[0] = g_width/2 - w/2;
	graphic->pos[2] = g_width/2 + w/2;
	graphic->pos[1] = g_height/2 + 150 - 44-44 - h;
	graphic->pos[3] = g_height/2 + 150 - 44-44;
	g_lasthint.graphic = tex;
	g_lasthint.gwidth = w;
	g_lasthint.gheight = h;
	//AddButton("gui\\buttonbg.png", "Continue", MAINFONT8, g_width/2 - 44, g_height/2 + 150 - 44 - 44, g_width/2 + 44, g_height/2 + 150 - 44, &Click_CloseMessage, NULL, NULL);
#endif
}

