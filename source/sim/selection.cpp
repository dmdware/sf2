











#include "selection.h"
#include "../math/matrix.h"
#include "../window.h"
#include "../math/plane3f.h"
#include "../math/frustum.h"
#include "../math/brush.h"
#include "mvtype.h"
#include "unit.h"
#include "bltype.h"
#include "building.h"
#include "../render/shader.h"
#include "../texture.h"
#include "../math/hmapmath.h"
#include "../utils.h"
#include "../app/appmain.h"
#include "build.h"
#include "player.h"
#include "../gui/widgets/spez/cstrview.h"
#include "../gui/widgets/spez/blview.h"
#include "../gui/widgets/spez/truckmgr.h"
#include "../gui/gui.h"
#include "map.h"
#include "../render/drawsort.h"
#include "../sound/sound.h"
#include "../path/pathjob.h"
#include "../render/foliage.h"
#include "../sim/umove.h"

// Selection circle texture index
unsigned int g_circle = 0;
Selection g_sel;

void Selection::clear()
{
	mv.clear();
	bl.clear();
	roads.clear();
	powls.clear();
	crpipes.clear();
}

// Selection frustum for drag/area-selection
static Vec3f normalLeft;
static Vec3f normalTop;
static Vec3f normalRight;
static Vec3f normalBottom;
static float distLeft;
static float distTop;
static float distRight;
static float distBottom;

static Frustum g_selfrust;	//selection frustum

//is unit selected?
ecbool USel(short ui)
{
	for(std::list<int>::iterator sit=g_sel.mv.begin(); sit!=g_sel.mv.end(); sit++)
		if(*sit == ui)
			return ectrue;

	return ecfalse;
}

//is bl selected?
ecbool BSel(short bi)
{
	for(std::list<int>::iterator sit=g_sel.bl.begin(); sit!=g_sel.bl.end(); sit++)
		if(*sit == bi)
			return ectrue;

	return ecfalse;
}

void DrawMarquee()
{
	Py* py = &g_py[g_localP];

	if(!g_mousekeys[0] || g_keyintercepted || g_appmode != APPMODE_PLAY || g_build != BL_NONE)
		return;

#ifdef PLATFORM_MOBILE
	return;
#endif

#if 0
	EndS();
	UseS(SHADER_COLOR2D);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(g_sh[SHADER_COLOR2D].slot[SSLOT_HEIGHT], (float)g_height);
	glUniform4f(g_sh[SHADER_COLOR2D].slot[SSLOT_COLOR], 0, 1, 0, 0.75f);
	glEnableVertexAttribArray(g_sh[SHADER_COLOR2D].slot[SSLOT_POSITION]);
	glEnableVertexAttribArray(g_sh[SHADER_COLOR2D].slot[SSLOT_TEXCOORD0]);
#endif

	float vertices[] =
	{
		//posx, posy    texx, texy
		(float)g_mousestart.x,	(float)g_mousestart.y, 0,			0, 0,
		(float)g_mousestart.x,	(float)g_mouse.y,0,				1, 0,
		(float)g_mouse.x,			(float)g_mouse.y,0,				1, 1,

		(float)g_mouse.x,			(float)g_mousestart.y,0,			1, 1,
		(float)g_mousestart.x,	(float)g_mousestart.y,0,			0, 1
	};
	
	Shader *s = g_sh+g_curS;

#ifdef PLATFORM_GL14
	//glVertexAttribPointer(g_sh[SHADER_COLOR2D].slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
#endif
	
#ifdef PLATFORM_GLES20
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, &vertices[0]);
#endif
	
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}

#if 0
void DrawSel(Matrix* projection, Matrix* modelmat, Matrix* viewmat)
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	UseS(SHADER_COLOR3D);
	Shader* s = g_sh+g_curS;
	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m);

	Matrix mvp;
#if 0
	mvp.set(modelview.m);
	mvp.postmult(g_camproj);
#elif 0
	mvp.set(g_camproj.m);
	mvp.postmult(modelview);
#else
	mvp.set(projection->m);
	mvp.postmult(*viewmat);
	mvp.postmult(*modelmat);
