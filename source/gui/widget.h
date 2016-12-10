










#ifndef WIDGET_H
#define WIDGET_H

#include "../utils.h"
#include "../texture.h"
#include "font.h"
#include "../render/shader.h"
#include "../window.h"
#include "draw2d.h"
#include "richtext.h"
#include "inevent.h"

#define MAX_OPTIONS_SHOWN	7

#define WIDGET_IMAGE				1
#define WIDGET_BUTTON				2
#define WIDGET_TEXT					3
#define WIDGET_LINK					4
#define WIDGET_DROPLIST				5
#define WIDGET_EDITBOX				6
#define WIDGET_BARBUTTON			7
#define WIDGET_HSCROLLER			8
#define WIDGET_TOUCHLISTENER		9
#define WIDGET_TEXTBLOCK			10
#define WIDGET_CHECKBOX				11
#define WIDGET_INSDRAW				12
#define WIDGET_LISTBOX				13
#define WIDGET_TEXTAREA				14
#define WIDGET_VIEWPORT				15
#define WIDGET_FRAME				16
#define WIDGET_RESTICKER			17
#define WIDGET_BOTTOMPANEL			18
#define WIDGET_BUILDPREVIEW			19
#define WIDGET_CONSTRUCTIONVIEW		20
#define WIDGET_GUI					21
#define WIDGET_VIEWLAYER			22
#define WIDGET_WINDOW				23
#define WIDGET_VSCROLLBAR			24
#define WIDGET_HSCROLLBAR			25
#define WIDGET_BUILDINGVIEW			26
#define WIDGET_SVLISTVIEW			27
#define WIDGET_NEWHOST				28
#define WIDGET_TRUCKMGR				29
#define WIDGET_SAVEVIEW				30
#define WIDGET_LOADVIEW				31
#define WIDGET_LOBBY				32
#define WIDGET_PANE					33
#define WIDGET_BLGRAPHS				34
#define WIDGET_GENGRAPHS			35
#define WIDGET_PYGRAPHS				36

#define CHCALL_VSCROLL				0
#define CHCALL_HSCROLL				1

struct Widget
{
	/* TODO trim members delegate to sub's */

	int type;
	Widget* parent;
	float pos[4];
	float crop[4];
	float scar[4];	//scrolling area rect for windows
	float texc[4];	//texture coordinates
	float tpos[4];	//text pos
	unsigned int tex;
	unsigned int bgtex;
	unsigned int bgovertex;
	ecbool over;
	ecbool ldown;	//was the left mouse button pressed while over this (i.e. drag)?
	std::string name;
	RichText text;
	int font;
	unsigned int frametex, filledtex, uptex, downtex;
	ecbool opened;
	ecbool hidden;
	Vector options; /* RichText */
	int selected;
	float scroll[2];
	ecbool mousescroll;
	float vel[2];
	int param;
	float rgba[4];
	char* value; /* RichText */
	int caret;
	ecbool passw;
	int maxlen;
	ecbool shadow;
	List sub;	/* Widget* */
	int lines;
	RichText label;
	ecbool popup;
	void* extra;	/* extra user params */
		
	void (*clickfunc)();
	void (*clickfunc2)(int p);
	void (*overfunc)();
	void (*overfunc2)(int p);
	void (*outfunc)();
	void (*changefunc)();
	void (*changefunc2)(int p);
	void (*reframefunc)(Widget* w);
	void (*clickfunc3)(Widget* w);
};

void Widget_init(Widget* w);
void Widget_free(Widget *w);
void Widget_draw(Widget *w);
void Widget_drawover(Widget *w);
void Widget_inev(Widget *w, InEv* ie);
void Widget_frameupd(Widget *w);
void Widget_reframe(Widget *w);	//resized or moved
void Widget_subframe(Widget *w, float* fr)
{
	memcpy((void*)fr, (void*)pos, sizeof(float)*4);
}
Widget* Widget_get(Widget *w, const char* name);
void Widget_add(Widget *w, Widget* neww);
void Widget_hide(Widget *w);
void Widget_show(Widget *w);
void Widget_chcall(Widget *w, Widget* ch, int type, void* data);	/* child callback */
void Widget_freech(Widget *w);	/* free subwidget children */
void Widget_tofront(Widget *w);	/* only used for windows. edit: needed for everything since droplist uses it on parent tree. */
void Widget_hideall(Widget *w);
void Widget_gainfocus(Widget *w);
void Widget_losefocus(Widget *w);

void CenterLabel(Widget* w);
void SubCrop(float *src1, float *src2, float *ndest);

#endif
