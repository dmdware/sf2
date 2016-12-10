










#ifndef SVLISTVIEW_H
#define SVLISTVIEW_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../../sim/selection.h"
#include "../winw.h"

#define SVLISTBOT	60 //sv list view bottom space for buttons and address+port fields

class SvInfo
{
public:
	bool replied;
	IPaddress addr;
	uint16_t dock;
	RichText name;
	int16_t nplayers;
	int16_t nmaxpys;
	RichText nplayersrt;
	std::list<RichText> players;
	int32_t ping;
	RichText pingrt;
	RichText mapnamert;
	bool started;

	SvInfo()
	{
		replied = false;
		nplayers = 0;
		nmaxpys = 0;
		started = false;
		dock = 0;
	}
};

class SvList : public Win
{
public:
	SvList(Widget* parent, const char* n, void (*reframef)(Widget* w));

	//when persistance of internal widget states is important between calls to "regen()",
	//then children widgets must be stored as members, rather than items in the m_subwidg list,
	//because the list is destroyed and regenerated.
	//in this case we need to keep the state of the scroll bar. the other widgets like the join button
	//probably don't need to be member variables.
	Image m_svlistbg;
	std::list<SvInfo> m_svlist;
	SvInfo* m_selected;
	VScroll m_vscroll;
	SvInfo* m_selsv;	//selected sv
	float m_listbot;	//bottom y screen coord of list items
	float m_listtop;
	Button m_joinbut;

	Text m_addrlab;
	Text m_portlab;
	EditBox m_addrbox;
	EditBox m_portbox;
	Button m_addbut;
	Button m_clearbut;
	Button m_qrefbut;
	Button m_refbut;
	Button m_lanrefbut;

	void regen();
	virtual void draw();
	virtual void drawover();
	virtual void reframe();
	virtual void inev(InEv* ie);
	virtual void frameupd();
};

extern std::list<SvInfo*> g_togetsv;
extern bool g_reqsvlist;
extern bool g_reqdnexthost;

#endif
