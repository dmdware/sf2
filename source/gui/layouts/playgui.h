










#ifndef PLAYGUI_H
#define PLAYGUI_H

#include "../../sim/selection.h"

struct Widget;

void FillPlay();
//void BuildMenu_OpenPage1();
//void BuildMenu_OpenPage2();
//void BuildMenu_OpenPage3();
void Click_RightMenu_BackToOpener();
void UpdResTicker();
void ShowMessage(const RichText& msg);
void Resize_Window(Widget* w);
void Resize_BlPreview(Widget* w);
void Click_Pause();
void Click_NextBuildButton(int nextpage);

extern Vec3i g_vdrag[2];

#endif
