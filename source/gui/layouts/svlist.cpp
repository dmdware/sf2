










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
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/appviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "svlist.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../net/sendpackets.h"
#include "../../../net/packets.h"
#include "../../../net/client.h"
#include "../../layouts/appgui.h"
#include "../../../language.h"
#include "../../../net/download.h"
#include "lobby.h"

bool g_reqsvlist = false;	//do we want to request a server list?
bool g_reqdnexthost = false;

//when renewing / quick-refreshing, this is
//the list of sv's to get info again from.
//first one is current, set to ->replied = false;
//when replied, set replied = true and erase pointer
//off this list.
std::list<SvInfo*> g_togetsv;

#if 0
//server list join button
void Resize_SL_JoinBut(Widget* w)
{
	SvList* parw = (SvList*)w->m_parent;

	w->m_pos[0] = parw->m_pos[2] - 70;
	w->m_pos[1] = parw->m_pos[3] - 30;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];

	CenterLabel(w);
}
#endif

void Click_SL_Add()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");

	EditBox* addrbox = &v->m_addrbox;
	EditBox* portbox = &v->m_portbox;
	std::string addrstr = addrbox->m_value.rawstr();

	if(addrstr.size() <= 0)
		return;

	std::string portstr = portbox->m_value.rawstr();
	int32_t port = PORT;
	sscanf(portstr.c_str(), "%d", &port);

	IPaddress ip;

	if(SDLNet_ResolveHost(&ip, addrstr.c_str(), port) == -1)
	{
		//char msg[1280];
		//sprintf(msg, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		//ErrMess("Error", msg);
		return;
	}

	//check if IP+port already added
	for(std::list<SvInfo>::iterator sit=v->m_svlist.begin(); sit!=v->m_svlist.end(); sit++)
	{
		if(!Same(&sit->addr, 0, &ip, 0))
			continue;

		return;	//already added
	}

	NetConn* prevnc = Match(&ip, 0);

	if(prevnc)
	{
		Disconnect(prevnc);
	}

	SvInfo sv;
	sv.addr = ip;
	sv.dock = 0;
	sv.replied = false;
	sv.name = addrbox->m_value + RichText(":") + portbox->m_value;
	sv.pingrt = RichText("???");
	v->m_svlist.push_back(sv);
	Connect(&ip, false, false, false, true);
	v->regen();
}

void Click_SL_Clear()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	v->m_selsv = NULL;
	v->m_svlist.clear();
	v->regen();

	// no? only need to get sv list or send keepalive for our host's listing
	//this will flush any queued packets
	if(g_mmconn && g_netmode != NETM_HOST)
		Disconnect(g_mmconn);
}

void Click_SL_Ref()	//refresh
{
	Click_SL_Clear();

	//g_needsvlist = true;
	//g_reqdsvlist = false;

	//Connect("localhost", PORT, false, true, false, false);
	//Connect(SV_ADDR, PORT, true, false, false, false);

	g_reqsvlist = true;
	//g_reqdnexthost = true;
	g_reqdnexthost = false;

	if(!g_mmconn)
		Connect(SV_ADDR, PORT, true, false, false, false);
	else if(!g_mmconn->handshook)
		return;
	else
	{
		g_reqdnexthost = true;
		GetSvListPacket gslp;
		gslp.header.type = PACKET_GETSVLIST;
		SendData((char*)&gslp, sizeof(GetSvListPacket), &g_mmconn->addr, true, false, g_mmconn, &g_sock, 0, NULL);
	}
}

