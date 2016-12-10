










#ifndef APPGUI_H
#define APPGUI_H

#include "../../math/vec3f.h"

struct ViewLayer;
struct Widget;

extern char g_lastsave[SFH_MAX_PATH+1];

void Resize_Fullscreen(Widget* w);
void Resize_FullscreenSq(Widget* w);
void Resize_AppLogo(Widget* w);
void Resize_AppTitle(Widget* w);
void Click_NewGame();
void Click_OpenEd();
void FillGUI();
void Click_LoadMapButton();
void Click_SaveMapButton();
void Click_QSaveMapButton();
void Resize_MenuItem(Widget* w);
void Resize_CenterWin(Widget* w);
void EnumDisp();
void Resize_Logo(Widget *w);

#endif	//GGUI_H
