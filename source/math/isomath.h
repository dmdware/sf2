










#ifndef ISOMATH_H
#define ISOMATH_H

#include "vec3i.h"
#include "vec3f.h"
#include "vec2i.h"
#include "fixmath.h"

#define TILE_SIZE			(10*100)	//10 meters = 1,000 centimeters
//#define TILE_RISE			(TILE_SIZE/3)
#define TILE_DIAG			(isqrt(TILE_SIZE*TILE_SIZE*2))
//#define TILE_DIAG			(sqrtf(TILE_SIZE*TILE_SIZE*2))
//#define TILE_RISE			(tan(DEGTORAD(30))*TILE_DIAG/2)
#define TILE_RISE			(tan(DEGTORAD(30))*TILE_DIAG/4)//268.64	//

//#define TILE_PIXEL_WIDTH	256
//#define TILE_PIXEL_WIDTH	64
#define TILE_PIXEL_WIDTH	128
#define TILE_PIXEL_RISE		(TILE_PIXEL_WIDTH / 8)

Vec2i CartToIso(Vec3i cmpos);
int CartZToIso(int cmy);
Vec2i CartToIso2(Vec3i cmpos);
int CartZToIso2(int cmy);
void IsoToCart(Vec2i pixpos, Vec3f *ray, Vec3f *point);
void CartToDepth(Vec3i cmpos, int *depth);

#endif
