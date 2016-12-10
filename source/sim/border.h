

#ifndef BORDER_H
#define BORDER_H

#include "../math/vec2i.h"

//territory per tile
extern signed char* g_border;

struct Mv;
struct Bl;

void DrawBords(Vec2i tmin, Vec2i tmax);
void MarkTerr(Mv* mv);
void MarkTerr(Bl* b);

#endif