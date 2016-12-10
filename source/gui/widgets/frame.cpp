










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
#include "frame.h"
#include "../../platform.h"

Frame::Frame(Widget* parent, const char* n, void (*reframef)(Widget* w))
{
	parent = parent;
	type = WIDGET_FRAME;
	name = n;
	reframefunc = reframef;
	ldown = ecfalse;
	reframe();
}

void Frame::draw()
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->draw();
}

void Frame::drawover()
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->drawover();
}

void Frame::frameupd()
{
	for(std::list<Widget*>::reverse_iterator i=sub.rbegin(); i!=sub.rend(); i++)
		(*i)->frameupd();
}

void Frame::inev(InEv* ie)
{
	for(std::list<Widget*>::reverse_iterator i=sub.rbegin(); i!=sub.rend(); i++)
		(*i)->inev(ie);
}
