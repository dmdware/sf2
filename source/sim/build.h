











#ifndef BUILD_H
#define BUILD_H

#include "../math/vec3f.h"
#include "../math/vec2i.h"

struct Bl;
struct BlType;
struct Matrix;

void DrawSBl(unsigned int renderdepthtex, unsigned int renderfb);
void UpdSBl();
ecbool CheckCanPlace(int type, Vec2i tpos, int targb);
void RecheckStand();
void DrawBReason(Matrix* mvp, float width, float height, ecbool persp);
ecbool PlaceBl(int type, Vec2i pos, ecbool finished, int owner, int* bid);
ecbool PlaceBAb(int btype, Vec2i tabout, Vec2i* tplace);
ecbool PlaceBAb4(int btype, Vec2i tabout, Vec2i* tplace);
ecbool PlaceUAb(int mvtype, Vec2i cmabout, Vec2i* cmplace);
ecbool BlCollides(int type, Vec2i tpos, int targb);
ecbool Collides(Vec2i cmmin, Vec2i cmmax, int targb);

#endif
