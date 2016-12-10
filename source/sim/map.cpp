











#include "../window.h"
#include "map.h"
#include "../utils.h"
#include "../math/vec3i.h"

Vec2i g_scroll(0,400);
Vec3i g_mouse3d;
//unsigned char* g_hmap = NULL;
Tile* g_surftile = NULL;

void ScrollTo(int x, int y)
{
	g_scroll.x = x;
	g_scroll.y = y;
}
