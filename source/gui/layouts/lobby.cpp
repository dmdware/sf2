










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
#include "../../../sys/unicode.h"
#include "../../../sys/utf8.h"
#include "../../../net/client.h"
#include "../../layouts/appgui.h"
#include "../../layouts/chattext.h"
#include "../../../app/appmain.h"
#include "../../../language.h"
#include "roleview.h"
#include "../../layouts/messbox.h"
#include "../../layouts/playgui.h"

//player lines top
#define PYLINESTOP	200
//player lines left start
#define PYLINESLEFT	100
//player lines seperator width
#define PYSEPW		110

#if 0
Image m_bg;
Image m_pybg;
float pylpos[4];	//player list pos
VScroll m_pylsc;	//player list scroller
float m_chpos[4];	//chat box pos
//TextBlock m_chls;	//chat lines
//Text m_chls[CHAT_LINES];
float m_chsl;	//chat start line number (scrolled)
VScroll m_chsc;	//chat lines scroll
Image m_chbg;
Text m_svname;	//server name/title
#endif

int32_t g_startin = -1;

uint32_t g_texstop;
uint32_t g_texplay;
uint32_t g_texfast;

void Resize_SvName(Widget* w)
{
	w->m_pos[0] = 20;
	w->m_pos[1] = 20;
	w->m_pos[2] = g_width - 20;
	w->m_pos[3] = g_height - 20;
	w->m_tpos[0] = 20;
	w->m_tpos[1] = 20;
}

void Resize_PyBG(Widget* w)
{
	w->m_pos[0] = g_width - 220;
	w->m_pos[1] = 20;
	w->m_pos[2] = g_width - 20;
	w->m_pos[3] = 420;
}

void Resize_LobbyQuit(Widget* w)
{
	w->m_pos[0] = g_width - 110;
	w->m_pos[1] = g_height - 50;
	w->m_pos[2] = g_width - 20;
	w->m_pos[3] = g_height - 10;
}

void Resize_LobbyReady(Widget* w)
{
	w->m_pos[0] = g_width - 220;
	w->m_pos[1] = g_height - 50;
	w->m_pos[2] = g_width - 130;
	w->m_pos[3] = g_height - 10;
}

/*
This is both end host for host
and exit lobby for client.
*/
void Click_JoinCancel()
{
	if(g_svconn)
	{
		Disconnect(g_svconn);
	}

	if(g_netmode == NETM_HOST)
	{
		for(int32_t i=0; i<CLIENTS; i++)
		{
			Client* c = &g_client[i];

			if(!c->on)
				continue;

			if(!c->nc)
				continue;

			Disconnect(c->nc);
		}

		if(g_mmconn)
		{
			//corpd fix
			//is this what lack of was causing "nat" problem?
			//faulty second call of Connect?
			Disconnect(g_mmconn);
		}
	}

	ResetCls();
	EndSess();
	//CleanSession();	//TODO free players

	g_netmode = NETM_SINGLE;

	GUI* gui = &g_gui;
	gui->hideall();
	gui->show("main");
}

void Click_Ready()
{
	//if(g_downlist.size() > 0)
	//	return;

	Client* c = &g_client[g_localC];

	if(c->downin >= 0)
	{
		//TODO translate
		RichText r = STRTABLE[STR_STILLDOWN];
		Mess(&r);
		return;
	}

	if(g_mapsz.x <= 0)
	{
		//TODO translate
		RichText r = STRTABLE[STR_MAPNOTLOADED];
		Mess(&r);
		return;
	}

	c->ready = !c->ready;

	ClStatePacket csp;
	csp.header.type = PACKET_CLSTATE;
	csp.client = g_localC;
	csp.chtype = c->ready ? CLCH_READY : CLCH_NOTREADY;
	csp.downin = c->downin;

	if(g_netmode == NETM_HOST)
		SendAll((char*)&csp, sizeof(ClStatePacket), true, false, NULL, 0);
	else if(g_netmode == NETM_CLIENT && g_svconn)
		SendData((char*)&csp, sizeof(ClStatePacket), &g_svconn->addr, true, false, g_svconn, &g_sock, 0, NULL);
}

