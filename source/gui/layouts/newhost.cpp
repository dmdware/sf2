










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
#include "newhost.h"
#include "blpreview.h"
#include "../../icon.h"
#include "../../../net/sendpackets.h"
#include "lobby.h"
#include "../../../net/client.h"
#include "../../layouts/appgui.h"
#include "../../../save/savemap.h"
#include "../../../language.h"
#include "roleview.h"
#include "../../layouts/messbox.h"
#include "saveview.h"

//new host create button
void Resize_NH_Create(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[2] - 70;
	w->m_pos[1] = parw->m_pos[1] + 30*2;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[1] + 30*3;

	CenterLabel(w);
}

//new host game name edit box
void Resize_NH_GameName(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 110;
	w->m_pos[1] = parw->m_pos[1];
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[1] + 30 - 10;

	CenterLabel(w);
}

//new host game name label
void Resize_NH_GNLab(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1];
	w->m_pos[2] = parw->m_pos[0] + 110;
	w->m_pos[3] = parw->m_pos[1] + 30;

	CenterLabel(w);
}

//new host map name drop-down list
void Resize_NH_MapName(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 110;
	w->m_pos[1] = parw->m_pos[1] + 30;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[1] + 30 * 2 - 10;

	CenterLabel(w);
}

//new host LAN-only checkbox
void Resize_NH_LANOnly(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 10;
	w->m_pos[1] = parw->m_pos[1] + 30 * 2;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[1] + 30 * 3 - 10;

	CenterLabel(w);
}


//new host map name label
void Resize_NH_MpLab(Widget* w)
{
	NewHost* parw = (NewHost*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[1] + 30;
	w->m_pos[2] = parw->m_pos[0] + 110;
	w->m_pos[3] = parw->m_pos[1] + 30 * 2;

	CenterLabel(w);
}

void Click_NH_Create()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	NewHost* v = (NewHost*)gui->get("new host");

	if(v->m_map.m_selected < 0)
		return;

	int32_t mapi = v->m_map.m_selected;
	RichText* maprich = &v->m_map.m_options[ mapi ];
	std::string mapname = maprich->rawstr();
	strcpy(g_mapname, mapname.c_str());
	
	char maprelative[1024];
	sprintf(maprelative, "%s%s", SAVEMODEPATH[SAVEMODE_MAPS], mapname.c_str());
	
	if(!LoadMap(maprelative))
	{
		RichText m = STRTABLE[STR_ERRLOADM];
		Mess(&m);
		return;
	}

	//2015/10/26 fix now quick saving in multiplayer doesn't overwrite loaded map file
	g_lastsave[0] = 0;

	//g_appmode = APPMODE_PLAY;
	//Click_NewGame();

	g_netmode = NETM_HOST;
	g_lanonly = (bool)v->m_lanonly.m_selected;

	//if(!g_sock)
	OpenSock();

	g_sentsvinfo = false;

	CheckAddSv();

	gui->hideall();
	gui->show("lobby");
	gui->show("chat");
	//gui->show("role");
	((RoleView*)gui->get("role"))->regen();
	Lobby_Regen();

	//BegSess();
	ResetCls();
	AddClient(NULL, g_name, &g_localC, CLPLATI);
	AssocPy(NewClPlayer(), g_localC);

	//contact matcher TO DO
}

NewHost::NewHost(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_NEWHOST;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

#if 0
	m_svlistbg = Image(this, "", "gui/mmbg.jpg", true, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selsv = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_joinbut = Button(this, "join but", "gui/transp.png", RichText("Join"), RichText("Join the currently selected game"), m_font, BUST_LINEBASED, Resize_SL_JoinBut, Click_SL_Join, NULL, NULL, NULL, NULL, -1);

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

	m_create = Button(this, "create", "gui/transp.png", STRTABLE[STR_CREATE], STRTABLE[STR_MAKEPUB], m_font, BUST_LINEBASED, Resize_NH_Create, Click_NH_Create, NULL, NULL, NULL, NULL, -1, NULL);
	m_gamename = EditBox(this, "game name", STRTABLE[STR_GAMEROOM], MAINFONT16, Resize_NH_GameName, false, SVNAME_LEN, NULL, NULL, -1);
	m_gnlab = Text(this, "gnlab", STRTABLE[STR_GAMENAME], MAINFONT16, Resize_NH_GNLab);
	m_maplabel = Text(this, "maplabel", STRTABLE[STR_MAP], MAINFONT16, Resize_NH_MpLab);
	m_map = DropList(this, "map", MAINFONT16, Resize_NH_MapName, NULL);
	m_lanonly = CheckBox(this, "lan only", STRTABLE[STR_LANONLYNOTPUB], MAINFONT16, Resize_NH_LANOnly, 0);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void NewHost::inev(InEv* ie)
{
	//m_vscroll.inev(ie);

	m_create.inev(ie);
	m_gamename.inev(ie);
	m_gnlab.inev(ie);
	m_maplabel.inev(ie);
	m_map.inev(ie);
	m_lanonly.inev(ie);

	Win::inev(ie);
}

void NewHost::regen()
{
	freech();

	m_map.m_options.clear();
	m_map.m_selected = -1;

	std::list<std::string> files;
	char full[WF_MAX_PATH+1];
	FullWritePath(SAVEMODEPATH[SAVEMODE_MAPS], full);
	ListFiles(full, files);

	for(std::list<std::string>::iterator fit=files.begin(); fit!=files.end(); fit++)
	{
		//InfoMess(fit->c_str(), fit->c_str());
		RichText mapname = RichText(fit->c_str());
		m_map.m_options.push_back(mapname);
	}

	reframe();
}

void NewHost::reframe()
{
	Win::reframe();

	BmpFont* f = &g_font[m_font];

	m_create.reframe();
	m_gamename.reframe();
	m_gnlab.reframe();
	m_maplabel.reframe();
	m_map.reframe();
	m_lanonly.reframe();
}

void NewHost::draw()
{
	Win::draw();

	m_create.draw();
	m_gamename.draw();
	m_gnlab.draw();
	m_maplabel.draw();
	m_map.draw();
	m_lanonly.draw();
}

void NewHost::drawover()
{
	Win::drawover();

	m_create.drawover();
	m_gamename.drawover();
	m_gnlab.drawover();
	m_maplabel.drawover();
	m_map.drawover();
	m_lanonly.drawover();
}
