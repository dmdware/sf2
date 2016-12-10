
#include "../save/edmap.h"
#include "door.h"

EdDoor::EdDoor()
{
	nsides = 0;
	sides = NULL;
}

EdDoor::~EdDoor()
{
	if(sides)
	{
		delete [] sides;
		sides = NULL;
		nsides = 0;
	}
}

EdDoor& EdDoor::operator=(const EdDoor& original)
{
	/*
	Vec3f axis;
	Vec3f point;
	float opendeg;	//show degrees
	ecbool startopen;
	*/

	axis = original.axis;
	point = original.point;
	opendeg = original.opendeg;
	startopen = original.startopen;

	nsides = original.nsides;

	if(sides)
	{
		delete [] sides;
		sides = NULL;
	}

	sides = new BrushSide[nsides];
	for(int i=0; i<nsides; i++)
		sides[i] = original.sides[i];

	return *this;
}

EdDoor::EdDoor(const EdDoor& original)
{
	nsides = 0;
	sides = NULL;
	*this = original;
}