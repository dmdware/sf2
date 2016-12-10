










#include "widget.h"
#include "gui.h"
#include "font.h"
#include "../window.h"
#include "icon.h"

void Widget_init(Widget* w)
{
	w->clickfunc = NULL;
	w->clickfunc2 = NULL;
	w->overfunc = NULL;
	w->overfunc2 = NULL;
	w->outfunc = NULL;
	w->changefunc = NULL;
	w->changefunc2 = NULL;
	w->caret = 0;
	w->parent = NULL;
	w->opened = ecfalse;
	w->ldown = ecfalse;
	w->reframefunc = NULL;
	w->hidden = ecfalse;
	w->clickfunc3 = NULL;
	w->extra = NULL;
	w->value = NULL;
}

void Widget_free(Widget *w)
{
	Widget_freech(w);
	free(w->extra);
	free(w->value);
	w->extra = NULL;
	w->value = NULL;
}

void Widget::hideall()
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->hide();
}

void Widget::hide(const char* name)
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		if(stricmp((*i)->name.c_str(), name) == 0)
		{
			(*i)->hide();
		}
}

void Widget::show(const char* name)
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		if(stricmp((*i)->name.c_str(), name) == 0)
		{
			(*i)->show();
			break;	//important - list may shift after show() and tofront() call
		}
}

void Widget::frameupd()
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->frameupd();
}

void Widget::reframe()	//resized or moved
{
	if(reframefunc)
		reframefunc(this);

#if 1

	if(parent)
	{
		SubCrop(parent->crop, pos, crop);
	}
	else
	{
		crop[0] = 0;
		crop[1] = 0;
		crop[2] = (float)g_width-1;
		crop[3] = (float)g_height-1;
	}
#endif

#if 0	//only use when add windows widgets, and then fix parent bounds of "zoom text" and "max elev" labels
	if(parent)
	{
		float* parp = parent->pos;
		
		//must be bounded by the parent's frame

		pos[0] = fmax(parp[0], pos[0]);
		pos[0] = fmin(parp[2], pos[0]);
		pos[2] = fmax(parp[0], pos[2]);
		pos[2] = fmin(parp[2], pos[2]);
		pos[1] = fmax(parp[1], pos[1]);
		pos[1] = fmin(parp[3], pos[1]);
		pos[3] = fmax(parp[1], pos[3]);
		pos[3] = fmin(parp[3], pos[3]);

		pos[1] = fmin(pos[1], pos[3]);
		pos[0] = fmin(pos[0], pos[2]);
	}
#endif

	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->reframe();
}

void Widget::draw()
{
	for(std::list<Widget*>::iterator wit=sub.begin(); wit!=sub.end(); wit++)
	{
		Widget* w = *wit;

		if(w->hidden)
			continue;

		w->draw();
	}
}

void Widget::drawover()
{
	for(std::list<Widget*>::iterator wit=sub.begin(); wit!=sub.end(); wit++)
	{
		Widget* w = *wit;

		if(w->hidden)
			continue;

		w->drawover();
	}
}

void Widget::inev(InEv* ie)
{
	ecbool intercepted = ie->intercepted;
	
	//safe, may shift during call
	for(std::list<Widget*>::reverse_iterator wit=sub.rbegin(); wit!=sub.rend();)
	{
		Widget* w = *wit;
		wit++;	//safe, may shift during call

		if(w->hidden)
			continue;
		
		w->inev(ie);
		
		if(ie->intercepted != intercepted)
		{
			intercepted = ie->intercepted;
			wit = sub.rbegin();
		}
	}
}

void Widget::tofront()
{
	//return;

	if(!parent)
		return;

	//return;

	std::list<Widget*>* subs = &parent->sub;

	for(std::list<Widget*>::iterator wi=subs->begin(); wi!=subs->end(); wi++)
	{
		if(*wi == this)
		{
			subs->erase(wi);
			subs->push_back(this);
			break;
		}
	}
}

void CenterLabel(Widget* w)
{
	Font* f = &g_font[w->font];

	int texwidth = TextWidth(w->font, &w->label);

#if 0
	char msg[128];
	sprintf(msg, "tw %d, tl %d, fn %d, gh %f", texwidth, w->label.texlen(), w->font, f->gheight);
	if(g_netmode == NETM_CLIENT)
	InfoMess(msg, msg);
#endif

	w->tpos[0] = (w->pos[2]+w->pos[0])/2 - texwidth/2;
	w->tpos[1] = (w->pos[3]+w->pos[1])/2 - f->gheight/2;
}

Widget* Widget::get(const char* name)
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		if(stricmp((*i)->name.c_str(), name) == 0)
			return *i;

	return NULL;
}

void Widget::add(Widget* neww)
{
	if(!neww)
		OUTOFMEM();

	sub.push_back(neww);
}

void Widget::gainfocus()
{
}

//TODO lose focus win blview members edit boxes

void Widget::losefocus()
{
	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->losefocus();
}

void Widget::hide()
{
	hidden = ectrue;
	losefocus();
	
	//for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
	//	(*i)->hide();
}

void Widget::show()
{
	hidden = ecfalse;
	//necessary for window widgets:
	//tofront();	//can't break list iterator, might shift

	//for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
	//	(*i)->show();
}

void Widget::chcall(Widget* ch, int type, void* data)
{
}

//free subwidget children
void Widget::freech()
{
	std::list<Widget*>::iterator witer = sub.begin();
	while(witer != sub.end())
	{
		delete *witer;
		witer = sub.erase(witer);
	}
}

void SubCrop(float *src1, float *src2, float *dest)
{
	dest[0] = fmax(src1[0], src2[0]);
	dest[1] = fmax(src1[1], src2[1]);
	dest[2] = fmin(src1[2], src2[2]);
	dest[3] = fmin(src1[3], src2[3]);

	//purposely inverted frame means it's out of view
	
	//if(ndest[0] > ndest[2])
	//	ndest[0] = ndest[2]+1.0f;

	//if(ndest[1] > ndest[3])
	//	ndest[1] = ndest[3]+1.0f;
}