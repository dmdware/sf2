










#ifndef GUI_H
#define GUI_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "viewlayer.h"
#include "widgets/image.h"
#include "widgets/barbutton.h"
#include "widgets/button.h"
#include "widgets/checkbox.h"
#include "widgets/droplist.h"
#include "widgets/editbox.h"
#include "widgets/insdraw.h"
#include "widgets/link.h"
#include "widgets/listbox.h"
#include "widgets/text.h"
#include "widgets/textarea.h"
#include "widgets/textblock.h"
#include "widgets/touchlistener.h"
#include "widgets/frame.h"
#include "widgets/viewportw.h"

#ifdef USEZOOM
extern unsigned int g_zoomtex;
extern ecbool g_zoomdrawframe;
extern unsigned int g_zoomfbo;
#endif

struct GUI : public Widget
{
public:
	std::list<ViewLayer> view;
	void (*keyupfunc[SDL_NUM_SCANCODES])();
	void (*keydownfunc[SDL_NUM_SCANCODES])();
	void (*anykeyupfunc)(int k);
	void (*anykeydownfunc)(int k);
	void (*mousemovefunc)(InEv* ie);
	void (*lbuttondownfunc)();
	void (*lbuttonupfunc)();
	void (*rbuttondownfunc)();
	void (*rbuttonupfunc)();
	void (*mbuttondownfunc)();
	void (*mbuttonupfunc)();
	void (*mousewheelfunc)(int delta);
	Widget* activewidg;

	GUI() : Widget()
	{
		type = WIDGET_GUI;

		//corpc fix
		for(int i=0; i<SDL_NUM_SCANCODES; i++)
		{
			keyupfunc[i] = NULL;
			keydownfunc[i] = NULL;
		}
		
		anykeyupfunc = NULL;
		anykeydownfunc = NULL;
		lbuttondownfunc = NULL;
		lbuttonupfunc = NULL;
		rbuttondownfunc = NULL;
		rbuttonupfunc = NULL;
		mbuttondownfunc = NULL;
		mbuttonupfunc = NULL;
		mousewheelfunc = NULL;
		mousemovefunc = NULL;

		hidden = ecfalse;
	}

	void draw();
	void drawover(){}
	void inev(InEv* ie);
	void reframe();
};

extern GUI g_gui;

ecbool MousePosition();
void CenterMouse();
void SetStatus(const char* status, ecbool logthis=ecfalse);
void Ortho(int width, int height, float r, float g, float b, float a);

#endif