//update lobby checks, check if it's time to start game etc.
void Lobby_Up()
{
	if(g_netmode != NETM_HOST)
		return;

	static uint64_t last = GetTicks();

	//check if we are ready to start
	bool ready = true;

	for(int32_t ci=0; ci<CLIENTS; ci++)
	{
		Client* c = &g_client[ci];

		if(!c->on)
			continue;

		if(c->ready)
			continue;

		ready = false;
		break;
	}

	if(!ready)
	{
		g_startin = -1;
		last = GetTicks();
		return;
	}

	if(g_startin < 0)
	{
		g_startin = 6 * 1000;	//6 seconds
		return;	//start count
	}

	uint64_t now = GetTicks();
	uint64_t passed = now - last;
	last = now;

	//check if we passed a new second
	int32_t lastsecs = g_startin / 1000;
	g_startin -= passed;

	if(g_startin / 1000 < lastsecs)
	{
		char cmsg[512];
		sprintf(cmsg, "%s %d...", 
			STRTABLE[STR_STARTINGIN].rawstr().c_str(), 
			g_startin / 1000);

		ChatPacket cp;
		cp.header.type = PACKET_CHAT;
		cp.client = -1;
		strcpy(cp.msg, cmsg);
		SendAll((char*)&cp, sizeof(ChatPacket), true, false, NULL, 0);

		RichText msg = RichText(cmsg);
		AddChat(&msg);
	}

	//time to start?
	if(g_startin <= 0)
	{
		g_startin = -1;
		//TODO start

		MapStartPacket msp;
		msp.header.type = PACKET_MAPSTART;
		SendAll((char*)&msp, sizeof(MapStartPacket), true, false, NULL, 0);
		
		BegSess();
		g_appmode = APPMODE_PLAY;
		GUI* gui = &g_gui;
		gui->hideall();
		gui->show("play");
		Click_NextBuildButton(0);
		gui->show("chat");
	}
}

void FillLobby()
{
	GUI* gui = &g_gui;
	gui->add(new ViewLayer(gui, "lobby"));
	ViewLayer* lobby = (ViewLayer*)gui->get("lobby");

	lobby->add(new Image(lobby, "bg", "gui/mmbg.jpg", true, Resize_Fullscreen));
	lobby->add(new Image(lobby, "", "gui/i-64x64.png", true, Resize_AppLogo));
	lobby->add(new Text(lobby, "app title", RichText(TITLE), MAINFONT32, Resize_AppTitle));
	lobby->add(new Image(lobby, "pybg", "gui/backg/white.jpg", true, NULL /*Resize_PyBG*/, 1.0f, 1.0f, 1.0f, 0.6f));
	lobby->add(new Text(lobby, "sv name", RichText(""), MAINFONT16, Resize_SvName));
	lobby->add(new InsDraw(lobby, Lobby_DrawPyL));
	lobby->add(new InsDraw(lobby, Lobby_DrawState));
	lobby->add(new Button(lobby, "ready", "gui/transp.png", STRTABLE[STR_READY], STRTABLE[STR_TOGGLEREADY], MAINFONT16, BUST_LINEBASED, Resize_LobbyReady, Click_Ready, NULL, NULL, NULL, NULL, -1, NULL));
	lobby->add(new Button(lobby, "quit", "gui/transp.png", STRTABLE[STR_QUIT3], STRTABLE[STR_EXITSESS], MAINFONT16, BUST_LINEBASED, Resize_LobbyQuit, Click_JoinCancel, NULL, NULL, NULL, NULL, -1, NULL));
	lobby->add(new InsDraw(lobby, Lobby_Up));

	gui->add(new RoleView(gui, "role", Resize_CenterWin));
}

