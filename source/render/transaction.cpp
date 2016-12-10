











#include "../math/matrix.h"
#include "transaction.h"
#include "../gui/font.h"
#include "../math/vec4f.h"
#include "../math/vec3i.h"
#include "../window.h"
#include "../utils.h"
#include "../sim/player.h"
#include "../math/frustum.h"
#include "../math/isomath.h"
#include "../math/hmapmath.h"
#include "../sim/map.h"
#include "heightmap.h"
#include "fogofwar.h"

std::list<Transaction> g_transx;
ecbool g_drawtransx = ecfalse;

void DrawTransx()
{
	//return;

	Py* py = &g_py[g_localP];

	Vec3f* pos;
	//Vec4f screenpos;
	int size = (int)g_font[MAINFONT8].gheight;
	float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::list<Transaction>::iterator tit = g_transx.begin();

	float interval = fmax(0.01f,(float)g_drawfrinterval);

	while(tit != g_transx.end())
	{
		tit->drawpos.z += TRANSACTION_RISE * interval;
		tit->life -= TRANSACTION_DECAY * interval;

		if(tit->life <= 0.0f || _isnan(tit->life))
		{
			tit = g_transx.erase( tit );
			continue;
		}

		tit ++;
	}

	if(!g_drawtransx)
		return;

	tit = g_transx.begin();

	Vec3i cmpos;
	Vec2i screenpos;
	int x1;
	int y1;
	Vec2i tpos;

	while(tit != g_transx.end())
	{
		pos = &tit->drawpos;

#if 0
		if(!g_frustum.pointin(pos->x, pos->y, pos->z))
		{
			tit++;
			continue;
		}
#endif

#if 0
		screenpos.x = pos->x;
		screenpos.y = pos->y;
		screenpos.z = pos->z;
#if	1 //if ortho projection
		screenpos.w = 1;
#endif

		screenpos.transform(projmodlview);
		screenpos = screenpos / screenpos.w;
		screenpos.x = (screenpos.x * 0.5f + 0.5f) * g_width;
		screenpos.y = (-screenpos.y * 0.5f + 0.5f) * g_height;

#if 0
		if(_isnan(screenpos.x))
			goto next;

		if(_isnan(screenpos.y))
			goto next;
#endif
#else
		cmpos = Vec3i((int)pos->x, (int)pos->y, (int)pos->z);
		screenpos = CartToIso(cmpos) - g_scroll;
		tpos = Vec2i(cmpos.x, cmpos.y) / TILE_SIZE;

		if(screenpos.x < 0)
			goto next;

		if(screenpos.y < 0)
			goto next;

		if(screenpos.x >= g_width)
			goto next;

		if(screenpos.y >= g_height)
			goto next;
#endif

		if(!IsTileVis(g_localP, tpos.x, tpos.y))
			goto next;

		x1 = (int)( screenpos.x - tit->halfwidth );
		y1 = (int)screenpos.y;
		color[3] = tit->life * 0.9f;

#if 0
		if(x1 < 0)
			goto next;
		if(y1 < 0)
			goto next;
		if(x1 > g_width)
			goto next;
		if(y1 > g_height)
			goto next;
#endif

		//DrawShadowedText(MAINFONT8, x1, y1, &tit->rtext, color);
		DrawBoxShadText(MAINFONT8, x1, y1, g_width, g_height, &tit->rtext, color, 0, -1);
		//DrawCenterShadText(MAINFONT8, x1, y1, &tit->rtext, color, -1);

next:
		tit ++;
	}
}

void NewTransx(Vec2i cmpos, const RichText* rtext)
{
	//return;	//for now
	Transaction t;
	t.life = 1;
	Vec3f cm3pos((float)cmpos.x, (float)cmpos.y, 0.0f);
	cm3pos.z = Bilerp(&g_hmap, cm3pos.x, cm3pos.y) * TILE_RISE;
	t.drawpos = cm3pos;
	t.rtext = *rtext;
	t.halfwidth = TextWidth(MAINFONT8, rtext) / 2.0f;
	g_transx.push_back(t);

#if 0
	std::list<Widget*>::iterator titer = g_transx.rbegin();

	Log("raw str = "<<titer->rtext.rawstr());
	
#endif
}

void FreeTransx()
{
	g_transx.clear();
}
