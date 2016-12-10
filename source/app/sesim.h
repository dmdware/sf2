

#ifndef SESIM_H
#define SESIM_H

#include "../sim/map.h"
#include "../save/edmap.h"
#include "../save/modelholder.h"
#include "../math/physics.h"
#include "../sim/simtile.h"

#define MERGEV_D		1.5f

struct Brush;

extern Brush g_copyB;
extern ModelHolder g_copyM;

#define EDTOOL_NONE			-1
#define EDTOOL_CUT			0
#define EDTOOL_EXPLOSION	1

extern int g_edtool;

#define LEADS_NE		0
#define LEADS_SE		1
#define LEADS_SW		2
#define LEADS_NW		3
#define LEADS_DIRS		4

extern ecbool g_leads[LEADS_DIRS];

void DrawFilled(EdMap* map, std::list<ModelHolder>& modelholder);
void DrawOutlines(EdMap* map, std::list<ModelHolder>& modelholder);
void DrawSelOutlines(EdMap* map, std::list<ModelHolder>& modelholder);
void DrawDrag(EdMap* map, Matrix* mvp, int w, int h, ecbool persp);
ecbool SelectDrag(EdMap* map, Matrix* mvp, int w, int h, int x, int y, Vec3f eye, ecbool persp);
void SelectBrush(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax);
ecbool PruneB(EdMap* map, Brush* b);
ecbool PruneB2(Brush* b, Plane3f* p, float epsilon=-CLOSE_EPSILON*2);

#endif