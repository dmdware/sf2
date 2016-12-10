












#ifndef LOBBY_H
#define LOBBY_H

#include "../../../platform.h"
#include "../button.h"
#include "../image.h"
#include "../text.h"
#include "../editbox.h"
#include "../touchlistener.h"
#include "../../widget.h"
#include "../viewportw.h"
#include "../../viewlayer.h"
#include "../../../sim/selection.h"
#include "../winw.h"
#include "../../layouts/chattext.h"

//TODO move lobby to layouts

void Lobby_Regen();
void FillLobby();
void Lobby_DrawPyL();
void Lobby_DrawState();
void Click_JoinCancel();

extern uint32_t g_texstop;
extern uint32_t g_texplay;
extern uint32_t g_texfast;

#endif
