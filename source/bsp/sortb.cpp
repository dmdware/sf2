

#include "sortb.h"
#include "../bsp/brush.h"
#include "../sim/map.h"
#include "../save/edmap.h"
#include "../math/camera.h"
#include "../math/3dmath.h"
#include "../platform.h"

static Vec3f sorteye;
static Vec3f sortview;

ecbool EdBCompare(const Brush& a, const Brush& b)
{
	//if(a.age == b.age)
	//	return a.name < b.name;

	//return a.age > b.age;

	float fara = 0;
	float farb = 0;

	for(int i=0; i<a.nsharedv; i++)
	{
		Vec3f* sharedv = &a.sharedv[i];

		float thisfar = Dot(sortview, *sharedv - sorteye);

		if(thisfar > fara || i == 0)
			fara = thisfar;
	}

	for(int i=0; i<b.nsharedv; i++)
	{
		Vec3f* sharedv = &b.sharedv[i];

		float thisfar = Dot(sortview, *sharedv - sorteye);

		if(thisfar > farb || i == 0)
			farb = thisfar;
	}

	return fara > farb;
}

void SortEdB(EdMap* map, Vec3f focus, Vec3f eye)
{
	sorteye = eye;
	sortview = Normalize(focus - eye);

	map->brush.sort(EdBCompare);
}