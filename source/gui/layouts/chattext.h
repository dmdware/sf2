












#ifndef CHATTEXT_H
#define CHATTEXT_H

#include "../../platform.h"

#define CHATFONT	MAINFONT16
#define NOTIFFONT	MAINFONT16


#define NOTIFTOP		(50)
#define NOTIFBOTTOM	(NOTIFTOP+16*4)
#define NOTIFRIGHT	(g_width*2/5)

#define CHATTOP		(NOTIFBOTTOM+10)
#define CHATBOTTOM	(g_height-180)
#define CHATRIGHT	(g_width*2/5)

//#define CHAT_LINES		(100/8)
#define NOTIF_LINES		((NOTIFBOTTOM-NOTIFTOP)/(g_font[NOTIFFONT].gheight)-1)
#define CHAT_LINES		((CHATBOTTOM-CHATTOP)/(g_font[CHATFONT].gheight)-1)

struct Widget;
struct ViewLayer;
struct RichText;

extern short g_chat;

void AddNotif(ViewLayer* cv);
void AddNotif(RichText* newl);
void AddChat(RichText* newl);
void ClearChat();

#endif
