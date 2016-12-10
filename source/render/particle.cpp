











#include "particle.h"
#include "../texture.h"
#include "../platform.h"
#include "billboard.h"
#include "../math/vec3f.h"
#include "../window.h"

PlType g_particleT[PARTICLE_TYPES];
Pl g_particle[PARTICLES];

int NewParticle()
{
	for(int i=0; i<PARTICLES; i++)
		if(!g_particle[i].on)
			return i;

	return -1;
}

void DefineParticle(int i, char* texpath, int del, float dec, Vec3f minV, Vec3f maxV, Vec3f minA, Vec3f maxA, float minS, float maxS, void (*collision)(Pl* part, Bb* bb, Vec3f trace, Vec3f normal))
{
	PlType* t = &g_particleT[i];

	t->billbT = IdentifyBb(texpath);
	t->delay = del;
	t->decay = dec * 30.0f;
	t->minvelocity = minV * 30.0f;
	t->velvariation = (maxV-minV) * 30.0f;
	t->minacceleration = minA * 30.0f;
	t->accelvariation = (maxA-minA) * 30.0f;
	t->minsize = minS/2.0f;
	t->sizevariation = (maxS-minS)/2.0f;
	t->collision = collision;
}

void LoadParticles()
{
#if 1
	//void Pl(int i, char* texpath, int del, float dec, Vec3f minV, Vec3f maxV, Vec3f minA, Vec3f maxA, float minS, float maxS, void (*collision)(Pl* part, Bb* bb, Vec3f trace, Vec3f normal))
	DefineParticle(PARTICLE_EXHAUST, "billboards/exhaust.png", 500, 0.002f, Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 30.0f, 100.0f, NULL);
	DefineParticle(PARTICLE_EXHAUST2, "billboards/exhaust2.png", 410, 0.003f,Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 30.0f, 100.0f, NULL);
	DefineParticle(PARTICLE_EXHAUSTBIG, "billboards/exhaust.png", 500, 0.003f, Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 3.5f*10, 17.0f*10, NULL);
	DefineParticle(PARTICLE_FIREBALL, "billboards/fireball.png", 500, 0.03f, Vec3f(0, 0.08f, 0)*100, Vec3f(0.0f, 0.08f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 16.0f*10, 16.0f*10, NULL);
	DefineParticle(PARTICLE_FIREBALL2, "billboards/fireball2.png", 500, 0.03f, Vec3f(0, 0.08f, 0)*100, Vec3f(0.0f, 0.08f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 16.0f*10, 16.0f*10, NULL);
	DefineParticle(PARTICLE_SMOKE, "billboards/smoke.png", 500, 0.005f, Vec3f(0, 0.4f, 0)*100, Vec3f(0.0f, 0.4f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 10.0f*10, 10.0f*10, NULL);
	DefineParticle(PARTICLE_SMOKE2, "billboards/smoke2.png", 500, 0.005f, Vec3f(0, 0.4f, 0)*100, Vec3f(0.0f, 0.4f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 10.0f*10, 10.0f*10, NULL);
	DefineParticle(PARTICLE_DEBRIS, "billboards/debris.png", 500, 0.005f, Vec3f(-0.8f, 0.8f, -0.8f)*100, Vec3f(0.8f, 0.8f, 0.8f)*100, Vec3f(0, -0.05f, 0)*100, Vec3f(0.0f, -0.09f, 0.0f)*100, 0.5f*10, 1.0f*10, NULL);
	DefineParticle(PARTICLE_FLAME, "billboards/flame.png", 50, 0.005f, Vec3f(-0.05f, 0.1f, -0.05f)*100, Vec3f(0.06f, 0.3f, 0.06f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0, 0.0f, 0)*100, 5.5f*10, 6.0f*10, NULL);
	DefineParticle(PARTICLE_PLUME, "billboards/plume.png", 25, 0.005f, Vec3f(-0.05f, 0.2f, -0.05f)*100, Vec3f(0.06f, 0.5f, 0.06f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0, 0.0f, 0)*100, 5.5f*10, 12.0f*10, NULL);
#elif 0
	DefineParticle(PARTICLE_EXHAUST, "billboards/exhaust.png", 500, 0.99f, Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 200.0f, 1000.0f, NULL);
	DefineParticle(PARTICLE_EXHAUST2, "billboards/exhaust2.png", 410, 0.99f,Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 200.0f, 1000.0f, NULL);
	DefineParticle(PARTICLE_EXHAUSTBIG, "billboards/exhaust.png", 500, 0.99f, Vec3f(0.01f, 0.05f, -0.01f)*100, Vec3f(0.05f, 0.08f, -0.05f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, Vec3f(-0.0001f, 0.0f, -0.0001f)*100, 5.0f*100, 17.0f*100, NULL);
	DefineParticle(PARTICLE_FIREBALL, "billboards/fireball.png", 500, 0.9f, Vec3f(0, 0.08f, 0)*100, Vec3f(0.0f, 0.08f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 16.0f*100, 16.0f*100, NULL);
	DefineParticle(PARTICLE_FIREBALL2, "billboards/fireball2.png", 500, 0.9f, Vec3f(0, 0.08f, 0)*100, Vec3f(0.0f, 0.08f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 16.0f*100, 16.0f*100, NULL);
	DefineParticle(PARTICLE_SMOKE, "billboards/smoke.png", 500, 0.999f, Vec3f(0, 0.4f, 0)*100, Vec3f(0.0f, 0.4f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 10.0f*100, 10.0f*100, NULL);
	DefineParticle(PARTICLE_SMOKE2, "billboards/smoke2.png", 500, 0.999f, Vec3f(0, 0.4f, 0)*100, Vec3f(0.0f, 0.4f, 0.0f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0.0f, 0.0f, 0.0f)*100, 10.0f*100, 10.0f*100, NULL);
	DefineParticle(PARTICLE_DEBRIS, "billboards/debris.png", 500, 0.999f, Vec3f(-0.8f, 0.8f, -0.8f)*100, Vec3f(0.8f, 0.8f, 0.8f)*100, Vec3f(0, -0.05f, 0)*100, Vec3f(0.0f, -0.09f, 0.0f)*100, 0.5f*100, 1.0f*100, NULL);
	DefineParticle(PARTICLE_FLAME, "billboards/flame.png", 50, 0.999f, Vec3f(-0.05f, 0.1f, -0.05f)*100, Vec3f(0.06f, 0.3f, 0.06f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0, 0.0f, 0)*100, 5.5f*100, 6.0f*100, NULL);
	DefineParticle(PARTICLE_PLUME, "billboards/plume.png", 25, 0.99f, Vec3f(-0.05f, 0.2f, -0.05f)*100, Vec3f(0.06f, 0.5f, 0.06f)*100, Vec3f(0, 0.0f, 0)*100, Vec3f(0, 0.0f, 0)*100, 5.5f*100, 12.0f*100, NULL);
#endif
}

void Pl::update(Bb* bb)
{
	PlType* t = &g_particleT[type];
#if 1
	life -= t->decay * g_drawfrinterval;
#else
	life *= t->decay;
#endif

#if 1
	if(life <= 0.0f)
#else
	if(life <= 0.001f)
#endif
	{
		on = ecfalse;
		bb->on = ecfalse;
		return;
	}

	/*
	Vec3f to = bb->pos + vel;

	Vec3f trace = g_edmap.TraceRay(bb->pos, to);

	if(trace != to && t->collision != NULL)
	t->collision(this, bb, trace, g_edmap.CollisionNormal());
	*/

	bb->pos = bb->pos + vel * g_drawfrinterval;
	Vec3f accel;
	accel.x = t->minacceleration.x + t->accelvariation.x * (float)(rand()%1000)/1000.0f;
	accel.y = t->minacceleration.y + t->accelvariation.y * (float)(rand()%1000)/1000.0f;
	accel.z = t->minacceleration.z + t->accelvariation.z * (float)(rand()%1000)/1000.0f;

	vel = vel + accel * g_drawfrinterval;
}

void EmitParticle(int type, Vec3f pos)
{
	int i = NewParticle();
	if(i < 0)
		return;

	PlType* t = &g_particleT[type];
	PlaceBb(t->billbT, pos, t->minsize, i);

	Pl* p = &g_particle[i];

	p->on = ectrue;
	p->life = 1;
	p->vel.x = t->minvelocity.x + t->velvariation.x * (float)(rand()%1000)/1000.0f;
	p->vel.y = t->minvelocity.y + t->velvariation.y * (float)(rand()%1000)/1000.0f;
	p->vel.z = t->minvelocity.z + t->velvariation.z * (float)(rand()%1000)/1000.0f;
	p->type = type;
}

void FreeParts()
{
	Bb* b;
	Pl* p;

	for(int i=0; i<BILLBOARDS; i++)
	{
		b = &g_bb[i];

		if(b->particle < 0)
			continue;

		p = &g_particle[b->particle];

		p->on = ecfalse;
		b->on = ecfalse;
	}
}

void UpdParts()
{
	Bb* b;
	Pl* p;

	for(int i=0; i<BILLBOARDS; i++)
	{
		b = &g_bb[i];

		if(!b->on)
			continue;

		if(b->particle < 0)
			continue;

		p = &g_particle[b->particle];

		p->update(b);
	}
}