void Lobby_Regen()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	ViewLayer* lobby = (ViewLayer*)gui->get("lobby");
	Text* svnamew = (Text*)lobby->get("sv name");

	if(g_netmode == NETM_HOST)
	{
		//ViewLayer* h = (ViewLayer*)gui->get("new host");
		//EditBox* n = (EditBox*)h->get("game name");
		NewHost* h = (NewHost*)gui->get("new host");
		EditBox* n = (EditBox*)&h->m_gamename;

		if(n->m_value.m_part.size() > 0)
		{
			//Log(" n->m_value.m_part.begin()->m_text.m_length = "<<n->m_value.m_part.begin()->m_text.m_length);
			//Log(" n->m_value.rawlen() = "<<n->m_value.rawlen());

			//unsigned char* utf8 = ToUTF8(n->m_value.m_part.begin()->m_text.m_data);
			//unsigned char* utf8 = ToUTF8((unsigned char*)n->m_value.rawstr().c_str());
			std::string rawstr = n->m_value.rawstr();
			unsigned char* utf8 = new unsigned char[ rawstr.length() + 1 ];
			strcpy((char*)utf8, rawstr.c_str());
			//unsigned char* utf8 = new unsigned char [ n->m_value.rawlen() * 4 + 1 ];
			//int32_t utf8len = wchar_to_utf8(n->m_value.m_part.begin()->m_text.m_data, n->m_value.rawlen(), (char*)utf8, n->m_value.rawlen() * 4, 0);
			//utf8[utf8len] = 0;
			//int32_t rawlen = n->m_value.rawlen();

			//ValidateUTF8(utf8, strlen((char*)utf8));

			//Log("n->m_value.m_part.begin()->m_text[0] = "<<n->m_value.m_part.begin()->m_text[0]);

			//Log(" strlen((char*)utf8) = "<<strlen((char*)utf8));

			if(strlen((char*)utf8) > SVNAME_LEN)
				utf8[SVNAME_LEN] = 0;

			//Log("utf8[0,1,2,3] = "<<(int32_t)utf8[0]<<","<<(int32_t)utf8[1]<<","<<(int32_t)utf8[2]<<","<<(int32_t)utf8[3]<<" = "<<(char)utf8[0]);

			strcpy(g_svname, (char*)utf8);

			//Log(" strlen(g_svname) = "<<strlen(g_svname));

			//Log("g_svname[0,1,2,3] = "<<(int32_t)g_svname[0]<<","<<(int32_t)g_svname[1]<<","<<(int32_t)g_svname[2]<<","<<(int32_t)g_svname[3]<<" = "<<(char)g_svname[0]);

			//uint32_t* utf32 = ToUTF32(utf8, strlen((char*)utf8));
			//uint32_t* utf32 = ToUTF32(utf8, rawlen);
			//uint32_t* utf32 = new uint32_t [ strlen((char*)utf8) + 1 ];
			//int32_t utf32len = utf8_to_wchar((char*)utf8, strlen((char*)utf8), utf32, strlen((char*)utf8), 0);
			//utf32[utf32len] = 0;
			uint32_t* utf32 = ToUTF32(utf8);
			delete [] utf8;

			svnamew->m_text = RichText(UStr(utf32));
			svnamew->m_text = ParseTags(svnamew->m_text, NULL);
			delete [] utf32;

			//Log(" m_svname.m_text.rawlen() = "<<m_svname.m_text.rawlen());
		}
		else
		{
			strcpy(g_svname, "");
			svnamew->m_text = RichText(UStr(g_svname));
		}
	}
	else if(g_netmode == NETM_CLIENT)
	{
		uint32_t* utf32 = ToUTF32((unsigned char*)g_svname);
		svnamew->m_text = RichText(UStr(utf32));
		svnamew->m_text = ParseTags(svnamew->m_text, NULL);
		delete [] utf32;
	}
}

void Lobby_DrawState()
{
	if(g_netmode != NETM_HOST)
		return;
	
	BmpFont* f = &g_font[MAINFONT16];
	
	if(g_lanonly)
	{
		RichText stat = (STRTABLE[STR_LANONLYNOTPUB]);
		float color[4] = {0,1,0,1};
		DrawShadowedText(MAINFONT16, g_width/2, f->gheight, &stat, color);
	}
	else if(!g_mmconn || !g_mmconn->handshook)
	{
		RichText stat = (STRTABLE[STR_NOMATCHSVNOT]);
		float color[4] = {1,0,0,1};
		DrawShadowedText(MAINFONT16, g_width/2, f->gheight, &stat, color);
	}
	else if(g_sentsvinfo)
	{
		RichText stat = (STRTABLE[STR_CONMATCHSVLIST]);
		float color[4] = {0,1,0,1};
		DrawShadowedText(MAINFONT16, g_width/2, f->gheight, &stat, color);
	}
	else
	{
		RichText stat = (STRTABLE[STR_CONMATCHLISTING]);
		float color[4] = {1,1,0,1};
		DrawShadowedText(MAINFONT16, g_width/2, f->gheight, &stat, color);
	}
}

