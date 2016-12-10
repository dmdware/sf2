










#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "blpreview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/appviewport.h"

//viewport
void Resize_BP_VP(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1];
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];
}

//title
void Resize_BP_Tl(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10;
	w->m_pos[1] = parw->m_pos[1] + 10;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[3] - 10;
}

//owner name
void Resize_BP_Ow(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 15;
	w->m_pos[1] = parw->m_pos[1] + 39;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[3] - 10;
}

//params text block: construction material
void Resize_BP_PB_C(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10;
	w->m_pos[1] = parw->m_pos[1] + 16 + 20;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[1] + 16 + 10 + 100;
}

//inputs
void Resize_BP_PB_I(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10;
	w->m_pos[1] = parw->m_pos[1] + 16 + 10 + 100;
	w->m_pos[2] = (parw->m_pos[0]+parw->m_pos[2])/2;
	w->m_pos[3] = parw->m_pos[1] + 16 + 10 + 200;
}

//outputs
void Resize_BP_PB_O(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = (parw->m_pos[0]+parw->m_pos[2])/2;
	w->m_pos[1] = parw->m_pos[1] + 16 + 10 + 100;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[1] + 16 + 10 + 200;
}

//description block
void Resize_BP_Ds(Widget* w)
{
	Widget* parw = w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10;
	w->m_pos[1] = parw->m_pos[1] + 16 + 10 + 200;
	w->m_pos[2] = parw->m_pos[2] - 10;
	w->m_pos[3] = parw->m_pos[3] - 0;
}

BlPreview::BlPreview(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_BUILDPREVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;

	//add(new Viewport(this, "viewport", Resize_BP_VP, &DrawViewport, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, VIEWPORT_ENTVIEW));
	add(new Text(this, "title", RichText("Buildings"), MAINFONT32, Resize_BP_Tl, true, 0.9f, 0.7f, 0.3f, 1));
	add(new TextBlock(this, "conmat block", RichText("conmat block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_C, 1, 0, 0, 1));
	add(new TextBlock(this, "input block", RichText("input block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_I, 0, 1, 0, 1));
	add(new TextBlock(this, "output block", RichText("output block \n1 \n2 \n3"), MAINFONT16, Resize_BP_PB_O, 1, 0, 1, 1));
	add(new TextBlock(this, "desc block", RichText("description block \n1 \n2 \n3"), MAINFONT16, Resize_BP_Ds, 0, 1, 1, 1));

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void BlPreview::inev(InEv* ie)
{
	//for mobile where Out_BuiltButton might not be triggered because of touch jump
	//so any touch on any other part of the screen should close this
	if(ie->type == INEV_MOUSEDOWN /* && !m_over */ )
	{
		hide();
		return;
	}

	Win::inev(ie);
}