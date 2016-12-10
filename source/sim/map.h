











#ifndef MAP_H
#define MAP_H

#include "../math/vec2i.h"
#include "../math/vec3i.h"
#include "../math/vec2f.h"
#include "../render/tile.h"
#include "../math/fixmath.h"

extern Vec2i g_scroll;
extern Vec3i g_mouse3d;

void ScrollTo(int x, int y);

//extern unsigned char* g_hmap;
extern Tile* g_surftile;

#endif
