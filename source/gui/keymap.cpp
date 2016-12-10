










#include "../app/appmain.h"
#include "keymap.h"
#include "../platform.h"
#include "../gui/gui.h"
#include "../math/camera.h"
#include "../math/vec3f.h"
#include "../math/hmapmath.h"
#include "../render/heightmap.h"
#include "../window.h"
#include "../sim/player.h"
#include "../trigger/console.h"

void MouseMidButtonDown()
{
	if(g_appmode == APPMODE_PLAY || g_appmode == APPMODE_EDITOR)
	{
		Py* py = &g_py[g_localP];

		if(g_mousekeys[MOUSE_MIDDLE])
		{
			CenterMouse();
		}
	}
}

void MouseMidButtonUp()
{
}

void MouseWheel(int delta)
{
	if(g_appmode == APPMODE_PLAY ||
		g_appmode == APPMODE_EDITOR)
	{
	}
}

void Esc()
{
	if(g_appmode == APPMODE_PLAY ||
		g_appmode == APPMODE_EDITOR)
	{
		Py* py = &g_py[g_localP];
		Widget *gui = (Widget*)&g_gui;
		Widget* ingame = gui->get("ingame");

		if(!ingame->hidden)
			gui->hide("ingame");
		else
			gui->show("ingame");
	}
}

void MapKeys()
{
	for(int i=0; i<PLAYERS; i++)
	{
		Py* py = &g_py[i];
		Widget *gui = (Widget*)&g_gui;
		gui->assignmousewheel(&MouseWheel);
		gui->assignmbutton(MouseMidButtonDown, MouseMidButtonUp);
		//gui->assignkey(SDL_SCANCODE_R, ZoomOut, NULL);
		//gui->assignkey(SDL_SCANCODE_F, ZoomIn, NULL);
		gui->assignkey(SDL_SCANCODE_GRAVE, NULL, ToggleConsole);
		gui->assignkey(SDL_SCANCODE_ESCAPE, NULL, Esc);
	}

	/*
	int key;
	void (*down)();
	void (*up)();
	ifstream f("keymap.ini");
	std::string line;
	char key[32];
	char act[32];

	while(!f.eof())
	{
		key = -1;
		down = NULL;
		up = NULL;
		strcpy(key, "");
		strcpy(act, "");

		getline(f, line);
		sscanf(line.c_str(), "%s %s", key, act);

		if(stricmp(key, "SDLK_ESCAPE") == 0)			key = SDLK_ESCAPE;
		else if(stricmp(key, "SDLK_SHIFT") == 0)		key = SDLK_SHIFT;
		else if(stricmp(key, "SDLK_CONTROL") == 0)		key = SDLK_CONTROL;
		else if(stricmp(key, "SDLK_SPACE") == 0)		key = SDLK_SPACE;
		else if(stricmp(key, "MouseLButton") == 0)	key = -2;
		else if(stricmp(key, "F1") == 0)				key = SDLK_F1;
		else if(stricmp(key, "F2") == 0)				key = SDLK_F2;
		else if(stricmp(key, "F3") == 0)				key = SDLK_F3;
		else if(stricmp(key, "F4") == 0)				key = SDLK_F4;
		else if(stricmp(key, "F5") == 0)				key = SDLK_F5;
		else if(stricmp(key, "F6") == 0)				key = SDLK_F6;
		else if(stricmp(key, "F7") == 0)				key = SDLK_F7;
		else if(stricmp(key, "F8") == 0)				key = SDLK_F8;
		else if(stricmp(key, "F9") == 0)				key = SDLK_F9;
		else if(stricmp(key, "F10") == 0)			key = SDLK_F10;
		else if(stricmp(key, "F11") == 0)			key = SDLK_F11;
		else if(stricmp(key, "F12") == 0)			key = SDLK_F12;
		else if(stricmp(key, "'A'") == 0)			key = 'A';
		else if(stricmp(key, "'B'") == 0)			key = 'B';
		else if(stricmp(key, "'C'") == 0)			key = 'C';
		else if(stricmp(key, "'D'") == 0)			key = 'D';
		else if(stricmp(key, "'E'") == 0)			key = 'E';
		else if(stricmp(key, "'cost'") == 0)			key = 'cost';
		else if(stricmp(key, "'G'") == 0)			key = 'G';
		else if(stricmp(key, "'H'") == 0)			key = 'H';
		else if(stricmp(key, "'I'") == 0)			key = 'I';
		else if(stricmp(key, "'J'") == 0)			key = 'J';
		else if(stricmp(key, "'K'") == 0)			key = 'K';
		else if(stricmp(key, "'L'") == 0)			key = 'L';
		else if(stricmp(key, "'M'") == 0)			key = 'M';
		else if(stricmp(key, "'N'") == 0)			key = 'N';
		else if(stricmp(key, "'O'") == 0)			key = 'O';
		else if(stricmp(key, "'P'") == 0)			key = 'P';
		else if(stricmp(key, "'Q'") == 0)			key = 'Q';
		else if(stricmp(key, "'R'") == 0)			key = 'R';
		else if(stricmp(key, "'S'") == 0)			key = 'S';
		else if(stricmp(key, "'T'") == 0)			key = 'T';
		else if(stricmp(key, "'U'") == 0)			key = 'U';
		else if(stricmp(key, "'V'") == 0)			key = 'V';
		else if(stricmp(key, "'W'") == 0)			key = 'W';
		else if(stricmp(key, "'X'") == 0)			key = 'X';
		else if(stricmp(key, "'Y'") == 0)			key = 'Y';
		else if(stricmp(key, "'Z'") == 0)			key = 'Z';
		else if(stricmp(key, "'0'") == 0)			key = '0';
		else if(stricmp(key, "'1'") == 0)			key = '1';
		else if(stricmp(key, "'2'") == 0)			key = '2';
		else if(stricmp(key, "'3'") == 0)			key = '3';
		else if(stricmp(key, "'4'") == 0)			key = '4';
		else if(stricmp(key, "'5'") == 0)			key = '5';
		else if(stricmp(key, "'6'") == 0)			key = '6';
		else if(stricmp(key, "'7'") == 0)			key = '7';
		else if(stricmp(key, "'8'") == 0)			key = '8';
		else if(stricmp(key, "'9'") == 0)			key = '9';

		if(key == -1)
		{
			Log("Unknown input: "<<key);
			continue;
		}

		if(stricmp(act, "Esc();") == 0)				{	down = &Esc;			up = NULL;			}
		else if(stricmp(act, "Forward();") == 0)			{	down = &Forward;		up = NULL;			}
		else if(stricmp(act, "Left();") == 0)			{	down = &Left;			up = NULL;			}
		else if(stricmp(act, "Right();") == 0)			{	down = &Right;			up = NULL;			}
		else if(stricmp(act, "Back();") == 0)			{	down = &Back;			up = NULL;			}

		if(down == NULL)		Log("Unknown action: "<<act);
		else if(key == -2)		AssignLButton(down, up);
		else					AssignKey(key, down, up);
	}
	*/
}