#endif
	glUniformMatrix4fv(s->slot[SSLOT_MVP], 1, 0, mvp.m);

	float* color = g_py[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 0.5f);

	Py* py = &g_py[g_localP];

	glLineWidth(3);

	for(std::list<Widget*>::iterator selit = g_sel.bl.begin(); selit != g_sel.bl.end(); selit++)
	{
		const int bi = *selit;
		const Bl* b = &g_bl[bi];
		const BlType* t = &g_bltype[b->type];

		const int tminx = b->tpos.x - t->width.x/2;
		const int tminz = b->tpos.y - t->width.y/2;
		const int tmaxx = tminx + t->width.x;
		const int tmaxz = tminz + t->width.y;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminy = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxy = tmaxz*TILE_SIZE;

		const int off = TILE_SIZE/100;

		const float y1 = g_hmap.accheight(cmmaxx + off, cmminy - off) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(cmmaxx + off, cmmaxy + off) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(cmminx - off, cmmaxy + off) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(cmminx - off, cmminy - off) + TILE_SIZE/20;

		const float vertices[] =
		{
			//posx, posy posz
			(float)(cmmaxx + off), y1, (float)(cmminy - off),
			(float)(cmmaxx + off), y2, (float)(cmmaxy + off),
			(float)(cmminx - off), y3, (float)(cmmaxy + off),

			(float)(cmminx - off), y4, (float)(cmminy - off),
			(float)(cmmaxx + off), y1, (float)(cmminy - off)
		};

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_LINE_STRIP, 0, 5);
	}


	glLineWidth(1);
	EndS();

	//if(g_projtype == PROJ_PERSPECTIVE)
	UseS(SHADER_BILLBOARD);
	//else
	//	UseS(SHADER_BILLBOAR);

	s = g_sh+g_curS;

	glUniformMatrix4fv(s->slot[SSLOT_PROJECTION], 1, 0, projection->m);
	glUniformMatrix4fv(s->slot[SSLOT_MODELMAT], 1, 0, modelmat->m);
	glUniformMatrix4fv(s->slot[SSLOT_VIEWMAT], 1, 0, viewmat->m);

	color = g_py[g_localP].color;
	glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);

	//glEnableVertexAttribArray(s->slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->slot[SSLOT_NORMAL]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture[ g_circle ].texname);
	glUniform1i(s->slot[SSLOT_TEXTURE0], 0);

	for(std::list<Widget*>::iterator selit = g_sel.mv.begin(); selit != g_sel.mv.end(); selit++)
	{
		Mv* mv = &g_mv[ *selit ];
		//Entity* e = g_entity[ 0 ];
		Vec2f p = mv->drawpos;
		MvType* t = &g_mvtype[ mv->type ];

		//Vec3f p = c->pos + Vec3f(0, t->vmin.y, 0) + Vec3f(0, 1.0f, 0);

		const float r = t->size.x * 1.0f;

#if 0
		float y1 = Bilerp(&g_hmap, p.x + r, p.z - r);
		float y2 = Bilerp(&g_hmap, p.x + r, p.z + r);
		float y3 = Bilerp(&g_hmap, p.x - r, p.z + r);
		float y4 = Bilerp(&g_hmap, p.x - r, p.z - r);
#elif 1
		const float y1 = g_hmap.accheight(p.x + r, p.y - r) + TILE_SIZE/20;
		const float y2 = g_hmap.accheight(p.x + r, p.y + r) + TILE_SIZE/20;
		const float y3 = g_hmap.accheight(p.x - r, p.y + r) + TILE_SIZE/20;
		const float y4 = g_hmap.accheight(p.x - r, p.y - r) + TILE_SIZE/20;
#else
		float y1 = p.y;
		float y2 = p.y;
		float y3 = p.y;
		float y4 = p.y;
#endif

		const float vertices[] =
		{
			//posx, posy posz   texx, texy
			p.x + r, y1, p.y - r,          1, 0,
			p.x + r, y2, p.y + r,          1, 1,
			p.x - r, y3, p.y + r,          0, 1,

			p.x - r, y3, p.y + r,          0, 1,
			p.x - r, y4, p.y - r,          0, 0,
			p.x + r, y1, p.y - r,          1, 0
		};

		//glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[0]);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*5, &vertices[0]);
		//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, &vertices[3]);
		glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);
		//glVertexAttribPointer(s->slot[SSLOT_NORMAL], 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, va->normals);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	EndS();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif

