

#ifndef DOOR_H
#define DOOR_H

#include "../math/vec3f.h"
#include "../math/3dmath.h"
//#include "../bsp/brushside.h"
//#include "../bsp/brush.h"

struct BrushSide;

struct EdDoor
{
public:
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	ecbool startopen;
	
	//Closed state brush sides
	int nsides;
	BrushSide* sides;

	EdDoor();
	~EdDoor();
	EdDoor& operator=(const EdDoor& original);
	EdDoor(const EdDoor& original);
};

struct Door
{
public:
	Vec3f axis;
	Vec3f point;
	float opendeg;	//degrees
	float openness;	//ratio of how show it is
	ecbool opening;

	//Brush* brushp;
	//Brush closedstate;
};

#endif