void Click_SL_LANRef()	//LAN refresh
{
	Click_SL_Clear();

	OpenSock();

	for(int32_t i=0; i<NPORTS; i++)
	{
		//limited broadcast IP address
		//uint32_t broadip = (uint32_t)-1;
		uint16_t port = PORT + i;
	
		IPaddress addr;
		//addr.host = ntohl(broadip);
		addr.host = INADDR_BROADCAST;
		addr.port = ntohs(port);
		//addr.port = (port);
		SDLNet_ResolveHost(&addr, "255.255.255.255", port); 
		//SDLNet_ResolveHost(&addr, "localhost", port); 

		LANCallPacket lcp;
		lcp.header.type = PACKET_LANCALL;

		SendData((char*)&lcp, sizeof(LANCallPacket), &addr, false, true, NULL, &g_sock, 0, NULL);
	}
}

void Click_SL_QRef()	//qk refresh
{
	g_reqsvlist = false;

	//g_needsvlist = false;
	//g_reqdsvlist = false;

	// TO DO get info again


	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");
	v->m_selsv = NULL;
	//v->m_svlist.clear();

	for(std::list<SvInfo>::iterator sit=v->m_svlist.begin(); sit!=v->m_svlist.end(); sit++)
	{
		SvInfo *sv = &*sit;

		if(!sv->replied)
			continue;

		Connect(&sv->addr, false, false, false, true);

		GetSvInfoPacket gsip;
		gsip.header.type = PACKET_GETSVINFO;
		SendData((char*)&gsip, sizeof(GetSvInfoPacket), &sv->addr, true, false, Match(&sv->addr, sv->dock), &g_sock, 100, NULL);
	}
}

void Click_SL_Join()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SvList* v = (SvList*)gui->get("sv list");

	if(!v->m_selsv)
		return;

	SvInfo* sinfo = v->m_selsv;

#if 1
	if(g_netmode == NETM_HOST)
		return;	//TO DO quit host

	if(g_netmode == NETM_CLIENT && g_svconn)
	{
		Disconnect(g_svconn);
		g_svconn = NULL;
		EndSess();
	}

	//Click_NewGame();
	g_netmode = NETM_CLIENT;

	std::string svname = sinfo->name.rawstr();
	strcpy(g_svname, svname.c_str());
	g_svname[SVNAME_LEN] = 0;

	NetConn* svnc = Connect(&sinfo->addr, false, true, false, false);

	//NetConn* svnc = Match(&sinfo->addr, );

	//if we already have a connection
	if(svnc->handshook)
	{
		JoinPacket jp;
		jp.header.type = PACKET_JOIN;
		SendData((char*)&jp, sizeof(JoinPacket), &sinfo->addr, true, false, svnc, &g_sock, 0, NULL);
	}

	//Connect("localhost", PORT, false, true, false, false);
	//BegSess();
	ResetCls();

	gui->hideall();
	gui->show("join");

	ViewLayer* join = (ViewLayer*)gui->get("join");
	Text* status = (Text*)join->get("status");
	status->m_text = (STRTABLE[STR_JOINING]);

	g_downmap = false;

#else
	Connect(&sinfo->addr, false, true, false, false);
#endif
}