void SelPtCd(Vec2i pt, Selection *sel)
{
	//for(int i=0; i<BUILDINGS; i++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	for(std::list<Dl*>::reverse_iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Dl* d = *dit;

		if(d->dtype != DEPTH_CD)
			continue;

		short tx = d->index % g_mapsz.x;
		short ty = d->index / g_mapsz.x;

		CdTile* ctile = GetCd(d->cdtype, tx, ty, d->plan);
		int i = d->index;

		CdType* ct = &g_cdtype[ d->cdtype ];
		Tile& tile = SurfTile(tx, ty, &g_hmap);
		
		int sli = ct->splist[ctile->conntype][(int)ctile->finished];
		SpList* sl = &g_splist[ sli ];
		int ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( tx * TILE_SIZE + TILE_SIZE/2, ty * TILE_SIZE + TILE_SIZE/2, tile.elev * TILE_RISE );

		Vec2i screenpos = CartToIso(cmpos) - g_scroll;
		
		//Shouldn't happen unless user mod
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		Texture* tex = &g_texture[ sp->difftexi ];
		
		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;
		
		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		std::list<Vec2i> *sellist = (std::list<Vec2i>*)(((char*)sel)+ct->seloff);
		sellist->push_back(Vec2i(tx,ty));
	}
}

/*
TODO
Go over all the code
remove commented unecessary code
remove if-0'd code
rewrite in C90
*/

