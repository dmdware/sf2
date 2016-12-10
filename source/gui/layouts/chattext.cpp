











#include "chattext.h"
#include "../gui.h"
#include "../../sim/player.h"
#include "../../net/packets.h"
#include "../font.h"
#include "../../net/sendpackets.h"
#include "../../net/client.h"
#include "../icon.h"

short g_chat = -1;

void Resize_ChatLine(Widget* w)
{
	Font* f = &g_font[w->font];
	Py* py = &g_py[g_localP];
	int i = 0;
	sscanf(w->name.c_str(), "%d", &i);
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->pos[0] = 0;
	w->pos[1] = topy + f->gheight * i;
	w->pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	w->pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatPrompt(Widget* w)
{
	Font* f = &g_font[w->font];
	Py* py = &g_py[g_localP];
	int i = CHAT_LINES+1;
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->pos[0] = 0;
	w->pos[1] = topy + f->gheight * i;
	w->pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	w->pos[3] = topy + f->gheight * (i+1);
}

void Resize_ChatBG(Widget* w)
{
	Font* f = &g_font[CHATFONT];
	Py* py = &g_py[g_localP];
	int i = 0;
	float topy = (float)CHATTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->pos[0] = 0;
	w->pos[1] = topy + f->gheight * i;
	w->pos[2] = (float)CHATRIGHT;	//f->gheight * 20;
	i = CHAT_LINES+1;
	w->pos[3] = (float)CHATBOTTOM;	//topy + f->gheight * (i+1);
}

void Resize_NotifBG(Widget* w)
{
	Font* f = &g_font[NOTIFFONT];
	Py* py = &g_py[g_localP];
	int i = 0;
	float topy = (float)NOTIFTOP;	//g_height - 200 - CHAT_LINES * f->gheight;
	w->pos[0] = 0;
	w->pos[1] = topy + f->gheight * i;
	w->pos[2] = (float)NOTIFRIGHT;	//f->gheight * 20;
	i = NOTIF_LINES+1;
	w->pos[3] = (float)NOTIFBOTTOM;	//topy + f->gheight * (i+1);
}

void Submit_Chat()
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* cv = (ViewLayer*)gui->get("chat");
	EditBox* enter = (EditBox*)cv->get("enter");
	RichText localmsg = g_name + RichText(": ") + enter->value;
	AddChat(&localmsg);
	//char msg[128];
	//sprintf(msg, "%d texlen", enter->value.texlen());
	//InfoMess("entermva len", msg);
	//InfoMess("entermva", enter->value.rawstr().c_str());
	//InfoMess("local", localmsg.rawstr().c_str());

	ChatPacket cp;
	cp.header.type = PACKET_CHAT;
	cp.client = g_localC;
	strncpy(cp.msg, enter->value.rawstr().c_str(), MAX_CHAT);
	cp.msg[MAX_CHAT] = 0;

	if(g_netmode == NETM_HOST)
		SendAll((char*)&cp, sizeof(ChatPacket), ectrue, ecfalse, NULL, 0);
	else if(g_netmode == NETM_CLIENT && g_svconn)
		SendData((char*)&cp, sizeof(ChatPacket), &g_svconn->addr, ectrue, ecfalse, g_svconn, &g_sock, 0, NULL);

	RichText blank;
	enter->changevalue(&blank);
	enter->caret = 0;
	enter->scroll[0] = 0;
	enter->scroll[1] = 0;
}

void ClearChat()
{
#if 0
	for(int i=0; i<CHAT_LINES+1; i++)
	{
		RichText blank;
		AddNotif(&blank);
	}
#endif
}

//add the chat text layout
void AddNotif(ViewLayer* cv)
{
	cv->add(new Image(cv, "", "gui/backg/white.jpg", ectrue, Resize_NotifBG, 1.0f, 1.0f, 1.0f, 0.3f));
	cv->add(new Image(cv, "", "gui/backg/white.jpg", ectrue, Resize_ChatBG, 1.0f, 1.0f, 1.0f, 0.3f));

#if 0
	for(int i=0; i<CHAT_LINES; i++)
	{
		char name[32];
		sprintf(name, "%d", i);
		cv->add(new Text(cv, name, RichText(), CHATFONT, Resize_ChatLine, ectrue, 1.0f, 1.0f, 1.0f, 1.0f));
		//TODO get rid of warnings
	}
#else
	cv->add(new TextBlock(cv, "block", RichText(), CHATFONT, Resize_NotifBG, 0.5f, 0.9f, 0.5f, 1.0f));
	cv->add(new TextBlock(cv, "block2", RichText(), CHATFONT, Resize_ChatBG, 0.2f, 0.9f, 0.9f, 1.0f));
#endif

	cv->add(new EditBox(cv, "enter", RichText(), CHATFONT, Resize_ChatPrompt, ecfalse, MAX_CHAT, NULL, Submit_Chat, -1));
}

