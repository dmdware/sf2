











#include "console.h"
#include "../gui/gui.h"
#include "../sim/player.h"

void Resize_ConsoleLine(Widget* w)
{
	Font* f = &g_font[w->font];
	Py* py = &g_py[g_localP];

	int i = 0;
	sscanf(w->name.c_str(), "%d", &i);

	w->pos[0] = 0;
	w->pos[1] = 30 + f->gheight * i;
	w->pos[2] = g_width;
	w->pos[3] = 30 + f->gheight * (i+1);
}

void Resize_Console(Widget* w)
{
	Font* f = &g_font[w->font];
	Py* py = &g_py[g_localP];
	int i = CONSOLE_LINES;
	w->pos[0] = 0;
	w->pos[1] = 30 + f->gheight * i;
	w->pos[2] = g_width;
	w->pos[3] = 30 + f->gheight * (i+1);
}

void Change_Console(unsigned int key, unsigned int scancode, ecbool down, int parm)
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* conview = (ViewLayer*)gui->get("console");
	EditBox* con = (EditBox*)conview->get("console");

#if 0
	int caret = con->caret;

	if(caret <= 0)
		return;

	unsigned int enter = con->value.substr(caret-1, 1).part.begin()->text.data[0];

	if(enter == '\n' || enter == '\r')
		InfoMess("console", "enter");
#endif

	if(scancode == SDL_SCANCODE_ESCAPE && !down)
		ToggleConsole();
}

void SubmitConsole(RichText* rt)
{
	//return;

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	std::list<Widget*>::iterator witer = con->sub.begin();
	witer++;	//skip bg image widget
	for(int i=0; i<CONSOLE_LINES-1; i++)
	{
		std::list<Widget*>::iterator witer2 = witer;
		witer2++;
		(*witer)->text = (*witer2)->text;
		witer = witer2;
	}

	std::list<Widget*>::iterator witer2 = witer;
	witer2++;
	(*witer)->text = ParseTags(*rt, NULL);
}

void Submit_Console()
{
	return;
	
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	EditBox* coned = (EditBox*)con->get("console");
	SubmitConsole(&coned->value);
	RichText blank("");
	coned->changevalue(&blank);
}

void Resize_Con_BG(Widget* w)
{
	Py* py = &g_py[g_localP];
	Font* f = &g_font[MAINFONT16];

	w->pos[0] = 0;
	w->pos[1] = 0;
	w->pos[2] = g_width;
	w->pos[3] = CONSOLE_LINES * f->gheight + 30;

	Texture* bg = &g_texture[ w->tex ];

	w->texc[0] = 0;
	w->texc[1] = 0;
	w->texc[2] = (w->pos[2] - w->pos[0]) / (float)bg->width;
	w->texc[3] = (w->pos[3] - w->pos[1]) / (float)bg->height;
}

void FillConsole()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	gui->add(new ViewLayer(gui, "console"));
	ViewLayer* con = (ViewLayer*)gui->get("console");

	con->add(new Image(gui, "", "gui/backg/svlistbg.jpg", ectrue, Resize_Con_BG, 1, 1, 1, 0.7f));

	int y = 30;
	for(int i=0; i<CONSOLE_LINES; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		con->add(new Text(con, name, RichText(name), MAINFONT16, Resize_ConsoleLine));
	}

	con->add(new EditBox(con, "console", RichText("console"), MAINFONT16, Resize_Console, ecfalse, 128, Change_Console, Submit_Console, -1));
}

void ToggleConsole()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* con = (ViewLayer*)gui->get("console");
	con->get("console")->opened = ectrue;
	con->opened =! con->opened;
}