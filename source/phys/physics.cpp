

#include "physics.h"
#include "../math/camera.h"
#include "../window.h"
#include "../bsp/trace.h"
#include "../bsp/tracework.h"
#include "../sim/entity.h"
#include "../save/savemap.h"
#include "../debug.h"
#include "../utils.h"
#include "../bsp/q3bsp.h"

#define MAX_CHAR_STEP	25
//#define MAX_CHAR_STEP	75

void Physics()
{
	//return;

	for(int ei=0; ei<ENTITIES; ei++)
	{
		//if(!g_pcam->grounded)
			//g_pcam->accelrise(- g_drawfrinterval * GRAVITY);
		
		Ent* e = &g_entity[ei];

		if(!e->on)
			continue;

		EType* et = &g_etype[e->type];
		Camera* c = &e->camera;

		Vec3f old = c->pos;
		Vec3f next = c->pos + c->vel;

		if(!c->grounded)
			c->accelrise(-GRAVITY * g_updfrinterval * SIM_FRAME_RATE * g_updfrinterval * SIM_FRAME_RATE);
		else
			c->vel.y = 0;

		c->friction();

#if 1
		c->moveto( g_bsp.TraceBox(old, next, et->vMin, et->vMax, et->maxStep) );
		c->grounded(g_bsp.IsOnGround());
#else
		TraceWork tw;
		TraceBox(&g_map.brush, &tw, old, next, et->vMin, et->vMax, MAX_CHAR_STEP);

		//g_applog<<"tracework ratio="<<tw.traceratio<<" delta="<<(next-old).x<<","<<(next-old).y<<","<<(next-old).z<<std::endl;
		//g_applog.flush();

		//c->moveto(old + (next-old) * tw.traceratio);
		c->moveto(tw.clip);
		c->grounded(tw.onground);
#endif
	}
}