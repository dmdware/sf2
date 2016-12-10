











#ifndef SELECTION_H
#define SELECTION_H

#include "../platform.h"
#include "../math/vec2i.h"
#include "../math/vec3f.h"
#include "conduit.h"

struct Selection
{
public:
	std::list<int> mv;
	std::list<int> bl;
	//std::list<Vec2i> cdtiles[CD_TYPES];
	std::list<Vec2i> roads;
	std::list<Vec2i> powls;
	std::list<Vec2i> crpipes;
	std::list<unsigned short> fl;

	void clear();
};

extern unsigned int g_circle;
extern Selection g_sel;

Selection DoSel();
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void DrawMarquee();
void ClearSel(Selection* s);
void AfterSel(Selection* s);
ecbool USel(short ui);
ecbool BSel(short bi);

#endif
