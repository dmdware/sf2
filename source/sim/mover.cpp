











#include "unit.h"
#include "../render/shader.h"
#include "mvtype.h"
#include "../texture.h"
#include "../utils.h"
#include "player.h"
#include "../math/hmapmath.h"
#include "umove.h"
#include "simdef.h"
#include "simflow.h"
#include "labourer.h"
#include "../debug.h"
#include "../math/frustum.h"
#include "building.h"
#include "labourer.h"
#include "truck.h"
#include "../econ/demand.h"
#include "../math/vec4f.h"
#include "../path/pathjob.h"
#include "../render/anim.h"
#include "map.h"
#include "../render/drawqueue.h"
#include "../render/drawsort.h"
#include "../path/collidertile.h"
#include "../path/pathjob.h"
#include "../render/fogofwar.h"
#include "../gui/layouts/chattext.h"

#ifdef RANDOM8DEBUG
int thatunit = -1;
#endif

Mv g_mv[MOVERS];


int CntMv(int mvtype)
{
	int c = 0;

	for(int i=0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;
		
		if(mv->type != mvtype)
			continue;

		c++;
	}

	return c;
}

int CntMv(int mvtype, int owner)
{
	int c = 0;

	for(int i=0; i<MOVERS; i++)
	{
		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;
		
		if(mv->type != mvtype)
			continue;

		if(mv->owner != owner)
			continue;

		c++;
	}

	return c;
}

Mv::Mv()
{
	on = ecfalse;
	threadwait = ecfalse;
	depth = NULL;
	//filled = ecfalse;
}

Mv::~Mv()
{
	destroy();
}

