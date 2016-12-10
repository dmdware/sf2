










#ifndef VSCROLLBAR_H
#define VSCROLLBAR_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"

struct VScroll : public Widget
{
public:

	struct ScrollEv
	{
		float delta;
		float newpos;
	};

	//scroll[1] is the ratio of the current position of the top of the scroll bar. mustn't go up to 1 because area below is covered by domain (subtract it from total 1.0).
	float domain;	//ratio of how much of the scroll space the current view covers
	float barpos[4];
	float uppos[4];
	float downpos[4];
	int mousedown[2];
	ecbool ldownbar;
	ecbool ldownup;
	ecbool ldowndown;

	VScroll();
	VScroll(Widget* parent, const char* n);

	void inev(InEv* ie);
	virtual void frameupd();
	void draw();
	void reframe();
};

#endif