SvList::SvList(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_SVLISTVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

	m_svlistbg = Image(this, "", "gui/backg/svlistbg.jpg", true, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selsv = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_joinbut = Button(this, "join but", "gui/transp.png", (STRTABLE[STR_JOIN]), (STRTABLE[STR_JOINCURGAME]), m_font, BUST_LINEBASED, NULL, Click_SL_Join, NULL, NULL, NULL, NULL, -1, NULL);

#if 0
	SvInfo sv;
	sv.name = RichText(UStr("asdjasld9a0f09230jf3"));
	sv.pingrt = RichText(UStr("124"));
	sv.nplayersrt = RichText(UStr("3/32"));
	sv.mapnamert = RichText(UStr("wr_fluffy"));
	m_svlist.push_back(sv);
	sv.name = RichText(UStr("Joe's"));
	sv.pingrt = RichText(UStr("340"));
	sv.nplayersrt = RichText(UStr("4/48"));
	sv.mapnamert = RichText(UStr("bz_dunes"));
	m_svlist.push_back(sv);
	//m_selsv = &*m_svlist.rbegin();
	sv.name = RichText(UStr("Bob's"));
	sv.pingrt = RichText(UStr("15"));
	sv.nplayersrt = RichText(UStr("2/12"));
	sv.mapnamert = RichText(UStr("bz_hills"));
	m_svlist.push_back(sv);
#endif

	char portstr[16];
	sprintf(portstr, "%d", (int32_t)(PORT+(rand()*GetTicks())%NPORTS));
	m_addrlab = Text(this, "addr lab", (STRTABLE[STR_ADDRESS]), MAINFONT16, NULL);
	m_portlab = Text(this, "port lab", (STRTABLE[STR_PORT]), MAINFONT16, NULL);
	m_addrbox = EditBox(this, "addr box", RichText(""), MAINFONT16, NULL, false, 64, NULL, NULL, -1);
	m_portbox = EditBox(this, "port box", RichText(portstr), MAINFONT16, NULL, false, 8, NULL, NULL, -1);
	m_addbut = Button(this, "add but", "gui/transp.png", (STRTABLE[STR_ADD]), (STRTABLE[STR_ADDSVDESC]), m_font, BUST_LINEBASED, NULL, Click_SL_Add, NULL, NULL, NULL, NULL, -1, NULL);
	m_clearbut = Button(this, "clear but", "gui/transp.png", (STRTABLE[STR_CLEAR]), (STRTABLE[STR_CLEARSVDESC]), m_font, BUST_LINEBASED, NULL, Click_SL_Clear, NULL, NULL, NULL, NULL, -1, NULL);
	m_qrefbut = Button(this, "qref but", "gui/transp.png", (STRTABLE[STR_QREF]), (STRTABLE[STR_REFSVDESC]), m_font, BUST_LINEBASED, NULL, Click_SL_QRef, NULL, NULL, NULL, NULL, -1, NULL);
	m_refbut = Button(this, "ref but", "gui/transp.png", (STRTABLE[STR_REFRESH]), (STRTABLE[STR_GETSVDESC]), m_font, BUST_LINEBASED, NULL, Click_SL_Ref, NULL, NULL, NULL, NULL, -1, NULL);
	m_lanrefbut = Button(this, "lan ref but", "gui/transp.png", (STRTABLE[STR_LANREF]), (STRTABLE[STR_SRLANDESC]), m_font, BUST_LINEBASED, NULL, Click_SL_LANRef, NULL, NULL, NULL, NULL, -1, NULL);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void SvList::inev(InEv* ie)
{
	m_vscroll.inev(ie);
	m_joinbut.inev(ie);
	m_addbut.inev(ie);
	m_addrlab.inev(ie);
	m_portlab.inev(ie);
	m_addrbox.inev(ie);
	m_portbox.inev(ie);
	m_clearbut.inev(ie);
	m_qrefbut.inev(ie);
	m_refbut.inev(ie);
	m_lanrefbut.inev(ie);

	Player* py = &g_player[g_localP];

	if(!ie->intercepted)
	{
		if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_listtop && g_mouse.y <= m_listbot)
			{
				m_ldown = true;
				int32_t sin = 0;
				BmpFont* f = &g_font[m_font];
				float scrollspace = m_listbot - m_listtop;
				int32_t viewable = scrollspace / f->gheight;
				float scroll = m_scroll[1];
				int32_t viewi = 0;

				for(std::list<SvInfo>::iterator sit=m_svlist.begin(); sit!=m_svlist.end(); sit++, sin++)
				{
					if(sin < scroll)
						continue;

					if(sin - scroll > viewable)
						break;

					if(g_mouse.y >= m_listtop + (sin-scroll)*f->gheight &&
						g_mouse.y <= m_listtop + (sin-scroll+1.0f)*f->gheight)
					{
						m_selsv = &*sit;
						ie->intercepted = true;
						break;
					}

					viewi++;
				}
			}
		}
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
		{
			if(m_ldown)
				ie->intercepted = true;
		}
	}

	if(ie->type == INEV_MOUSEMOVE)
	{
	}

	Win::inev(ie);
}

