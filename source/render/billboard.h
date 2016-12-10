










#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "../platform.h"
#include "../math/3dmath.h"
#include "../math/vec3f.h"

struct BbType
{
public:
	ecbool on;
	char name[32];
	unsigned int tex;

	BbType()
	{
		on = ecfalse;
	}
};

#define BB_TYPES			64
extern BbType g_bbt[BB_TYPES];

struct Bb
{
public:
	ecbool on;
	int type;
	float size;
	Vec3f pos;
	float dist;
	int particle;

	Bb()
	{
		on = ecfalse;
		particle = -1;
	}
};

#define BILLBOARDS  256
extern Bb g_bb[BILLBOARDS];

extern unsigned int g_muzzle[4];

void Effects();
int NewBb();
int NewBb(char* tex);
int IdentifyBb(const char* name);
void SortBb();
void DrawBillboards();
void PlaceBb(const char* n, Vec3f pos, float size, int particle=-1);
void PlaceBb(int type, Vec3f pos, float size, int particle=-1);

#endif
