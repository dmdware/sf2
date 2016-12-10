#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include "../math/vec3i.h"

//NOT USED

struct Collidable
{
public:
	Vec3i colbox;
	signed char coltype;
};

#define COLLIDABLE_BUILDING		0
#define COLLIDABLE_UNIT			1
#define COLLIDABLE_TILE			2


#endif
