

#ifndef APPVIEWPORT_H
#define APPVIEWPORT_H

#include "../math/3dmath.h"
#include "../math/camera.h"
#include "../math/vec2i.h"

#define VERT_DRAG_HSIZE		3
#define VERT_DRAG_OUTLRGBA	{0.5f, 0.5f, 0.5f, 0.5f}
#define VERT_DRAG_FILLRGBA	{0.7f, 0.7f, 0.7f, 0.5f}
#define FACE_DRAG_HSIZE		4
#define FACE_DRAG_OUTLRGBA	{0.7f, 0.1f, 0.1f, 0.5f}
#define FACE_DRAG_FILLRGBA	{0.9f, 0.3f, 0.3f, 0.5f}
#define FACE_DRAG_SOUTLRGBA	{0.7f, 0.1f, 0.1f, 1.0f}
#define FACE_DRAG_SFILLRGBA	{0.9f, 0.3f, 0.3f, 1.0f}
#define BRUSH_DRAG_HSIZE	5
#define BRUSH_DRAG_OUTLRGBA {0.0f, 0.0f, 0.0f, 0.75f}
#define BRUSH_DRAG_FILLRGBA {0.4f, 0.4f, 0.4f, 0.75f}
#define DOOR_POINT_DRAG_HSIZE		7
#define DOOR_POINT_DRAG_OUTLRGBA	{0.7f, 0.7f, 0.0f, 0.5f}
#define DOOR_POINT_DRAG_FILLRGBA	{0.5f, 0.5f, 0.0f, 0.5f}
#define DOOR_AXIS_DRAG_HSIZE		4
#define DOOR_AXIS_DRAG_OUTLRGBA		{0.9f, 0.9f, 0.0f, 0.5f}
#define DOOR_AXIS_DRAG_FILLRGBA		{0.7f, 0.7f, 0.0f, 0.5f}

#define DRAG_TOP		0
#define DRAG_BOTTOM		1
#define DRAG_LEFT		2
#define DRAG_RIGHT		3
#define DRAG_NEAR		4
#define DRAG_FAR		5

#define VIEWPORT_FRONT		0
#define VIEWPORT_LEFT		1
#define VIEWPORT_TOP		2
//#define VIEWPORT_BACK		3
//#define VIEWPORT_RIGHT	4
//#define VIEWPORT_BOTTOM	5
#define VIEWPORT_ANGLE45O	6
#define VIEWPORT_TYPES		7

struct VpType
{
public:
	Vec3f offset;
	Vec3f up;
	char label[32];

	VpType(){}
	VpType(Vec3f offset, Vec3f up, const char* label);
};

extern VpType g_vptype[VIEWPORT_TYPES];

struct VpWrap
{
public:
	int type;
	ecbool ldown;
	ecbool rdown;
	ecbool mdown;
	Vec2i lastmouse;
	Vec2i curmouse;
	ecbool drag;

	VpWrap();
	VpWrap(int type);
	Vec3f up();
	Vec3f up2();
	Vec3f strafe();
	Vec3f focus();
	Vec3f viewdir();
	Vec3f pos();
};

extern VpWrap g_viewport[4];
//extern Vec3f g_focus;

void DrawViewport(int which, int x, int y, int width, int height);
ecbool ViewportLDown(int which, int relx, int rely, int width, int height);
ecbool ViewportLUp(int which, int relx, int rely, int width, int height);
ecbool ViewportMousemove(int which, int relx, int rely, int width, int height);
ecbool ViewportRDown(int which, int relx, int rely, int width, int height);
ecbool ViewportRUp(int which, int relx, int rely, int width, int height);
ecbool ViewportMousewheel(int which, int delta);

#endif