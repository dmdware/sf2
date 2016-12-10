











#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "../platform.h"
#include "../gui/richtext.h"
#include "../math/vec3f.h"
#include "../math/vec2f.h"
#include "../math/vec2i.h"

struct Transaction
{
public:
	RichText rtext;
	Vec3f drawpos;
	float life;
	float halfwidth;
};

#define TRANSACTION_RISE		(15.0f*30.0f)
#define TRANSACTION_DECAY		(0.015f*30.0f)

extern std::list<Transaction> g_transx;
extern ecbool g_drawtransx;

struct Matrix;

void DrawTransx();
void NewTransx(Vec2i cmpos, const RichText* rtext);
void FreeTransx();

#endif
