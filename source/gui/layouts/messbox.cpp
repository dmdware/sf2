












#include "../gui.h"
#include "../viewlayer.h"
#include "../widgets/textblock.h"
#include "../widgets/button.h"
#include "../widgets/winw.h"
#include "../widgets/touchlistener.h"
#include "appgui.h"
#include "../../language.h"

void (*g_continuefun)() = NULL;

void Resize_Fill(Widget* w)
{
	Widget* parw = w->parent;
	
	w->pos[0] = parw->pos[0];
	w->pos[1] = parw->pos[1];
	w->pos[2] = parw->pos[2];
	w->pos[3] = parw->pos[3];
}

void Resize_MB_Continue(Widget* w)
{
	Widget* parw = w->parent;
	
	w->pos[0] = (parw->pos[0] + parw->pos[2]) / 2.0f - 50;
	w->pos[1] = parw->pos[3] - 50;
	w->pos[2] = (parw->pos[0] + parw->pos[2]) / 2.0f + 50;
	w->pos[3] = parw->pos[3];

	CenterLabel(w);
}

void Click_MB_Continue()
{	
	Widget *gui = (Widget*)&g_gui;
	gui->hide("message");

	if(g_continuefun)
		g_continuefun();
}

void FillMess()
{
	Widget *gui = (Widget*)&g_gui;
	gui->add(new ViewLayer(gui, "message"));
	ViewLayer* messlayer = (ViewLayer*)gui->get("message");
	//messlayer->add(new TouchListener(messlayer, "", Resize_Fullscreen, NULL, NULL, NULL, -1));
	messlayer->add(new Win(messlayer, "win", Resize_CenterWin));
	Win* win = (Win*)messlayer->get("win");
	win->add(new TextBlock(win, "block", RichText("Message"), MAINFONT16, Resize_Fill, 0.8f, 0.8f, 0.2f, 1.0f));
	win->add(new Button(win, "continue", "gui/transp.png", STRTABLE[STR_CONTINUE], RichText(), MAINFONT16, BUST_LINEBASED, Resize_MB_Continue, Click_MB_Continue, NULL, NULL, NULL, NULL, -1, NULL));
}

void Mess(RichText* mess, void (*cfun)())
{
	Widget *gui = (Widget*)&g_gui;
	gui->show("message");
	ViewLayer* messlayer = (ViewLayer*)gui->get("message");
	Win* win = (Win*)messlayer->get("win");
	win->show();
	TextBlock* block = (TextBlock*)win->get("block");
	block->text = *mess;
	g_continuefun = cfun;
}