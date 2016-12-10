










#ifndef WINDOWW_H
#define WINDOWW_H

#include "../widget.h"
#include "image.h"
#include "../cursor.h"
#include "vscrollbar.h"

struct Win : public Widget
{
public:

	Image bg_logo_image;

	Text title_text;
	VScroll vscroll;
	
	Button trclose;	//top right close
	Button trfull;	//top full size toggle
	float prevpos[4];	//for fullscreen toggle

	int mousedown[2];

	Win();
	Win(Widget* parent, const char* n, void (*reframef)(Widget* w));

	//in msvs2012, not making these virtual still calls these wtf
	//but not on mac
	virtual void fillout(float* outpos);
	virtual void show();
	virtual void inev(InEv* ie);
	virtual void draw();
	virtual void drawover();
	virtual void reframe();
	virtual void chcall(Widget* ch, int type, void* data);
	virtual void subframe(float* fr);
	void fullsize();	//toggle fullscreen size
};

#endif
