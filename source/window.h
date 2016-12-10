



#ifndef WINDOW_H
#define WINDOW_H

#include "platform.h"
#include "math/vec2i.h"
#include "math/camera.h"
#include "algo/vector.h"
#include "algo/list.h"

#define INI_WIDTH			640
#define INI_HEIGHT			480
#define INI_BPP				32
#define DRAW_FRAME_RATE		30
#define SIM_FRAME_RATE		30
#define MIN_DISTANCE		1
#define MAX_DISTANCE		(5 * 1000 * 10)
#define FIELD_OF_VIEW		90
#define PROJ_RIGHT			600
#define MIN_ZOOM		0.05f
#define MAX_ZOOM		0.7f
#define INI_ZOOM			0.05f

extern ecbool g_quit;
extern ecbool g_background;
extern ecbool g_active;
extern ecbool g_fs;
extern double g_instantdrawfps;
extern double g_instantupdfps;
extern double g_updfrinterval;
extern double g_drawfrinterval;

extern Vec2i g_selres;
extern Vector g_ress;
extern Vector g_bpps;

#ifndef MATCHMAKER
extern Camera g_cam;
extern int g_currw;
extern int g_currh;
extern int g_width;
extern int g_height;
extern int g_bpp;
extern Vec2i g_mouse;
extern Vec2i g_mousestart;
extern ecbool g_keyintercepted;
extern ecbool g_keys[SDL_NUM_SCANCODES];
extern ecbool g_mousekeys[5];
extern float g_zoom;
extern ecbool g_mouseout;
extern ecbool g_moved;
extern ecbool g_canplace;
extern int g_bpcol;
extern int g_build;
extern Sel g_sel;
extern ecbool g_mouseoveraction;
extern int g_curst;	/* cursor state */
extern int g_kbfocus;	/* keyboad focus counter */

#endif

void CalcDrawRate();
ecbool DrawNextFrame();
void CalcUpdRate();
ecbool UpdNextFrame();
void EnumDisp();
void Resize(int width, int height);
void BreakWin(const char* title);
ecbool MakeWin(const char* title);

#endif
