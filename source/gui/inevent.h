










#ifndef INEV_H
#define INEV_H

#include "../platform.h"

#define INEV_MOUSEMOVE		0
#define INEV_MOUSEDOWN		1
#define INEV_MOUSEUP		2
#define INEV_KEYDOWN		3
#define INEV_KEYUP			4
#define INEV_CHARIN			5
#define INEV_MOUSEWHEEL		6
#define INEV_TEXTED			7
#define INEV_TEXTIN			8
#define INEV_COPY			9
#define INEV_PASTE			10
#define INEV_SELALL			11	//select all

#define MOUSE_LEFT			0
#define MOUSE_MIDDLE		1
#define MOUSE_RIGHT			2

struct Widget;

struct InEv
{
public:
	int type;
	int x;
	int y;
	int dx;
	int dy;
	unsigned int key;
	int scancode;
	int amount;
	ecbool intercepted;
	std::string text;	//UTF8
	int cursor;	//cursor pos in composition
	int sellen;	//selection length
	signed char curst;	//cursor state/type eg. drag, resize, normal
	Widget* interceptor;	//corpd fix

	InEv();
};


#endif
