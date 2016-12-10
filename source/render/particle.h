











#ifndef PARTICLE_H
#define PARTICLE_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"
#include "../utils.h"

struct Pl;
struct Bb;

struct PlType
{
public:
	int billbT;
	int delay;
	float decay;
	Vec3f minvelocity;
	Vec3f velvariation;
	Vec3f minacceleration;
	Vec3f accelvariation;
	float minsize;
	float sizevariation;
	void (*collision)(Pl* part, Bb* bb, Vec3f trace, Vec3f normal);
};

#define PARTICLE_EXHAUST		0
#define PARTICLE_EXHAUST2		1
#define PARTICLE_EXHAUSTBIG		2
#define PARTICLE_FIREBALL		3
#define PARTICLE_FIREBALL2		4
#define PARTICLE_SMOKE			5
#define PARTICLE_SMOKE2			6
#define PARTICLE_DEBRIS			7
#define PARTICLE_FLAME			8
#define PARTICLE_PLUME			9
#define PARTICLE_TYPES			10
extern PlType g_particleT[PARTICLE_TYPES];

struct EmCntr
{
public:
	unsigned __int64 last;

	EmCntr()
	{
		last = GetTicks();
	}
	ecbool emitnext(int delay)
	{
		if(GetTicks()-last > (unsigned __int64)delay)
		{
			last = GetTicks();
			return ectrue;
		}
		else
			return ecfalse;
	}
};

struct Bb;

struct Pl
{
public:
	ecbool on;
	int type;
	float life;
	Vec3f vel;
	float dist;

	Pl()
	{
		on = ecfalse;
	}
	Pl(Vec3f p, Vec3f v)
	{
		on = ectrue;
		vel = v;
	}

	void update(Bb* bb);
};

#define PARTICLES 512
extern Pl g_particle[PARTICLES];

struct EmitterPlace
{
public:
	EmitterPlace()
	{
		on = ecfalse;
	}
	EmitterPlace(int t, Vec3f off)
	{
		on = ectrue;
		type = t;
		offset = off;
	}
	ecbool on;
	Vec3f offset;
	int type;
};

void LoadParticles();
void Particles();
void EmitParticle(int type, Vec3f pos);
void UpdParts();
void FreeParts();

#endif