void SvList::regen()
{
	RichText bname;
	int32_t* price;

	Player* py = &g_player[g_localP];
	
	//CreateTex(g_texstop, "gui/accept.png", true, false);
	//CreateTex(g_texplay, "gui/cancel.png", true, false);

#if 0
	if(b->type == BL_TRFAC)
	{
		char rowname[32];
		sprintf(rowname, "%d %d", col, 0);
		RichText label;

		Resource* r = &g_resource[RES_DOLLARS];
#if 0
		Button(Widget* parent, const char* name, const char* filepath, const RichText label, const RichText tooltip,int32_t f, int32_t style, void (*reframef)(Widget* w), void (*click)(), void (*click2)(int32_t p), void (*overf)(), void (*overf2)(int32_t p), void (*out)(), int32_t parm);
#endif
		add(new Button(this, rowname, "gui/transp.png", RichText(UStr("Order Truck for ")) + RichText(RichPart(RICH_ICON, r->icon)) + RichText(UStr(":")), RichText(), m_font, BUST_LINEBASED, Resize_BV_Cl, NULL, NULL, NULL, NULL, NULL, UNIT_TRUCK));

		sprintf(rowname, "%d %d", col, 1);
		char clabel[64];
		sprintf(clabel, "%d", b->manufprc[UNIT_TRUCK]);

		if(owned)
			add(new EditBox(this, rowname, RichText(UStr(clabel)), m_font, Resize_BV_Cl, false, 6, Change_BV_MP, NULL, UNIT_TRUCK));
		else
			add(new Text(this, rowname, RichText(UStr(clabel)), m_font, Resize_BV_Cl));

		col++;
	}
#endif

	freech();

	reframe();
}

void SvList::frameupd()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();

	m_vscroll.frameupd();

	m_scroll[1] = m_vscroll.m_scroll[1] * m_svlist.size() * (1.0f);
}