void Mv::destroy()
{
	//corpc fix
	if(on)
		ResetMode(this);	//make sure to disembark, reset truck driver, etc.

	//corpc fix TODO
	//if(g_collider.size && on)
	if(on && g_pathnode)
		freecollider();

	on = ecfalse;
	threadwait = ecfalse;

	if(home >= 0)
		Evict(this, ectrue);

	if(mode == UMODE_GOSUP
		//|| mode == GOINGTOREFUEL
			//|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
				)
	{
		if(supplier >= 0)
		{
			//Bl* b = &g_bl[supplier];
			//b->transporter[cargotype] = -1;
		}
	}

	if(type == MV_TRUCK)
	{
		ecbool targbl = ecfalse;
		ecbool targcd = ecfalse;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_BL )
			targbl = ectrue;

		if( (mode == UMODE_GOSUP || mode == UMODE_ATSUP || mode == UMODE_GOREFUEL || mode == UMODE_REFUELING) && targtype == TARG_CD )
			targbl = ectrue;

		if(mode == UMODE_GODEMB)
			targbl = ectrue;

		if(mode == UMODE_ATDEMB)
			targbl = ectrue;

		if(mode == UMODE_GODEMCD)
			targcd = ectrue;

		if(mode == UMODE_ATDEMCD)
			targcd = ectrue;

		if(targtype == TARG_BL)
			targbl = ectrue;

		if(targtype == TARG_CD)
			targcd = ectrue;

		if( targbl )
		{
			if(target >= 0)
			{
				Bl* b = &g_bl[target];
				b->transporter[cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(target >= 0 && target2 >= 0 && cdtype >= 0)
			{
				CdTile* ctile = GetCd(cdtype, target, target2, ecfalse);
				ctile->transporter[cargotype] = -1;
			}
		}
	}

	cyclehist.clear();

	std::list<Dl>::iterator qit=g_drawlist.begin();
	while(qit!=g_drawlist.end())
	{
		if(&*qit != depth)
		{
			qit++;
			continue;
		}

		qit = g_drawlist.erase(qit);
	}
	depth = NULL;
}

/*
How is this diff from ResetPath?
Find out and fix this.
*/
void Mv::resetpath()
{
	path.clear();
	tpath.clear();
	subgoal = cmpos;
	goal = cmpos;
	pathblocked = ecfalse;
}

#if 0
void DrawUnits()
{
	Shader* s = g_sh+g_curS;

	for(int i=0; i<MOVERS; i++)
	{
		StartTimer(TIMER_DRAWUMAT);

		Mv* mv = &g_mv[i];

		if(!mv->on)
			continue;

		if(mv->hidden())
			continue;

		MvType* t = &g_mvtype[mv->type];
#if 0
		Vec3f vmin(mv->drawpos.x - t->size.x/2, mv->drawpos.y, mv->drawpos.y - t->size.x/2);
		Vec3f vmax(mv->drawpos.x + t->size.x/2, mv->drawpos.y + t->size.y, mv->drawpos.y + t->size.x/2);

		if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
			continue;

		Py* py = &g_py[mv->owner];
		float* color = py->color;
		glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

		Model* m = &g_model[t->model];
#endif
		StopTimer(TIMER_DRAWUMAT);

		//m->draw(mv->frame[BODY_LOWER], mv->drawpos, mv->rotation.z);
		Sprite* sp = &g_sprite[t->sprite[0][0]];
		Texture* difftex = &g_texture[sp->difftexi];
		Texture* depthtex = &g_texture[sp->depthtexi];

		Vec3i cm3pos;
		cm3pos.x = mv->cmpos.x;
		cm3pos.y = mv->cmpos.y;
		cm3pos.z = (int)( g_hmap.accheight(mv->cmpos.x, mv->cmpos.y) * TILE_RISE );
		Vec2i isopos = CartToIso(cm3pos);
		Vec2i screenpos = isopos - g_scroll;

#if 0
		DrawImage(difftex->texname,
			(float)screenpos.x + sp->offset[0], (float)screenpos.y + sp->offset[1],
			(float)screenpos.x + sp->offset[2], (float)screenpos.y + sp->offset[3], 
			0,0,1,1, g_gui.crop);
#else
		DrawDeep(difftex->texname, depthtex->texname, 0,
			(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
			(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
			sp->crop[0], sp->crop[1],
			sp->crop[2], sp->crop[3]);
#endif
	}
}
#endif


void CalcRot(Mv* mv)
{
		Vec3f center = Vec3f(mv->cmpos.x, mv->cmpos.y, g_hmap.accheight(mv->cmpos.x, mv->cmpos.y) * TILE_RISE);
			
#if 0
		MvType* mvt = &g_mvtype[mv->type];

		Vec3f sidevec = center + Vec3f(mvt->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, mvt->size.x/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		//sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		
		//sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		
		//sidevec = RotateAround(sidevec, center, -( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, -( mv->rotation.z ), 0, 0, 1);

		mv->rotation.x = atan2(frontvec.x, frontvec.z) - M_PI/2;	//pitch
		
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		Vec3f viewvec = Normalize(frontvec - center);
		
		sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		sidevec = RotateAround(sidevec, center, ( mv->rotation.x ), viewvec.x, viewvec.y, viewvec.z);

		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		
		sidevec = RotateAround(sidevec, center, -( mv->rotation.x ), viewvec.x, viewvec.y, viewvec.z);
		sidevec = RotateAround(sidevec, center, -( mv->rotation.z ), 0, 0, 1);

		mv->rotation.y = atan2(sidevec.y, sidevec.z) - M_PI/2; //roll
		//mv->rotation.y = 0;
		//mv->rotation.x = 0;
		//mv->rotation.x = mv->rotation.x + 40;

		//if(mv->rotation.x > 360)
		//	mv->rotation.x = 0;
#elif 1
		
		MvType* mvt = &g_mvtype[mv->type];

		Vec3f sidevec = center + Vec3f(-mvt->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, mvt->size.x/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		
		sidevec = RotateAround(sidevec, center, -( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, -( mv->rotation.z ), 0, 0, 1);

		mv->rotation.x = atan2(frontvec.x, frontvec.z) - M_PI/2;	//pitch
#if 0
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		Vec3f viewvec = Normalize(frontvec - center);
		
		sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		sidevec = RotateAround(sidevec, center, ( mv->rotation.x ), viewvec.x, viewvec.y, viewvec.z);

		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		
		sidevec = RotateAround(sidevec, center, -( mv->rotation.x ), viewvec.x, viewvec.y, viewvec.z);
		sidevec = RotateAround(sidevec, center, -( mv->rotation.z ), 0, 0, 1);
#endif

		mv->rotation.y = atan2(sidevec.y, sidevec.z) - M_PI/2; //roll
		//mv->rotation.y = 0;
		//mv->rotation.x = 0;
		//mv->rotation.x = mv->rotation.x + 40;

		//if(mv->rotation.x > 360)
		//	mv->rotation.x = 0;

#elif 0
		Vec3f sidevec = center + Vec3f(TILE_SIZE/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, TILE_SIZE/2, 0);
		
		//TODO get rid of deg-to-rad rad-to-deg conversions and use rad for all
		sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y);
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y);
		
		sidevec = sidevec - center;
		frontvec = frontvec - center;
		
		//sidevec.z *= 2;
		//frontvec.z *= 2;

#if 1
		
		sidevec = Normalize(sidevec);
		frontvec = Normalize(frontvec);

		//yaw
		Vec3f d = frontvec;
		//mv->rotation.z = atan2(d.x, d.y);

		//pitch
		d = frontvec;
		float lateral = MAG_VEC3F(Vec3f(d.x, d.y, 0));
		mv->rotation.x = ( atan2(d.z, lateral) ) * 500;

		//roll
		d = sidevec;
		lateral = MAG_VEC3F(Vec3f(d.x, d.y, 0));
		mv->rotation.y = ( atan2(d.z, lateral) ) * 500;
#endif
#if 0
		if(sidevec.z < 0)
			mv->rotation.y = 360 - 360/SDIRS;
		else if(sidevec.z > 0)
			mv->rotation.y = 360/SDIRS;
		else
			mv->rotation.y = 0;
		
		if(frontvec.z < 0)
			mv->rotation.x = 360 - 360/SDIRS;
		else if(frontvec.z > 0)
			mv->rotation.x = 360/SDIRS;
		else
			mv->rotation.x = 0;
#endif
#elif 1

		MvType* mvt = &g_mvtype[mv->type];

		// http://www.jldoty.com/code/DirectX/YPRfromUF/YPRfromUF.html
		
#if 0
		Vec3f sidevec = center + Vec3f(-mvt->size.x/2, 0, 0);
		Vec3f side2vec = center + Vec3f(mvt->size.x/2, 0, 0);
		Vec3f frontvec = center + Vec3f(0, mvt->size.x/2, 0);
		Vec3f backvec = center + Vec3f(0, -mvt->size.x/2, 0);
#endif
		Vec3f sidevec = center + Vec3f(-1, 0, 0);
		Vec3f side2vec = center + Vec3f(1, 0, 0);
		Vec3f frontvec = center + Vec3f(0, 1, 0);
		Vec3f backvec = center + Vec3f(0, -1, 0);
		
		sidevec = RotateAround(sidevec, center, ( mv->rotation.z ), 0, 0, 1);
		frontvec = RotateAround(frontvec, center, ( mv->rotation.z ), 0, 0, 1);
		side2vec = RotateAround(side2vec, center, ( mv->rotation.z ), 0, 0, 1);
		backvec = RotateAround(backvec, center, ( mv->rotation.z ), 0, 0, 1);
		
		sidevec.z = g_hmap.accheight(sidevec.x, sidevec.y) * TILE_RISE;
		frontvec.z = g_hmap.accheight(frontvec.x, frontvec.y) * TILE_RISE;
		side2vec.z = g_hmap.accheight(side2vec.x, side2vec.y) * TILE_RISE;
		backvec.z = g_hmap.accheight(backvec.x, backvec.y) * TILE_RISE;
		
		frontvec = ( frontvec - center );
		sidevec = ( sidevec - center );
		backvec = ( backvec - center );
		side2vec = ( side2vec - center );
		
		//frontvec = ( frontvec - backvec );
		//sidevec = ( sidevec - side2vec );
		
		frontvec.z = ( frontvec.z * 2 );
		sidevec.z = ( sidevec.z * 2 );
		
		frontvec = Normalize( frontvec );
		sidevec = Normalize( sidevec );

		mv->rotation.x = asin( -frontvec.z );
		//mv->rotation.z = atanf( frontvec.x / frontvec.y );
		//mv->rotation.z = atanf( forward.x / forward.y );

		//up0 = with yaw and pitch but no roll
		Vec3f right0 = Cross(Vec3f(0,0,1), frontvec);
		//Vec3f right0 = Cross(frontvec, Vec3f(0,0,1));
		//Vec3f right0 = sidevec;
		Vec3f up0 = Cross(frontvec, right0);
		Vec3f upr = Cross(frontvec, sidevec);
		//mv->rotation.y = acosf( Dot(up0, upr) );

		//The acos() function computes the principal value of the arc cosine of __x. The returned value is in the range [0, pi] radians. A domain error occurs for arguments not in the range [-1, +1].
		float iniroty = acosf( Dot(up0, upr) );

		float up0comp;
		float uprcomp;
		float right0comp;

		//x or z greatest
		if(sidevec.x > sidevec.y)
		{
			//z greatest
			if(sidevec.z > sidevec.x)
			{
				up0comp = up0.z;
				uprcomp = upr.z;
				right0comp = right0.z;

				//sin(Oz) = ( cos(Oz) y0z - yrz ) / x0z
				//sin(Oz) x0z = ( cos(Oz) y0z - yrz )
				//sin(Oz) x0z - cos(Oz) y0z = - yrz
				//
				//Oz = asin ( ( cos(Oz) y0z - yrz ) / x0z ) 
				//
				// op/hyp = ( adj/hyp y0z - yrz ) / x0z
				// hyp=1
				// op = ( adj y0z - yrz ) / x0z
				// 
			}
			//x greatest
			else
			{
				up0comp = up0.x;
				uprcomp = upr.x;
				right0comp = right0.x;
			}
		}
		//z greatest
		else if(sidevec.z > sidevec.y)
		{
			up0comp = up0.z;
			uprcomp = upr.z;
			right0comp = right0.z;
		}
		//y greatest
		else
		{
			up0comp = up0.y;
			uprcomp = upr.y;
			right0comp = right0.y;
		}

		//The asin() function computes the principal value of the arc sine of __x. The returned value is in the range [-pi/2, pi/2] radians. A domain error occurs for arguments not in the range [-1, +1].
		//float roty1 = asinf( ( cosf(iniroty) * up0comp - uprcomp ) / right0comp ) + M_PI*2.0f;
		//float roty2 = asinf( ( cosf(-iniroty) * up0comp - uprcomp ) / right0comp ) + M_PI*2.0f;
		float roty1 = asinf( ( cosf(iniroty) * up0comp - uprcomp ) / right0comp );
		float roty2 = asinf( ( cosf(-iniroty) * up0comp - uprcomp ) / right0comp );
		
		float d1 = fmin( fabs(roty1 - iniroty), fmin( fabs(2*M_PI + roty1 - iniroty), fabs(-2*M_PI + roty1 - iniroty)) );
		float d2 = fmin( fabs(roty2 - iniroty), fmin( fabs(2*M_PI + roty2 - iniroty), fabs(-2*M_PI + roty2 - iniroty)) );
		//float d1 = fabs(roty1 - iniroty);
		//float d2 = fabs(roty2 - iniroty);

		if(d1 < d2)
			mv->rotation.y = iniroty;
		else
			mv->rotation.y = -iniroty;
		
#if 0
		if(sidevec.z >= 0 && iniroty >= 0)
			mv->rotation.y = iniroty;
		else if(sidevec.z < 0 && iniroty < 0)
			mv->rotation.y = iniroty;
		else
			mv->rotation.y = -iniroty;
#endif

#if 0
		if(frontvec.z < 0)
			mv->rotation.x = -mv->rotation.x;
#endif
#if 0
		if(sidevec.z < 0)
			mv->rotation.y = -mv->rotation.y;
#endif

#if 0
		if( fsign(sidevec.z) == fsign(mv->rotation.y) )
			mv->rotation.y = -mv->rotation.y;	
#endif
#if 0
		if( fsign(frontvec.z) != fsign(mv->rotation.x) )
			mv->rotation.x = -mv->rotation.x;
#endif

		/* 
		The atan2() function computes the principal value of the arc tangent of __y / __x, using the signs of 
		both arguments to determine the quadrant of the return value. The returned value is in the range [-pi, +pi] radians.
		*/

#if 0
		int deg = (int)( 720 + RADTODEG( mv->rotation.z ) ) % 360;
		//if(mv->rotation.z >= M_PI || mv->rotation.z < 0)
		//if( ! ( (mv->rotation.z >= M_PI/2 && mv->rotation.z <= M_PI) ||
		//	(mv->rotation.z <= -M_PI/2 && mv->rotation.z >= -M_PI) ) )
		///if( mv->rotation.z < 0 )
		//if( deg >= 45 && deg <= (180 + 45)%360 )
		if( deg >= 90+45 && deg <= (180 + 45+90)%360 &&
			!(deg >= 90+45+90 && deg <= (90+45+90+90)%360))
		{
			mv->rotation.x = -mv->rotation.x;
			mv->rotation.y = -mv->rotation.y;
		}
#endif
		
		//mv->rotation.y = iniroty;
		//mv->rotation.y *= 500;
		//mv->rotation.x *= 500;
#endif
}


/*
TODO
Mv selection based on drawpos x,y.
Check if sprite clip rect + drawpos x,y
is in g_mouse + g_scroll.
Same for bl, cd.
*/

void DrawUnit(Mv* mv, float rendz, unsigned int renderdepthtex, unsigned int renderfb)
{
	StartTimer(TIMER_DRAWUNITS);

	short tx = mv->cmpos.x / TILE_SIZE;
	short ty = mv->cmpos.y / TILE_SIZE;

	Shader* s = g_sh+g_curS;
	
	glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);

#if 0
	if(IsTileVis(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
	else if(Explored(g_localP, tx, ty))
		glUniform4f(s->slot[SSLOT_COLOR], 0.5f, 0.5f, 0.5f, 1.0f);
	else
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}
#endif

	if(!IsTileVis(g_localP, tx, ty))
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(mv->hidden())
	{
		StopTimer(TIMER_DRAWUNITS);
		return;
	}

	if(USel(u - g_mv))
	{
		EndS();
#ifdef ISOTOP
		UseS(SHADER_COLOR2D);
#else
		UseS(SHADER_DEEPCOLOR);
#endif
		s = g_sh+g_curS;
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

		MvType* t = &g_mvtype[ mv->type ];

		Dl* d = mv->depth;

		Vec3i top3 = Vec3i(d->cmmin.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i bot3 = Vec3i(d->cmmax.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i lef3 = Vec3i(d->cmmin.x, d->cmmax.y, d->cmmin.z + TILE_SIZE/200);
		Vec3i rig3 = Vec3i(d->cmmax.x, d->cmmin.y, d->cmmin.z + TILE_SIZE/200);

		Vec2i top = CartToIso(top3) - g_scroll;
		Vec2i bot = CartToIso(bot3) - g_scroll;
		Vec2i lef = CartToIso(lef3) - g_scroll;
		Vec2i rig = CartToIso(rig3) - g_scroll;

		//glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 0.5f);
		glUniform4f(s->slot[SSLOT_COLOR], 0.0f, 1.0f, 0.0f, 1.0f);

		int topd, botd, lefd, rigd;
		
#if 1
		CartToDepth(top3, &topd);
		CartToDepth(bot3, &botd);
		CartToDepth(lef3, &lefd);
		CartToDepth(rig3, &rigd);

#if 0
		top = Vec2i(50, 50);
		bot = Vec2i(50, 100);
		lef = Vec2i(0, 75);
		rig = Vec2i(100, 75);
#endif

#if 0
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)topd,
			(float)rig.x, (float)rig.y, (float)rigd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)bot.x, (float)bot.y, (float)botd,
			(float)lef.x, (float)lef.y, (float)lefd,
			(float)top.x, (float)top.y, (float)topd
		};
#endif
#else
		float vertices[] =
		{
			//posx, posy
			(float)top.x, (float)top.y, (float)d->rendz,
			(float)rig.x, (float)rig.y, (float)d->rendz,
			(float)bot.x, (float)bot.y, (float)d->rendz,
			(float)lef.x, (float)lef.y, (float)d->rendz,
			(float)top.x, (float)top.y, (float)d->rendz
		};
#endif

#if 0
		//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &vertices[0]);
		//glVertexPointer(3, GL_FLOAT, 0, &vertices[0]);

		glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &vertices[0]);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*5, &vertices[3]);

		glDrawArrays(GL_LINE_LOOP, 0, 4);
#else
		//DrawDeepColor(0, 1, 0, 1, vertices, 5, GL_LINE_STRIP);
		DrawDeepColor(0, 1, 0, 1, vertices, 6, GL_TRIANGLES);
#endif

		EndS();

#ifdef ISOTOP
		UseS(SHADER_ORTHO);
#else
		UseS(SHADER_DEEPTEAMELEV);
#endif
		s = g_sh+g_curS;
		glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
		glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
		glUniform1f(s->slot[SSLOT_MIND], (float)MIN_DISTANCE);
		glUniform1f(s->slot[SSLOT_MAXD], (float)MAX_DISTANCE);
		glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);
		glUniform1f(s->slot[SSLOT_BASEELEV], 0);
	}

	MvType* t = &g_mvtype[mv->type];
#if 0
	Vec3f vmin(mv->drawpos.x - t->size.x/2, mv->drawpos.y, mv->drawpos.y - t->size.x/2);
	Vec3f vmax(mv->drawpos.x + t->size.x/2, mv->drawpos.y + t->size.y, mv->drawpos.y + t->size.x/2);

	if(!g_frustum.boxin2(vmin.x, vmin.y, vmin.z, vmax.x, vmax.y, vmax.z))
		continue;

	Py* py = &g_py[mv->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], color[3]);

	Model* m = &g_model[t->model];
