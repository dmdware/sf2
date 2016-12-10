











#include "deposit.h"
#include "resources.h"
#include "../math/3dmath.h"
#include "../render/shader.h"
#include "../render/heightmap.h"
#include "../math/vec3f.h"
#include "../gui/icon.h"
#include "../math/hmapmath.h"
#include "../gui/gui.h"
#include "../math/vec4f.h"
#include "player.h"

Deposit g_deposit[DEPOSITS];

Deposit::Deposit()
{
	on = ecfalse;
	occupied = ecfalse;
}

void FreeDeposits()
{
	for(int i=0; i<DEPOSITS; i++)
	{
		g_deposit[i].on = ecfalse;
		g_deposit[i].occupied = ecfalse;
	}
}

void DrawDeposits(const Matrix projection, const Matrix viewmat)
{
	if(g_mapsz.x <= 0 || g_mapsz.y <= 0)
		return;

	Shader* s = g_sh+g_curS;

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, g_texture[ g_tiletexs[IN_SAND] ].texname);
	//glUniform1i(s->slot[SSLOT_SANDTEX], 0);

	Matrix mvp;
	mvp.set(projection.m);
	mvp.postmult2(viewmat);

	Py* py = &g_py[g_localP];

	for(int i=0; i<DEPOSITS; i++)
	{
		Deposit* d = &g_deposit[i];

		if(!d->on)
			continue;

		if(d->occupied)
			continue;

		Vec3f pos = d->drawpos;
#if 0
		pos.x = d->tpos.x*TILE_SIZE + TILE_SIZE/2.0f;
		pos.z = d->tpos.y*TILE_SIZE + TILE_SIZE/2.0f;
		pos.y = Bilerp(&g_hmap, pos.x, pos.z);
#endif
		//Vec4f ScreenPos(Matrix* mvp, Vec3f vec, float width, float height)
		Vec4f spos = ScreenPos(&mvp, pos, g_currw, g_currh, ectrue);

		Resource* res = &g_resource[d->restype];
		Icon* ic = &g_icon[res->icon];

		DrawImage(g_texture[ ic->tex ].texname, spos.x - 25, spos.y - 25, spos.x + 25, spos.y + 25, 0,0,1,1, g_gui.crop);
		//DrawShadowedText(MAINFONT8, spos.x, spos.y, &dep->label);
		//DrawCenterShadText(MAINFONT8, spos.x, spos.y, &dep->label);
	}
}