void SvList::reframe()
{
	Win::reframe();

	BmpFont* f = &g_font[m_font];
	m_listtop = m_pos[1] + f->gheight;
	m_listbot = m_pos[3] - SVLISTBOT;

	Texture* im = &g_texture[ m_svlistbg.m_tex ];
	m_svlistbg.m_pos[0] = m_pos[0];
	m_svlistbg.m_pos[1] = m_pos[1];
	m_svlistbg.m_pos[2] = m_pos[2];
	m_svlistbg.m_pos[3] = m_pos[3];
	m_svlistbg.m_texc[2] = (m_pos[2]-m_pos[0])/im->width;
	m_svlistbg.m_texc[3] = (m_pos[3]-m_pos[1])/im->height;

	m_vscroll.m_pos[0] = m_pos[2] - 16;
	m_vscroll.m_pos[1] = m_listtop;
	m_vscroll.m_pos[2] = m_pos[2];
	m_vscroll.m_pos[3] = m_listbot;
	float scrollable = f->gheight * m_svlist.size();
	float viewable = m_listbot - m_listtop;
	m_vscroll.m_domain = fmin(1, viewable / scrollable);
	m_vscroll.reframe();

	//m_joinbut.reframe();
	m_joinbut.m_pos[0] = m_pos[2] - 130;
	m_joinbut.m_pos[1] = m_pos[3] - 30;
	m_joinbut.m_pos[2] = m_pos[2];
	m_joinbut.m_pos[3] = m_pos[3];
	CenterLabel(&m_joinbut);
	m_joinbut.reframe();

	m_addbut.m_pos[0] = m_pos[2] - 90;
	m_addbut.m_pos[1] = m_pos[3] - 30 - 30 - 10;
	m_addbut.m_pos[2] = m_pos[2];
	m_addbut.m_pos[3] = m_pos[3] - 30 - 10;
	CenterLabel(&m_addbut);
	m_addbut.reframe();

#if 0
	Text m_addrlab;
	Text m_portlab;
	EditBox m_addrbox;
	EditBox m_portbox;
	Button m_addbut;
#endif

	m_addrlab.m_pos[0] = m_pos[0] + 0;
	m_addrlab.m_pos[1] = m_pos[3] - 30 - 30 - 10;
	m_addrlab.m_pos[2] = m_pos[0] + 60;
	m_addrlab.m_pos[3] = m_pos[3] - 30 - 10;
	m_addrlab.reframe();

	m_addrbox.m_pos[0] = m_pos[0] + 60;
	m_addrbox.m_pos[1] = m_pos[3] - 30 - 30 - 10;
	m_addrbox.m_pos[2] = m_pos[0] + 60 + 150;
	m_addrbox.m_pos[3] = m_pos[3] - 30 - 30 - 10 + 16;
	m_addrbox.reframe();

	m_portlab.m_pos[0] = m_pos[0] + 0;
	m_portlab.m_pos[1] = m_pos[3] - 30 - 30 - 10 + 20;
	m_portlab.m_pos[2] = m_pos[0] + 60;
	m_portlab.m_pos[3] = m_pos[3] - 30 - 10 + 20;
	m_portlab.reframe();

	m_portbox.m_pos[0] = m_pos[0] + 60;
	m_portbox.m_pos[1] = m_pos[3] - 30 - 30 - 10 + 20;
	m_portbox.m_pos[2] = m_pos[0] + 60 + 60;
	m_portbox.m_pos[3] = m_pos[3] - 30 - 30 - 10 + 16 + 20;
	m_portbox.reframe();

#if 0
	Button m_clearbut;
	Button m_qrefbut;
	Button m_refbut;
#endif

	m_clearbut.m_pos[0] = m_pos[2] - 130 - 10 - 70;
	m_clearbut.m_pos[1] = m_pos[3] - 30;
	m_clearbut.m_pos[2] = m_pos[2] - 10 - 130;
	m_clearbut.m_pos[3] = m_pos[3];
	CenterLabel(&m_clearbut);
	m_clearbut.reframe();

	m_qrefbut.m_pos[0] = m_pos[2] - 130 - 10 - 70 - 10 - 130;
	m_qrefbut.m_pos[1] = m_pos[3] - 30;
	m_qrefbut.m_pos[2] = m_pos[2] - 10 - 130 - 10 - 70;
	m_qrefbut.m_pos[3] = m_pos[3];
	CenterLabel(&m_qrefbut);
	m_qrefbut.reframe();

	m_refbut.m_pos[0] = m_pos[2] - 130 - 10 - 70 - 10 - 130 - 10 - 70 - 140;
	m_refbut.m_pos[1] = m_pos[3] - 30;
	m_refbut.m_pos[2] = m_pos[2] - 10 - 130 - 10 - 130 - 10 - 70;
	m_refbut.m_pos[3] = m_pos[3];
	CenterLabel(&m_refbut);
	m_refbut.reframe();

	//TODO clean up
	//TODO read todo's some time
	
	m_lanrefbut.m_pos[0] = m_pos[2] - 10 - 130 - 90 - 10;
	m_lanrefbut.m_pos[1] = m_pos[3] - 30 - 30 - 10;
	m_lanrefbut.m_pos[2] = m_pos[2] - 90 - 10;
	m_lanrefbut.m_pos[3] = m_pos[3] - 30 - 10;
	CenterLabel(&m_lanrefbut);
	m_lanrefbut.reframe();
}