#endif
	
	Py* py = &g_py[mv->owner];
	float* color = py->color;
	glUniform4f(s->slot[SSLOT_OWNCOLOR], color[0], color[1], color[2], 1);

	//m->draw(mv->frame[BODY_LOWER], mv->drawpos, mv->rotation.z);

	//assert( dir < 8 );

	//int spi = t->sprite[ dir % DIRS ][(int)(mv->frame[BODY_LOWER]) % sl->nframes ];
	
	int sli = t->splist;
	SpList* sl = &g_splist[ sli ];
	unsigned char dir = (int)( sl->nsides - ((int)(  mv->rotation.z / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) ) % sl->nsides;
	unsigned char pitch = (int)( (sl->nsides - (int)(sl->nsides -  mv->rotation.x / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	unsigned char roll = (int)( (sl->nsides - (int)(sl->nsides -  mv->rotation.y / (2.0f*M_PI) * sl->nsides + 4 * sl->nsides + 1.0f/3.0f ) % sl->nsides) % sl->nsides);
	int ci = SpriteRef(sl, (int)(mv->frame[BODY_LOWER]) % sl->nframes, 0, pitch, dir, roll, 0, 0);
	Sprite* sp = &g_sprite[ sl->sprites[ ci ] ];

	//char m[123];
	//sprintf(m,"draw u ci %d", ci);
	//InfoMess(m,m);

	//assert( (int)mv->frame[BODY_LOWER] < t->nframes );
	
	Texture* difftex = &g_texture[sp->difftexi];
	Texture* depthtex = &g_texture[sp->depthtexi];
	Texture* teamtex = &g_texture[sp->teamtexi];
	Texture* elevtex = &g_texture[sp->elevtexi];

	Vec3i cm3pos;
	cm3pos.x = mv->cmpos.x;
	cm3pos.y = mv->cmpos.y;
	cm3pos.z = (int)( g_hmap.accheight2(mv->cmpos.x, mv->cmpos.y) );
	Vec2i isopos = CartToIso(cm3pos);
	Vec2i screenpos = isopos - g_scroll;
	glUniform1f(s->slot[SSLOT_BASEELEV], cm3pos.z);

#if 0
	DrawImage(difftex->texname,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->cropoff[0]/(float)difftex->width, sp->cropoff[1]/(float)difftex->height, 
		sp->cropoff[2]/(float)difftex->width, sp->cropoff[3]/(float)difftex->height,
		g_gui.crop);
#elif 0
	DrawDeep(difftex->texname, depthtex->texname, rendz,
		screenpos.x + sp->offset[0], screenpos.y + sp->offset[1],
		screenpos.x + sp->offset[2], screenpos.y + sp->offset[3],
		0, 0, 1, 1);
#else
	DrawDeep2(difftex->texname, depthtex->texname, teamtex->texname, elevtex->texname,
		renderdepthtex, renderfb,
		rendz, cm3pos.z,
		(float)screenpos.x + sp->cropoff[0], (float)screenpos.y + sp->cropoff[1],
		(float)screenpos.x + sp->cropoff[2], (float)screenpos.y + sp->cropoff[3],
		sp->crop[0], sp->crop[1],
		sp->crop[2], sp->crop[3]);
#endif

	StopTimer(TIMER_DRAWUNITS);
}

int NewUnit()
{
	for(int i=0; i<MOVERS; i++)
		if(!g_mv[i].on)
			return i;

	return -1;
}

// starting belongings for labourer
void StartBel(Mv* mv)
{
	Zero(mv->belongings);
	mv->car = -1;
	mv->home = -1;

	if(mv->type == MV_LABOURER)
	{
		//if(mv->owner >= 0)
		{
			//mv->belongings[ RES_DOLLARS ] = 100;
			//mv->belongings[ RES_DOLLARS ] = CYCLE_FRAMES * LABOURER_FOODCONSUM * 30;
			mv->belongings[ RES_DOLLARS ] = CYCLE_FRAMES/FOOD_CONSUM_DELAY_FRAMES * LABOURER_FOODCONSUM * 10;
		}

		mv->belongings[ RES_RETFOOD ] = STARTING_RETFOOD;
		mv->belongings[ RES_LABOUR ] = STARTING_LABOUR;
	}
	else if(mv->type == MV_TRUCK)
	{
		mv->belongings[ RES_RETFUEL ] = STARTING_FUEL;
	}
}

ecbool PlaceUnit(int type, Vec2i cmpos, int owner, int *reti)
{
	int i = NewUnit();

	if(i < 0)
		return ecfalse;

	if(reti)
		*reti = i;

#if 0
	ecbool on;
	int type;
	int stateowner;
	int corpowner;
	int unitowner;

	/*
	The draw (floating-point) position vectory is used for drawing.
	*/
	Vec3f drawpos;

	/*
	The real position is stored in integers.
	*/
	Vec3i cmpos;
	Vec3f facing;
	Vec2f rotation;

	deque<Vec2i> path;
	Vec2i goal;

	int step;
	int target;
	int target2;
	ecbool targetu;
	ecbool underorder;
	int fuelstation;
	int belongings[RESOURCES];
	int hp;
	ecbool passive;
	Vec2i prevpos;
	int taskframe;
	ecbool pathblocked;
	int jobframes;
	int supplier;
	int reqamt;
	int targtype;
	int home;
	int car;
	//std::vector<TransportJob> bids;

	float frame[2];
#endif

	Mv* mv = &g_mv[i];
	MvType* t = &g_mvtype[type];

	cmpos.x = imax(0, cmpos.x);
	cmpos.y = imax(0, cmpos.y);
	cmpos.x = imin(g_mapsz.x*TILE_SIZE-1, cmpos.x);
	cmpos.y = imin(g_mapsz.y*TILE_SIZE-1, cmpos.y);

	mv->on = ectrue;
	mv->type = type;
	mv->cmpos = cmpos;
	//mv->drawpos = Vec3f(cmpos.x, cmpos.y, Bilerp(&g_hmap, (float)cmpos.x, (float)cmpos.y)*TILE_RISE);
	mv->owner = owner;
	mv->path.clear();
	mv->goal = cmpos;
	mv->target = -1;
	mv->target2 = -1;
	mv->targetu = ecfalse;
	mv->underorder = ecfalse;
	mv->fuelstation = -1;
	mv->targtype = TARG_NONE;
	//mv->home = -1;
	StartBel(u);
	mv->hp = t->starthp;
	mv->passive = ecfalse;
	mv->prevpos = mv->cmpos;
	mv->taskframe = 0;
	mv->pathblocked = ecfalse;
	mv->jobframes = 0;
	//mv->jobframes = (g_simframe * i + 1) % LOOKJOB_DELAY_MAX;
	mv->supplier = -1;
	mv->exputil = 0;
	mv->reqamt = 0;
	mv->targtype = -1;
	mv->frame[BODY_LOWER] = 0;
	mv->frame[BODY_UPPER] = 0;
	mv->subgoal = mv->goal;

	mv->mode = UMODE_NONE;
	mv->pathdelay = 0;
	mv->lastpath = g_simframe;

	mv->cdtype = CD_NONE;
	mv->driver = -1;
	//mv->framesleft = 0;
	mv->cyframes = WORK_DELAY-1;
	//mv->cyframes = (g_simframe * i + 111) % WORK_DELAY;
	mv->cargoamt = 0;
	mv->cargotype = -1;

	mv->rotation.z = 0;
	mv->frame[BODY_LOWER] = 0;
	CalcRot(u);

	mv->forsale = ecfalse;
	mv->price = 1;

	mv->incomerate = 0;
	mv->cyclehist.clear();

	if(type == MV_TRUCK)
	{
		mv->cyclehist.push_back(TrCycleHist());
		
		while(mv->cyclehist.size() > 100)
			mv->cyclehist.erase(mv->cyclehist.begin());
		
		TrCycleHist* trch = &*mv->cyclehist.rbegin();
		Py* py = &g_py[owner];

		trch->opwage = py->truckwage;
		trch->trprice = py->transpcost;
	}

	if(type == MV_LABOURER)
	{
		mv->jobframes = (g_simframe * i + 1) % LOOKJOB_DELAY_MAX;
		mv->cyframes = (g_simframe * i + 111) % WORK_DELAY;
	}

	//if (UnitCollides(u, mv->cmpos, mv->type))
	//	InfoMess("c", "c");
	
	//if(u - g_mv == 182 && g_simframe > 118500)
		//Log("f5");

	mv->fillcollider();
	g_drawlist.push_back(Dl());
	Dl* d = &*g_drawlist.rbegin();
	d->dtype = DEPTH_U;
	d->index = i;
	mv->depth = d;
	UpDraw(u);
	AddVis(u);
	Explore(u);

	mv->mazeavoid = ecfalse;

	return ectrue;
}

void FreeUnits()
{
	for(int i=0; i<MOVERS; i++)
	{
		g_mv[i].destroy();
		g_mv[i].on = ecfalse;
	}
}

ecbool Mv::hidden() const
{
	switch(mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		return ectrue;
	default:break;
	}

	return ecfalse;
}

void AnimUnit(Mv* mv)
{
	MvType* t = &g_mvtype[mv->type];

	if(/* mv->type == MV_BATTLECOMP || */ mv->type == MV_LABOURER)
	{
		//if(mv->prevpos == mv->cmpos)
		//if(mv->mode == UMODE_NONE)
		//if(!mv->path.size())
		if(mv->goal == mv->cmpos)
		{
			//mv->frame[BODY_LOWER] = 0;
			return;
		}

		PlayAni(mv->frame[BODY_LOWER], 0, 29, ectrue, 1.3f);
	}
}

void UpdAI(Mv* mv)
{
	//return;	//do nothing for now?

	if(mv->type == MV_LABOURER)
		UpdLab(u);
	else if(mv->type == MV_TRUCK)
		UpdTruck(u);
}

void UpdCheck(Mv* mv)
{
	if(mv->type == MV_LABOURER)
		UpdLab2(u);
}

void UpdMvs()
{
#ifdef TSDEBUG
	if(tracku)
	{
		Log("tracku t"<<tracku->type<<" mode"<<(int)tracku->mode<<" tpathsz"<<tracku->tpath.size()<<" pathsz"<<tracku->path.size());
	}
#endif

	for(int i = 0; i < MOVERS; i++)
	{
		StartTimer(TIMER_UPDUONCHECK);

		Mv* mv = &g_mv[i];

		if(!mv->on)
		{
			StopTimer(TIMER_UPDUONCHECK);
			continue;
		}

		RemVis(u);
		AddVis(u);

		StopTimer(TIMER_UPDUONCHECK);

#if 1
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif

#if 1
		/*
		Second check, because unit might be destroyed in last call,
		which might remove Dl ->depth, which would cause a
		crash in MoveUnit.
		*/
		if(!mv->on)
			continue;
#endif

		//TODO animate mv movement
		StartTimer(TIMER_MOVEUNIT);
		MoveUnit(u);
		StopTimer(TIMER_MOVEUNIT);
		//must be called after Move... labs without paths are stuck
		UpdCheck(u);
		StartTimer(TIMER_ANIMUNIT);
		AnimUnit(u);
		StopTimer(TIMER_ANIMUNIT);

#if 0
		/*
		Last call, because unit might be destroyed in this call,
		which might remove Dl ->depth, which would cause a
		crash in MoveUnit.
		*/
		StartTimer(TIMER_UPDUNITAI);
		UpdAI(u);
		StopTimer(TIMER_UPDUNITAI);
#endif
	}
}

void ResetPath(Mv* mv)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_mv == 19)
	{
		Log("the 13th unit:");
		Log("path reset");
		InfoMess("pr", "pr");
	}
#endif

	mv->path.clear();
	mv->tpath.clear();

#ifdef RANDOM8DEBUG
	if(u - g_mv == thatunit)
	{
		Log("ResetPath u=thatunit");
	}
#endif
}

void ResetGoal(Mv* mv)
{
#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_mv == 19)
	{
		Log("the 13th unit:");
		Log("g reset");
		InfoMess("rg", "rg");
	}
#endif

	mv->exputil = 0;
	mv->goal = mv->subgoal = mv->cmpos;
	ResetPath(u);

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset path track u 3", "reset path track u 3");
	}
#endif
}