void AddChat(RichText* newl)
{
	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* cv = (ViewLayer*)gui->get("chat");

#if 0
	for(int i=0; i<CHAT_LINES-1; i++)
	{
		char name[32];
		sprintf(name, "%d", i);

		char name2[32];
		sprintf(name2, "%d", i+1);

		Text* text = (Text*)cv->get(name);
		Text* text2 = (Text*)cv->get(name2);

		text->text = text2->text;
	}

	char name[32];
	sprintf(name, "%d", CHAT_LINES-1);
	Text* text = (Text*)cv->get(name);
	std::string datetime = DateTime();
	//text->text = RichText(UStr("[")) + RichText(UStr(datetime.c_str())) + RichText(UStr("] ")) + *newl;
	text->text = *newl;
#else
	TextBlock* block = (TextBlock*)cv->get("block2");
	//RichText old = block->text;
	/*
	necessary to wrap block in RichText() copy constructor because otherwise
	there will be pointer problems as it copies itself while erasing itself.
	TODO add check to see if itself is passed as param
	*/
	RichText newblock = RichText(block->text) + RichText("\n") + *newl;
	block->text = newblock;
	//block->text = RichText(block->text) + RichText("\n") + *newl;
	//block->text = RichText(block->text) + ParseTags(RichText("\\labour\\labour\\labour"),NULL);
	//block->text = *newl;

	//InfoMess("newl", newl->rawstr().c_str());
	//InfoMess("block", block->text.rawstr().c_str());

	//block->scroll[1] += 1.0f;

	int lines = CountLines(&block->text, CHATFONT, block->pos[0], block->pos[1], block->pos[2]-block->pos[0], block->pos[3]-block->pos[1]);
	int cutlines = imax(0, lines - CHAT_LINES);

	if(cutlines > 0)
	{
		int glyphi = GetLineStart(&block->text, CHATFONT, block->pos[0], block->pos[1], block->pos[2]-block->pos[0], block->pos[3]-block->pos[1], cutlines);
		int len = block->text.texlen();
		//block->text = block->text.substr(glyphi, len-glyphi);
		newblock = block->text.substr(glyphi, len-glyphi);
		block->text = newblock;
	}
#endif
}

//add a single chat message
void AddNotif(RichText* newl)
{
	//Log(newl->rawstr());

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;
	ViewLayer* cv = (ViewLayer*)gui->get("chat");

#if 0
	for(int i=0; i<CHAT_LINES-1; i++)
	{
		char name[32];
		sprintf(name, "%d", i);

		char name2[32];
		sprintf(name2, "%d", i+1);

		Text* text = (Text*)cv->get(name);
		Text* text2 = (Text*)cv->get(name2);

		text->text = text2->text;
	}

	char name[32];
	sprintf(name, "%d", CHAT_LINES-1);
	Text* text = (Text*)cv->get(name);
	std::string datetime = DateTime();
	//text->text = RichText(UStr("[")) + RichText(UStr(datetime.c_str())) + RichText(UStr("] ")) + *newl;
	text->text = *newl;
#else
	TextBlock* block = (TextBlock*)cv->get("block");
	//RichText old = block->text;
	/*
	necessary to wrap block in RichText() copy constructor because otherwise
	there will be pointer problems as it copies itself while erasing itself.
	TODO add check to see if itself is passed as param
	*/
	RichText newblock = RichText(block->text) + RichText("\n") + *newl;
	block->text = newblock;
	//block->text = RichText(block->text) + RichText("\n") + *newl;
	//block->text = RichText(block->text) + ParseTags(RichText("\\labour\\labour\\labour"),NULL);
	//block->text = *newl;

	//InfoMess("newl", newl->rawstr().c_str());
	//InfoMess("block", block->text.rawstr().c_str());

	//block->scroll[1] += 1.0f;

	int lines = CountLines(&block->text, NOTIFFONT, block->pos[0], block->pos[1], block->pos[2]-block->pos[0], block->pos[3]-block->pos[1]);
	int cutlines = imax(0, lines - NOTIF_LINES);

	if(cutlines > 0)
	{
		int glyphi = GetLineStart(&block->text, NOTIFFONT, block->pos[0], block->pos[1], block->pos[2]-block->pos[0], block->pos[3]-block->pos[1], cutlines);
		int len = block->text.texlen();
		//block->text = block->text.substr(glyphi, len-glyphi);
		newblock = block->text.substr(glyphi, len-glyphi);
		block->text = newblock;
	}
#endif
}
