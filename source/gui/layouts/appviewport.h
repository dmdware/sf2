










#ifndef APPVIEWPORT_H
#define APPVIEWPORT_H

#include "../../math/3dmath.h"
#include "../../math/camera.h"
#include "../../math/vec2i.h"

#define VIEWPORT_MINIMAP		0
#define VIEWPORT_ENTVIEW		1
#define VIEWPORT_TYPES			2

struct VpType
{
	Vec3f offset;
	Vec3f up;
	char label[32];
	ecbool axial;
};

void VpType_init(VpType* vt, Vec3f offset, Vec3f up, const char* label, ecbool axial);

extern VpType g_vptype[VIEWPORT_TYPES];

#endif
