










#ifndef DRAW2D_H
#define DRAW2D_H

#include "../platform.h"

extern float g_basedepth;

//#define SPEDDEPTH	(28 + 256 * 200 + 256 * 256 * 7)
#define SPEDDEPTH	g_basedepth

#ifdef PLATFORM_MOBILE
#define ISOTOP	//topological depth sort? (instead of per-pixel using depth maps?)
#endif

//#define ISOTOP	//440 fps
//304 fps perpix

void DrawImage(unsigned int tex, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom, float *crop);
void DrawDeep(unsigned int difftex, unsigned int depthtex, unsigned int teamtex, int basedepth, float baseelev,
			  unsigned int renderdepthtex, unsigned int renderfb,
			  float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawDeep2(unsigned int difftex, unsigned int depthtex, unsigned int teamtex, unsigned int elevtex, 
			  unsigned int renderdepthtex, unsigned int renderfb,
			  int basedepth,  float baseelev,
			  float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);
void DrawSquare(float r, float g, float b, float a, float left, float top, float right, float bottom, float *crop);
void DrawLine(float r, float g, float b, float a, float x1, float y1, float x2, float y2, float *crop);
void DrawDeepColor(float r, float g, float b, float a, float *v, int nv, GLenum mode);
void DrawDepth(unsigned int difftex, unsigned int depthtex,  unsigned int renderdepthtex, unsigned int renderfb, int basedepth, float left, float top, float right, float bottom, float texleft, float textop, float texright, float texbottom);

void DrawSphericalBlend(unsigned int difftex, unsigned int depthtex, unsigned int renderdepthtex, unsigned int renderfb, float basedepth,
						float cx, float cy,
						float pixradius, float angle,
						float texleft, float textop, float texright, float texbottom);
#endif