void ResetMode(Mv* mv, ecbool chcollider)
{
#ifdef RANDOM8DEBUG
	if(u - g_mv == thatunit)
	{
		Log("\tResetMode u=thatunit");
	}
#endif

#ifdef HIERDEBUG
	if(u - g_mv == 5 && mv->mode == UMODE_GOBLJOB && mv->target == 5)
	{
		InfoMess("rsu5", "rsu5");
	}
#endif

#ifdef HIERDEBUG
	//if(pathnum == 73)
	if(u - g_mv == 19)
	{
		Log("the 13th unit:");
		Log("mode reset");
		char msg[128];
		sprintf(msg, "rm %s prevm=%d", g_mvtype[mv->type].name, (int)mv->mode);
		InfoMess("rm", msg);
	}
#endif

#if 0
	switch(mv->mode)
	{
	case UMODE_BLJOB:
	case UMODE_CSTJOB:
	case UMODE_CDJOB:
	case UMODE_SHOPPING:
	case UMODE_RESTING:
	case UMODE_DRIVE:
	//case UMODE_REFUELING:
	//case UMODE_ATDEMB:
	//case UMODE_ATDEMCD:
		mv->freecollider();
		PlaceUAb(mv->type, mv->cmpos, &mv->cmpos);
		mv->fillcollider();
		UpDraw(u);
	default:break;
	}
#else
	if(mv->mode == UMODE_DRIVE &&
		mv->type == MV_LABOURER)	//corpc fix
	{
		Mv* tr = &g_mv[mv->target];
		tr->driver = -1;
		
		RemVis(u);
		//mv->freecollider();	//d fix
		PlaceUAb(mv->type, tr->cmpos, &mv->cmpos);
#if 0
		//TODO
		mv->drawpos.x = mv->cmpos.x;
		mv->drawpos.y = mv->cmpos.y;
		mv->drawpos.y = g_hmap.accheight(mv->cmpos.x, mv->cmpos.y);
#endif
		
	//if(u - g_mv == 182 && g_simframe > 118500)
		//Log("f6");

		if(chcollider)
			mv->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
	else if(mv->hidden())
	{
		RemVis(u);
		//mv->freecollider();	//d fix
		PlaceUAb(mv->type, mv->cmpos, &mv->cmpos);
#if 0
		//TODO
		mv->drawpos.x = mv->cmpos.x;
		mv->drawpos.y = mv->cmpos.y;
		mv->drawpos.y = g_hmap.accheight(mv->cmpos.x, mv->cmpos.y);
#endif
		
	//if(u - g_mv == 182 && g_simframe > 118500)
		//Log("f13 m=%d", (int)mv->mode);

		if (chcollider)
			mv->fillcollider();
		AddVis(u);
		UpDraw(u);
	}
#endif

#if 0
	//URAN_DEBUG
	if(u-g_mv == 19)
	{
		Bl* b = &g_bl[5];
		char msg[1280];
		sprintf(msg, "ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n mv->cargotype=%d",
			(int)b->transporter[RES_URANIUM],
			(int)g_mv[b->transporter[RES_URANIUM]].mode,
			(int)g_mv[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_mv[b->transporter[RES_URANIUM]].targtype,
			(int)g_mv[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 1");
	if(mv->type == MV_LABOURER)
	{
		//LastNum("resetmode 1a");
		if(mv->mode == UMODE_BLJOB ||
			mv->mode == UMODE_CSTJOB)
			RemWorker(u);

		else if(mv->mode == UMODE_SHOPPING)
			RemShopper(u);

		mv->jobframes = LOOKJOB_DELAY_MAX;

		//if(hidden())
		//	relocate();
	}
	else if(mv->type == MV_TRUCK)
	{
#if 1
		if(mv->mode == UMODE_GOSUP
		                //|| mode == GOINGTOREFUEL
		                //|| mode == GOINGTODEMANDERB || mode == GOINGTODEMROAD || mode == GOINGTODEMPIPE || mode == GOINGTODEMPOWL
		  )
		{
			if(mv->supplier >= 0)
			{
				//necessary?
				Bl* b = &g_bl[mv->supplier];
				b->transporter[mv->cargotype] = -1;
			}
		}

		ecbool targbl = ecfalse;
		ecbool targcd = ecfalse;

		if( (mv->mode == UMODE_GOSUP || mv->mode == UMODE_ATSUP || mv->mode == UMODE_GOREFUEL || mv->mode == UMODE_REFUELING) && mv->targtype == TARG_BL )
			targbl = ectrue;

		if( (mv->mode == UMODE_GOSUP || mv->mode == UMODE_ATSUP || mv->mode == UMODE_GOREFUEL || mv->mode == UMODE_REFUELING) && mv->targtype == TARG_CD )
			targbl = ectrue;

		if(mv->mode == UMODE_GODEMB)
			targbl = ectrue;

		if(mv->mode == UMODE_ATDEMB)
			targbl = ectrue;

		if(mv->mode == UMODE_GODEMCD)
			targcd = ectrue;

		if(mv->mode == UMODE_ATDEMCD)
			targcd = ectrue;

		if(mv->targtype == TARG_BL)
			targbl = ectrue;

		if(mv->targtype == TARG_CD)
			targcd = ectrue;

		if( targbl )
		{
			if(mv->target >= 0)
			{
				Bl* b = &g_bl[mv->target];
				b->transporter[mv->cargotype] = -1;
			}
		}
		else if( targcd )
		{
			if(mv->target >= 0 && mv->target2 >= 0 && mv->cdtype >= 0)
			{
				CdTile* ctile = GetCd(mv->cdtype, mv->target, mv->target2, ecfalse);
				ctile->transporter[mv->cargotype] = -1;
			}
		}
#endif
		mv->targtype = TARG_NONE;

		if(mv->driver >= 0)
		{
			Mv* op = &g_mv[mv->driver];
			//LastNum("resetmode 1b");
			//g_mv[mv->driver].Disembark();

			//corpc fix
			//important to reset driver first and then tell him to disembark so we don't get stuck in a resetting loop
			mv->driver = -1;

			if(op->mode == UMODE_DRIVE)
				Disembark(op);
		}
	}

	//LastNum("resetmode 2");

	//transportAmt = 0;
	mv->targtype = TARG_NONE;
	mv->target = mv->target2 = -1;
	mv->supplier = -1;
	mv->mode = UMODE_NONE;
	ResetGoal(u);

#if 0
	//URAN_DEBUG
	if(u-g_mv == 19)
	{
		Bl* b = &g_bl[5];
		char msg[1280];
		sprintf(msg, "/ResetMode u13truck culprit \n ur tr:%d tr's mode:%d tr's tar:%d thisb%d targtyp%d \n cargty%d",
			(int)b->transporter[RES_URANIUM],
			(int)g_mv[b->transporter[RES_URANIUM]].mode,
			(int)g_mv[b->transporter[RES_URANIUM]].target,
			5,
			(int)g_mv[b->transporter[RES_URANIUM]].targtype,
			(int)g_mv[b->transporter[RES_URANIUM]].cargotype);
		InfoMess(msg, msg);
	}
#endif

	//LastNum("resetmode 3");
}

void ResetTarget(Mv* mv)
{

#ifdef TSDEBUG
	if(u == tracku)
	{
		InfoMess("reset mode track u 3", "reset mode track u 3");
	}
#endif

	ResetMode(u);
	mv->target = -1;
}
