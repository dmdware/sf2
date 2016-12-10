

#ifndef WEGUI_H
#define WEGUI_H

#include "../platform.h"

#define TOP_PANEL_HEIGHT		64
#define LEFT_PANEL_WIDTH		128

#define CM_SCALES		{1000, 500, 437.5, 250, 100, 50, 25, 10, 1}
#define CM_SCALES_TXT	{"10 m", "5 m", "4.375 m", "2.5 m", "1 m", "50 cm", "25 cm", "10 cm", "1 cm"}

#define PROJ_ORTHO		1
#define PROJ_PERSP		2
extern int g_projtype;

extern float g_snapgrid;
extern ecbool g_showsky;
extern char g_lastsave[SFH_MAX_PATH+1];

void FillGUI();
void SkipLogo();
void UpdateLogo();
void Click_NewBrush();
void RedoBSideGUI();
void CloseSideView();
int GetNumFrames();
void SetNumFrames(int nframes);
ecbool SaveFileDialog(char* initdir, char* filepath);
ecbool OpenFileDialog(char* initdir, char* filepath);
void GetDoFrames();
void GetDoInclines();
void GetDoSides();
void GetDoRotations();

struct Widget;

void Resize_ViewportsFrame(Widget* w);
void Resize_TopLeftViewport(Widget* w);
void Resize_BottomLeftViewport(Widget* w);
void Resize_TopRightViewport(Widget* w);
void Resize_BottomRightViewport(Widget* w);
void Resize_FullViewport(Widget* w);

#endif