//draw player list
void Lobby_DrawPyL()
{
#define PYLFONT		MAINFONT16
	int32_t jpin = 0;
	BmpFont* f = &g_font[PYLFONT];
	int32_t viewi = 0;

	GUI* gui = &g_gui;
	ViewLayer* lobby = (ViewLayer*)gui->get("lobby");
	Image* pybg = (Image*)lobby->get("pybg");
	Texture* bgtex = &g_texture[pybg->m_tex];

	float pylpos[4];
	pylpos[0] = g_width * 3/6;
	//pylpos[1] = 100;
	pylpos[1] = 60;
	pylpos[2] = g_width - 20;
	//pylpos[3] = g_height - 180;
	pylpos[3] = g_height - 60;

	float outpos[4];
	outpos[0] = pylpos[0] - 40;
	outpos[1] = pylpos[1];
	outpos[2] = pylpos[2];
	outpos[3] = pylpos[3];

	Shader* s = &g_shader[g_curS];
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 0.6f);
	DrawImage(bgtex->texname, pylpos[0], pylpos[1], pylpos[2], pylpos[3], 0,0,1,1, g_gui.m_crop);
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

	const int32_t rows = (int32_t)(pylpos[3]-pylpos[1])/f->gheight;
	
#if 0
	AddClient(NULL, RichText("---"), 1);
	AddClient(NULL, RichText("---"), 2);
	AddClient(NULL, RichText("---"), 3);
	AddClient(NULL, RichText("---"), 4);
	AddClient(NULL, RichText("---"), 5);
	AddClient(NULL, RichText("---"), 6);
	AddClient(NULL, RichText("---"), 7);
	AddClient(NULL, RichText("---"), 8);
	AddClient(NULL, RichText("---"), 9);
	AddClient(NULL, RichText("---"), 10);
	AddClient(NULL, RichText("---"), 11);
	AddClient(NULL, RichText("---"), 12);
	AddClient(NULL, RichText("---"), 13);
	
	AssocPy(1, 1);
	AssocPy(2, 2);
	AssocPy(3, 3);
	AssocPy(4, 4);
	AssocPy(5, 5);
	AssocPy(6, 6);
	AssocPy(7, 7);
	AssocPy(8, 8);
	AssocPy(9, 9);
	AssocPy(10, 10);
	AssocPy(11, 11);
	AssocPy(12, 12);
	AssocPy(13, 13);