void SvList::draw()
{
	Win::draw();
	//m_svlistbg.draw();

	//2015/10/27 fixed now sv list text is cropped

	if(g_reqsvlist && g_reqdnexthost)
	{
		if(!g_mmconn || !g_mmconn->handshook)
		{
			RichText stat = (STRTABLE[STR_CONTOMATCH]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
		else
		{
			RichText stat = (STRTABLE[STR_CONMATCHGETNEXT]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
	}
	else if(g_reqsvlist)
	{	
		if(!g_mmconn || !g_mmconn->handshook)
		{
			RichText stat = (STRTABLE[STR_CONMATCHREQSV]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
		else
		{
			RichText stat = (STRTABLE[STR_GETSVLIST]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
	}
	else
	{
		if(!g_mmconn || !g_mmconn->handshook)
		{
			RichText stat = (STRTABLE[STR_NOCONMATCH]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
		else
		{
			RichText stat = (STRTABLE[STR_CONMATCH]);
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1], m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
		}
	}

	if(m_svlist.size() <= 0)
	{
			RichText stat = (STRTABLE[STR_NOSV]);
			BmpFont* f = &g_font[MAINFONT16];
			DrawShadowedTextF(MAINFONT16, m_pos[0], m_pos[1] + f->gheight, m_crop[0], m_crop[1], m_crop[2], m_crop[3], &stat);
	}

	int32_t sin = 0;
	BmpFont* f = &g_font[m_font];
	float scrollspace = m_listbot - m_listtop;
	int32_t viewable = scrollspace / f->gheight;
	float scroll = m_scroll[1];
	int32_t viewi = 0;

	for(std::list<SvInfo>::iterator sit=m_svlist.begin(); sit!=m_svlist.end(); sit++, sin++)
	{
		if(sin < scroll)
			continue;

		if(sin - scroll > viewable)
			break;

		if(&*sit == m_selsv)
		{
			UseS(SHADER_COLOR2D);
			Player* py = &g_player[g_localP];

			Shader* s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			//glUniform4f(g_shader[SHADER_ORTHO].slot[SSLOT_COLOR], 0.2f, 1.0f, 0.2f, 0.6f);

			DrawSquare(0.2f, 1.0f, 0.2f, 0.6f, m_pos[0], m_listtop + (sin-scroll)*f->gheight, m_pos[2], m_listtop + (sin-scroll+1.0f)*f->gheight, m_crop);

			Ortho(g_width, g_height, 1, 1, 1, 1);
		}

		if(sit->started)
			DrawImage(g_texture[g_texplay].texname, m_pos[0], (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0] + f->gheight, (int32_t)(m_listtop + (sin-scroll+1)*f->gheight), 0, 0, 1, 1, m_crop);
		else
			DrawImage(g_texture[g_texstop].texname, m_pos[0], (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0] + f->gheight, (int32_t)(m_listtop + (sin-scroll+1)*f->gheight), 0, 0, 1, 1, m_crop);

		float namecolor[4] = {0.9f, 0.6f, 0.2f, 0.9f};
		DrawShadowedTextF(m_font, m_pos[0] + 32, (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &sit->name, namecolor);

#if 1
		DrawShadowedTextF(m_font, m_pos[0] + 150, (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &sit->mapnamert, namecolor);

		DrawShadowedTextF(m_font, m_pos[0] + 380, (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &sit->nplayersrt, namecolor);

		DrawShadowedTextF(m_font, m_pos[0] + 430, (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &sit->pingrt, namecolor);

#else

		DrawShadowedTextF(m_font, m_pos[0] + 250, (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &sit->pingrt, namecolor);
#endif

		viewi++;
	}

	m_vscroll.draw();
	m_joinbut.draw();
	m_addbut.draw();
	m_addrlab.draw();
	m_portlab.draw();
	m_addrbox.draw();
	m_portbox.draw();
	m_clearbut.draw();
	m_refbut.draw();
	m_lanrefbut.draw();
	m_qrefbut.draw();
}

void SvList::drawover()
{
	Win::drawover();

	m_vscroll.drawover();
	m_joinbut.drawover();
	m_addbut.drawover();
	m_addrlab.drawover();
	m_portlab.drawover();
	m_addrbox.drawover();
	m_portbox.drawover();
	m_clearbut.drawover();
	m_refbut.drawover();
	m_lanrefbut.drawover();
	m_qrefbut.drawover();
}
