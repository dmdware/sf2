










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

InsDraw::InsDraw() : Widget()
{
	parent = NULL;
	type = WIDGET_INSDRAW;
	clickfunc = NULL;
	ldown = ecfalse;
}

InsDraw::InsDraw(Widget* parent, void (*inst)()) : Widget()
{
	parent = parent;
	type = WIDGET_INSDRAW;
	clickfunc = inst;
	ldown = ecfalse;
}

void InsDraw::draw()
{
	if(clickfunc != NULL)
		clickfunc();
}