#endif

	//InfoMess("d","d");

	//tcin = total players' index (out of all players that can exist)
	//jcin = joined players' index (subset that is actually listed/drawn)
	for(int32_t tpin=0; tpin<PLAYERS; tpin++)
	//for(int32_t tcin=0; tcin<CLIENTS; tcin++)
	{
		Player* py = &g_player[tpin];

		if(!py->on)
			continue;

		//Client* c = &g_client[tcin];

		//if(!c->on)
		//	continue;

		//int32_t row = jcin % rows;
		//int32_t col = jcin / rows;
		int32_t row = jpin;
		int32_t col = 0;

		float color[4] = {0.8f, 0.6f, 0.2f, 0.9f};	//orange

		RichText name;

		uint32_t speedtex = 0;

		if(py->client >= 0)
		{
			Client* c = &g_client[py->client];
			
			//reported pings are kept in Client's
			float ping = c->ping;

			//if we are a client and this is the host,
			//we keep their ping in an actual NetConn.
			//or if we are the host, we keep all cl's with NetConn's.
			if(c->nc)
				ping = c->nc->ping;

			if(c->speed == SPEED_PAUSE)
				speedtex = g_texstop;
			else if(c->speed == SPEED_PLAY)
				speedtex = g_texplay;
			else if(c->speed == SPEED_FAST)
				speedtex = g_texfast;

			char pingstr[32];
			sprintf(pingstr, ":%0.0f", ping);
			RichText pingrt(pingstr);

			name = c->name + pingrt;
			
			if(c->unresp)
			{
				//red
				color[0] = 1.0f;
				color[1] = 0.1f;
				color[2] = 0.1f;
			}
			else if(c->ready)
			{
				//green
				color[0] = 0.1f;
				color[1] = 0.9f;
				color[2] = 0.1f;
			}
		}
		//must be either a client or AI bot
		else if(py->ai)
		{
			//name = py->name + RichText(":BOT");
			name = py->name;
		}
		else
			continue;

		char numcs[16];
		sprintf(numcs, "%d.", jpin+1);
		RichText numrt = RichText(UStr(numcs));
		DrawShadowedTextF(PYLFONT, pylpos[0] + col*f->gheight*10, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &numrt, color);

		RichPart rico;
		Icon* iic;

		//if(c->player >= 0)
		{
			//Player* py = &g_player[c->player];

			if(py->insttype == INST_FIRM)
			{
				rico = RichPart(RICH_ICON, ICON_FIRM);
				iic = &g_icon[ICON_FIRM];
			}
			else if(py->insttype == INST_STATE)
			{
				rico = RichPart(RICH_ICON, ICON_GOV);
				iic = &g_icon[ICON_GOV];
			}

			std::string funds = iform( py->global[RES_DOLLARS] );
			std::string util = ullform( py->util );
			std::string thru = ullform( py->gnp );

			RichText funds2 = RichText(funds.c_str());
			RichText util2 = RichText(util.c_str());
			RichText thru2 = RichText(thru.c_str());
			
			DrawShadowedTextF(PYLFONT, pylpos[0] + 0.8f*f->gheight*10 + f->gheight*1.5f, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &funds2, color);
			DrawShadowedTextF(PYLFONT, pylpos[0] + 1.6f*f->gheight*10 + f->gheight*1.5f, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &util2, color);
			DrawShadowedTextF(PYLFONT, pylpos[0] + 2.4f*f->gheight*10 + f->gheight*1.5f, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &thru2, color);
		}

		//RichText displayinst = RichText(rico);
		Texture* itex = &g_texture[iic->m_tex];
		
		DrawShadowedTextF(PYLFONT, pylpos[0] + col*f->gheight*10 + f->gheight*1.5f, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &name, color);
		//
		//DrawShadowedTextF(PYLFONT, pylpos[0] - 20, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0] - 32, pylpos[1], pylpos[2], pylpos[3], &displayinst, color);
		glUniform4f(s->slot[SSLOT_COLOR], py->color[0], py->color[1], py->color[2], py->color[3]);
		DrawImage(itex->texname, pylpos[0] - 20, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], (int32_t)(pylpos[1] + row*f->gheight) + 20, 0, 0, 1, 1, outpos);
		//
		
		if(py->client >= 0)
		{
			Client* c = &g_client[py->client];

			Texture* stex = &g_texture[speedtex];
			glUniform4f(s->slot[SSLOT_COLOR], 1,1,1,1);
			DrawImage(stex->texname, pylpos[0] - 40, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0] - 20, (int32_t)(pylpos[1] + row*f->gheight) + 20, 0, 0, 1, 1, outpos);

			if(c->downin >= 0)
			{
#if 0
				float pct = (float)c->downin / (float)g_mapfsz * 100;
				char cpct[32];
				sprintf(cpct, "%03.2f", pct);
#else
				char trf[64];
				sprintf(trf, "%d/%d", c->downin, g_mapfsz);
				RichText r = RichText(trf);
				DrawShadowedTextF(MAINFONT8, pylpos[0] + col*f->gheight*10 + f->gheight*6.0f, (int32_t)(pylpos[1] + row*f->gheight), pylpos[0], pylpos[1], pylpos[2], pylpos[3], &r, color);
#endif
			}
		}

		viewi++;
		jpin++;
	}

	f = &g_font[MAINFONT16];
	float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	RichText pysrt = STRTABLE[STR_PLAYER];
	RichText funds = RichText(RichPart(RICH_ICON, ICON_DOLLARS)) + RichText(RichPart(" ")) + STRTABLE[STR_NETWORTH];
	RichText util = RichText(RichPart(RICH_ICON, ICON_SMILEY)) + RichText(RichPart(" ")) + STRTABLE[STR_UTILPROV];
	RichText thru = RichText(RichPart(RICH_ICON, ICON_DOLLARS)) + RichText(RichPart(" ")) + STRTABLE[STR_THRUPUT];
	
	DrawShadowedTextF(MAINFONT16, pylpos[0] + 0*f->gheight*10, (int32_t)(pylpos[1] + -1*f->gheight), pylpos[0], pylpos[1]-f->gheight*2, pylpos[2], pylpos[3], &pysrt, color);
	DrawShadowedTextF(MAINFONT16, pylpos[0] + 0.8f*f->gheight*10, (int32_t)(pylpos[1] + -1*f->gheight), pylpos[0], pylpos[1]-f->gheight*2, pylpos[2], pylpos[3], &funds, color);
	DrawShadowedTextF(MAINFONT16, pylpos[0] + 1.6f*f->gheight*10, (int32_t)(pylpos[1] + -1*f->gheight), pylpos[0], pylpos[1]-f->gheight*2, pylpos[2], pylpos[3], &util, color);
	DrawShadowedTextF(MAINFONT16, pylpos[0] + 2.4f*f->gheight*10, (int32_t)(pylpos[1] + -1*f->gheight), pylpos[0], pylpos[1]-f->gheight*2, pylpos[2], pylpos[3], &thru, color);

}