int SelPtBl(Vec2i pt)
{
	int sel = -1;

	//for(int i=0; i<BUILDINGS; i++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	for(std::list<Dl*>::reverse_iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Dl* d = *dit;

		if(d->dtype != DEPTH_BL)
			continue;

		Bl* b = &g_bl[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		BlType* t = &g_bltype[ b->type ];

#if 0
		const int tminx = b->tpos.x - t->width.x/2;
		const int tminz = b->tpos.y - t->width.y/2;
		const int tmaxx = tminx + t->width.x;
		const int tmaxz = tminz + t->width.y;

		const int cmminx = tminx*TILE_SIZE;
		const int cmminy = tminz*TILE_SIZE;
		const int cmmaxx = tmaxx*TILE_SIZE;
		const int cmmaxy = tmaxz*TILE_SIZE;

		const int cmx = (cmminx+cmmaxx)/2;
		const int cmz = (cmminy+cmmaxy)/2;
#endif

#if 0
		Dl* d = b;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;
#endif
		
		Tile tile = SurfTile(b->tpos.x, b->tpos.y, &g_hmap);
		int sli = t->splist;
		if(!b->finished)
			sli = t->csplist;
		SpList* sl = &g_splist[ sli ];
		int ci = SpriteRef(sl, 0, tile.incltype, 0, 0, 0,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( b->tpos.x * TILE_SIZE + ((t->width.x % 2 == 1) ? TILE_SIZE/2 : 0),
			b->tpos.y * TILE_SIZE + ((t->width.y % 2 == 1) ? TILE_SIZE/2 : 0),
			SurfTile(b->tpos.x, b->tpos.y, &g_hmap).elev * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Texture* tex = &g_texture[ sp->difftexi ];

#if 0
		DrawImage(tex->texname,
			(float)screenpos.x + sp->offset[0],
			(float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2],
			(float)screenpos.y + sp->offset[3]);
#endif

#if 0
		if(!sp->pixels)
			InfoMess("!","!1");
		if(!sp->pixels->data)
			InfoMess("!","!2");
#endif

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

unsigned short SelPtFol(Vec2i pt)
{
	unsigned short sel = USHRT_MAX;

	//for(int i=0; i<BUILDINGS; i++)
	for(std::list<Dl*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	//for(std::list<Widget*>::iterator dit=g_subdrawq.rbegin(); dit!=g_subdrawq.rend(); dit++)
	{
		Dl* d = *dit;

		if(d->dtype != DEPTH_FOL)
			continue;

		Foliage* f = &g_fl[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		FlType* t = &g_fltype[ f->type ];
		
		int sli = t->splist;
		SpList* sl = &g_splist[ sli ];
		int ci = SpriteRef(sl, 0, 0, 0, 0, 0, 0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec2i screenmin = d->pixmin - g_scroll;
		Vec2i screenmax = d->pixmax - g_scroll;
	
		if(screenmin.x >= g_width)
			continue;

		if(screenmin.y >= g_height)
			continue;

		if(screenmax.x < 0)
			continue;

		if(screenmax.y < 0)
			continue;

		Vec3i cmpos = Vec3i( f->cmpos.x, f->cmpos.y, Bilerp(&g_hmap, (float)f->cmpos.x, (float)f->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;

		int pixx = pt.x - screenmin.x;

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - screenmin.y;

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select unit at point
int SelPtU(Vec2i pt)
{
	int sel = -1;
	
	for(std::list<Dl*>::iterator dit=g_subdrawq.begin(); dit!=g_subdrawq.end(); dit++)
	{
		Dl* d = *dit;

		if(d->dtype != DEPTH_U)
			continue;

		Mv* mv = &g_mv[d->index];
		int i = d->index;

		//if(!b->on)
		//	continue;

		MvType* t = &g_mvtype[ mv->type ];

		//assert( dir < 8 );

		//int spi = t->sprite[ dir % DIRS ][(int)(mv->frame[BODY_LOWER]) % sl->nframes ];
	
		int sli = t->splist;
		SpList* sl = &g_splist[ sli ];
		unsigned char dir = (int)( sl->nsides - ((int)(  mv->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ) % sl->nsides;
		unsigned char pitch = (int)( (sl->nsides - (int)(sl->nsides -  mv->rotation.x / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
		unsigned char roll = (int)( (sl->nsides - (int)(sl->nsides -  mv->rotation.y / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
		int ci = SpriteRef(sl, (int)(mv->frame[BODY_LOWER]) % sl->nframes, 0, pitch, dir, roll,
			0, 0);
		Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

		Vec3i cmpos = Vec3i( mv->cmpos.x, mv->cmpos.y, Bilerp(&g_hmap, (float)mv->cmpos.x, (float)mv->cmpos.y) * TILE_RISE );
		Vec2i screenpos = CartToIso(cmpos) - g_scroll;

		//Shouldn't happen unless user mod building
		if(!sp->pixels)
			continue;
		if(!sp->pixels->data)
			continue;
		
		int pixx = pt.x - (screenpos.x + (int)sp->offset[0]);

		if(pixx < 0)
			continue;

		if(pixx >= sp->pixels->sizex)
			continue;

		int pixy = pt.y - (screenpos.y + (int)sp->offset[1]);

		if(pixy < 0)
			continue;

		if(pixy >= sp->pixels->sizey)
			continue;

		int pixin = pixx + pixy * sp->pixels->sizex;

		//if transparent, not a pixel
		if(sp->pixels->data[ pixin * 4 + 3 ] < 255 / 2)
			continue;

		sel = i;
	}

	return sel;
}

//select a point
Selection SelPt()
{
	int selu = SelPtU(g_mouse);

	Selection sel;

#if 0
	InfoMess("sel one", "a");
#endif

	if(selu >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		sel.mv.push_back( selu );
	}

	int selb = SelPtBl(g_mouse);

	if(selb >= 0)
	{
		sel.bl.push_back( selb );
	}

	SelPtCd(g_mouse, &sel);
	
	unsigned short self = SelPtFol(g_mouse);

	if(self != USHRT_MAX)
	{
		sel.fl.push_back(self);
	}

	return sel;
}


Selection SelHover()
{
		int selu = SelPtU(g_mouse);

	Selection sel;

#if 0
	InfoMess("sel one", "a");
#endif

	if(selu >= 0)
	{
#if 0
		InfoMess("sel one", "b");
#endif
		sel.mv.push_back( selu );
	}

	int selb = SelPtBl(g_mouse);

	if(selb >= 0)
	{
		sel.bl.push_back( selb );
	}

	SelPtCd(g_mouse, &sel);
	
	unsigned short self = SelPtFol(g_mouse);

	if(self != USHRT_MAX)
	{
		sel.fl.push_back(self);
	}

	return sel;
}
//select area mv
std::list<int> SelArU(int minx, int miny, int maxx, int maxy)
{
	std::list<int> unitsel;

	ecbool haveowned = ecfalse;
	ecbool haveowmili = ecfalse;

	for(int i=0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;

		MvType* t = &g_mvtype[ mv->type ];

		Dl* d = mv->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmax.z);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z);

		Vec2i top = CartToIso(top3);
		Vec2i bot = CartToIso(bot3);
		Vec2i lef = CartToIso(lef3);
		Vec2i rig = CartToIso(rig3);

		if(minx > rig.x)
			continue;

		if(miny > bot.y)
			continue;

		if(maxx < lef.x)
			continue;

		if(maxy < top.y)
			continue;

		unitsel.push_back(i);

		if(mv->owner == g_localP && mv->type != MV_LABOURER)
		{
			haveowned = ectrue;

			if(t->military)
				haveowmili = ectrue;
		}
	}

	//filter mv....

	if(haveowmili)
	{
		//only owned military

		std::list<int>::iterator uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Mv* mv = &g_mv[*uit];

			if(mv->type == MV_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(mv->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			MvType* t = &g_mvtype[mv->type];

			if(!t->military)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}
	else if(haveowned)
	{
		//only owned (no labourers)

		std::list<int>::iterator uit=unitsel.begin();
		while(uit!=unitsel.end())
		{
			Mv* mv = &g_mv[*uit];

			if(mv->type == MV_LABOURER)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			if(mv->owner != g_localP)
			{
				uit = unitsel.erase(uit);
				continue;
			}

			uit++;
		}
	}

	return unitsel;
}

//select area
Selection SelAr()
{
	Py* py = &g_py[g_localP];

	int minx = imin(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int maxx = imax(g_mousestart.x, g_mouse.x) + g_scroll.x;
	int miny = imin(g_mousestart.y, g_mouse.y) + g_scroll.y;
	int maxy = imax(g_mousestart.y, g_mouse.y) + g_scroll.y;

	Selection selection;

	selection.mv = SelArU(minx, miny, maxx, maxy);

	return selection;
}

Selection DoSel()
{
	Selection sel;

	if(g_mousestart.x == g_mouse.x && g_mousestart.y == g_mouse.y)
		sel = SelPt();
#ifndef PLATFORM_MOBILE
	//no annoying area select each time touch is dragged to scroll
	else
		sel = SelAr();
#endif

	return sel;
}

void ClearSel(Selection* s)
{
	s->bl.clear();
#if 1
	s->crpipes.clear();
	s->powls.clear();
	s->roads.clear();
#else
	for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
		s->cdtiles[ctype].clear();
#endif
	s->mv.clear();
}

void AfterSel(Selection* s)
{
	ecbool haveconstr = ecfalse;
	ecbool havefini = ecfalse;
	ecbool havetruck = ecfalse;

	BlType* bt = NULL;
	CdType* ct = NULL;
	MvType* mvt = NULL;

	for(std::list<int>::iterator selit = s->bl.begin(); selit != s->bl.end(); selit++)
	{
		int bi = *selit;
		Bl* b = &g_bl[bi];
		bt = &g_bltype[b->type];

		if(!b->finished)
		{
			haveconstr = ectrue;
			break;
		}
		else
		{
			havefini = ectrue;
			break;
		}
	}

	if(!havefini && !haveconstr && s->mv.size() > 0)
	{
		std::list<int>::iterator selit = s->mv.begin();
		int ui = *selit;
		Mv* mv = &g_mv[ui];
		if(mv->type == MV_TRUCK)
			havetruck = ectrue;
		mvt = &g_mvtype[mv->type];
	}

#if 1//TODO make abstract generic conduits use
	for(std::list<Vec2i>::iterator selit = s->roads.begin(); selit != s->roads.end(); selit++)
	{
		ct = &g_cdtype[CD_ROAD];
		CdTile* cdtile = GetCd(CD_ROAD, selit->x, selit->y, ecfalse);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->bl.clear();
			haveconstr = ectrue;
			break;
		}
	}

	for(std::list<Vec2i>::iterator selit = s->powls.begin(); selit != s->powls.end(); selit++)
	{
		ct = &g_cdtype[CD_POWL];
		CdTile* cdtile = GetCd(CD_POWL, selit->x, selit->y, ecfalse);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->bl.clear();
			s->roads.clear();
			haveconstr = ectrue;
			break;
		}
	}

	for(std::list<Vec2i>::iterator selit = s->crpipes.begin(); selit != s->crpipes.end(); selit++)
	{
		ct = &g_cdtype[CD_CRPIPE];
		CdTile* cdtile = GetCd(CD_CRPIPE, selit->x, selit->y, ecfalse);

		if(!cdtile->finished)
		{
			bt = NULL;
			s->bl.clear();
			s->roads.clear();
			s->powls.clear();
			haveconstr = ectrue;
			break;
		}
	}
#else
	for(unsigned char ctype=0; ctype<CD_TYPES; ++ctype)
	{
		CdType* ct = &g_cdtype[ctype];

		//if(!ct->on)
		//	continue;

		if(!s->cdtiles[ctype].size())
			continue;

		for(std::list<Widget*>::iterator selit = s->cdtiles[ctype].begin(); selit != s->cdtiles[ctype].end(); selit++)
		{
			CdTile* cdtile = GetCd(ctype, selit->x, selit->y, ecfalse);

			if(!cdtile->finished)
			{
				bt = NULL;
				s->bl.clear();

				for(unsigned char ctype2=0; ctype2<CD_TYPES; ++ctype2)
				{
					if(ctype2 == ctype)
						continue;

					s->cdtiles[ctype2].clear();
				}

				haveconstr = ectrue;
				break;
			}
		}
	}
#endif

	Py* py = &g_py[g_localP];
	Widget *gui = (Widget*)&g_gui;

	gui->hide("bl graphs");

	if(haveconstr)
	{
		CstrView* cv = (CstrView*)gui->get("cs view");
		cv->regen(s);
		gui->show("cs view");

		if(bt)
			PlayClip(bt->sound[BLSND_CSEL]);
		//else if(ct)
		//	PlayClip(ct->sound[BLSND_CSEL]);
	}
	else if(havefini)
	{
		BlView* bv = (BlView*)gui->get("bl view");
		bv->regen(s);
		gui->show("bl view");
		PlayClip(bt->sound[BLSND_SEL]);
	}
	else if(havetruck)
	{
		TruckMgr* tm = (TruckMgr*)gui->get("truck mgr");
		tm->regen(s);
		gui->show("truck mgr");
		PlayClip(mvt->sound[USND_SEL]);

#ifdef TSDEBUG
		tracku = &g_mv[ *s->mv.begin() ];
#endif
	}

#if 0
	//35
	//2
	//15
	//39
	//12
	//457
	if(s->mv.size() > 0)
	{
		int ui = *s->mv.begin();
		g_mv[ui].freecollider();
		ecbool cl = UnitCollides(&g_mv[ui], g_mv[ui].cmpos, g_mv[ui].type);
		g_mv[ui].fillcollider();
		char msg[128];
		g_mv[ui].freecollider();
		ecbool c = UnitCollides(&g_mv[ui], g_mv[ui].cmpos, g_mv[ui].type);
		
		g_mv[ui].fillcollider();

		ecbool t = Trapped(&g_mv[ui], NULL);

		MvType* mvt = &g_mvtype[g_mv[ui].type];

		int cmminx = g_mv[ui].cmpos.x - mvt->size.x/2;
		int cmminy = g_mv[ui].cmpos.y - mvt->size.x/2;
		int cmmaxx = cmminx + mvt->size.x - 1;
		int cmmaxy = cmminy + mvt->size.x - 1;

		sprintf(msg, "sel %d u cl=%d \n target=%d \n dgoal=%d,%d \n psz=%d \n supi=%d \n c=%d \n p=(%d,%d),(%d,%d) t=%d",
			*s->mv.begin(), (int)cl,
			g_mv[*s->mv.begin()].target,
			g_mv[*s->mv.begin()].goal.x - g_mv[*s->mv.begin()].cmpos.x,
			g_mv[*s->mv.begin()].goal.y - g_mv[*s->mv.begin()].cmpos.y,
			(int)g_mv[*s->mv.begin()].path.size(),
			(int)g_mv[*s->mv.begin()].supplier,
			(int)c, cmminx, cmminy, cmmaxx, cmmaxy, (int)t);
		InfoMess(msg, msg);
	}
#endif
}
