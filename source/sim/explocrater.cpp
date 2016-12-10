

#include "../render/shader.h"
#include "../platform.h"
#include "../utils.h"
#include "../math/3dmath.h"
#include "../math/vec4f.h"
#include "../math/plane3f.h"
#include "../gui/gui.h"
#include "sim.h"
#include "door.h"
#include "../save/compilemap.h"
#include "../app/segui.h"
#include "../app/sesim.h"

void MakeCrater(Vec3f center, std::list<Plane3f> &crater, float radius, int sides)
{
	BrushSide down(Vec3f(0.0f, -1.0f, 0.0f), center + Vec3f(0, -radius, 0));

	crater.push_back(down.plane);

	for(int y=1; y<sides; y++)
		for(int x=1; x<sides; x++)
		{
			Plane3f transformed(down.plane);
			RotatePlane(transformed, center, DEGTORAD((360*y/(sides))), Vec3f(0,0,1));
			RotatePlane(transformed, center, DEGTORAD((360*x/(sides))), Vec3f(0,1,0));
			crater.push_back(transformed);
		}
}

ecbool BrushInCrater(Brush* b, std::list<Plane3f> &crater)
{
	ecbool intersect = ectrue;

	for(std::list<Plane3f>::iterator ep=crater.begin(); ep!=crater.end(); ep++)
	{
		ecbool outall = ectrue;

		for(int bv=0; bv<b->nsharedv; bv++)
		{
			Vec3f v = b->sharedv[bv];

			if(PointOnOrBehindPlane(v, ep->normal, ep->d))
			{
				outall = ecfalse;
				break;
			}
		}

		if(outall)
		{
			intersect = ecfalse;
			break;
		}
	}

	return intersect;
}

void ExplodeCrater(EdMap* map, Vec3f line[], Vec3f vmin, Vec3f vmax)
{
	g_dragV = -1;
	g_dragS = -1;
	g_dragW = ecfalse;
	g_dragD = -1;
	g_selB.clear();
	g_selM.clear();
	CloseSideView();
	//CloseView("brush edit");

	//Log("select brush ("<<line[0].x<<","<<line[0].y<<","<<line[0].z<<")->("<<line[1].x<<","<<line[1].y<<","<<line[1].z<<")");
	Brush* hitB = NULL;

	for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)
	{
		Vec3f trace = b->traceray(line);
		if(trace != line[1] && trace.y <= g_maxelev
#if 1
			&&
			trace.x >= vmin.x && trace.x <= vmax.x &&
			trace.y >= vmin.y && trace.y <= vmax.y &&
			trace.z >= vmin.z && trace.z <= vmax.z
#endif
			)
		{
			line[1] = trace;
			//g_selB.clear();
			//g_selB.push_back(&*b);
			hitB = &*b;
			//OpenAnotherView("brush edit");
			//return;
		}
	}

	if(!hitB)
		return;

	std::list<Plane3f> crater;
	MakeCrater(line[1], crater, 100.0f, 6);

	std::list<Brush> hitBs;

	//for(std::list<Brush>::iterator b=map->brush.begin(); b!=map->brush.end(); b++)

	std::list<Brush>::iterator b = map->brush.begin();

	while(b != map->brush.end())
	{
		ecbool intersect = BrushInCrater(&*b, crater);

		if(intersect)
		{
			hitBs.push_back(*b);
			b = map->brush.erase(b);
			continue;
		}

		b++;
	}

#if 0
	Brush craterb;

	for(std::list<Brush>::iterator pl=crater.begin(); pl!=crater.end(); pl++)
	{
		Vec3f point = PointOnPlane(*pl);
		BrushSide bs(pl->normal, point);
		craterb.add(bs);
	}

	craterb.collapse();

	g_edmap.brush.push_back(craterb);
#endif

#if 1
	for(std::list<Brush>::iterator b=hitBs.begin(); b!=hitBs.end(); b=hitBs.erase(b))
	{
		std::list<Brush> outfrags;
		std::list<Brush> infrags;

		infrags.push_back(*b);

		for(std::list<Plane3f>::iterator ep=crater.begin(); ep!=crater.end(); ep++)
		{
			Vec3f pop = PointOnPlane(*ep);
			std::list<Brush> nextoutfrags;
			std::list<Brush> nextinfrags;

			BrushSide news0(ep->normal, pop);
			BrushSide news1(Vec3f(0,0,0)-ep->normal, pop);

			for(std::list<Brush>::iterator fragb=infrags.begin(); fragb!=infrags.end(); fragb++)
			{
				if(!BrushInCrater(&*fragb, crater))
				{
					nextoutfrags.push_back(*fragb);
					continue;
				}

				if(!PruneB2(&*fragb, &news0.plane))
				{
					Brush newfrag0 = *fragb;
					newfrag0.add(news0);
					newfrag0.collapse();
					newfrag0.remaptex();
					nextinfrags.push_back(newfrag0);
					std::list<Brush>::const_reverse_iterator j = nextinfrags.rbegin();
					if(PruneB(map, (Brush*)&*j))
						nextinfrags.erase(--(j.base()));
				}

				if(!PruneB2(&*fragb, &news1.plane))
				{
					Brush newfrag1 = *fragb;
					newfrag1.add(news1);
					newfrag1.collapse();
					newfrag1.remaptex();
					nextoutfrags.push_back(newfrag1);
					std::list<Brush>::const_reverse_iterator j = nextoutfrags.rbegin();
					if(PruneB(map, (Brush*)&*j))
						nextoutfrags.erase(--(j.base()));
				}
			}

			for(std::list<Brush>::iterator fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
			{
				if(!BrushInCrater(&*fragb, crater))
				{
					nextoutfrags.push_back(*fragb);
					continue;
				}

				if(!PruneB2(&*fragb, &news0.plane))
				{
					Brush newfrag0 = *fragb;
					newfrag0.add(news0);
					newfrag0.collapse();
					newfrag0.remaptex();
					nextoutfrags.push_back(newfrag0);
					std::list<Brush>::const_reverse_iterator j = nextoutfrags.rbegin();
					if(PruneB(map, (Brush*)&*j))
						nextoutfrags.erase(--(j.base()));
				}

				if(!PruneB2(&*fragb, &news1.plane))
				{
					Brush newfrag1 = *fragb;
					newfrag1.add(news1);
					newfrag1.collapse();
					newfrag1.remaptex();
					nextoutfrags.push_back(newfrag1);
					std::list<Brush>::const_reverse_iterator j = nextoutfrags.rbegin();
					if(PruneB(map, (Brush*)&*j))
						nextoutfrags.erase(--(j.base()));
				}
			}

			infrags = nextinfrags;
			outfrags = nextoutfrags;
		}

#if 1
		for(std::list<Brush>::iterator fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
		{
			ecbool binall = ectrue;

			for(std::list<Plane3f>::iterator ep=crater.begin(); ep!=crater.end(); ep++)
			{
				Vec3f pop = PointOnPlane(*ep);

				ecbool vinall = ectrue;

				for(int svi=0; svi<fragb->nsharedv; svi++)
				{
					Vec3f sv = fragb->sharedv[svi];

					if(!PointOnOrBehindPlane(sv, ep->normal, ep->d, CLOSE_EPSILON*2))
					{
						vinall = ecfalse;
						break;
					}
				}

				if(!vinall)
				{
					binall = ecfalse;
					break;
				}
			}

			if(binall)
				continue;

			g_edmap.brush.push_back(*fragb);
		}
#else
		for(std::list<Brush>::iterator fragb=outfrags.begin(); fragb!=outfrags.end(); fragb++)
		{
			g_edmap.brush.push_back(*fragb);
		}
#endif

#if 0
	Vec3f pop = PointOnPlane(cuttingp);
	BrushSide news0(cuttingp.normal, pop);
	BrushSide news1(Vec3f(0,0,0)-cuttingp.normal, pop);

	std::vector<Brush*> newsel;

	for(std::list<Brush>::iterator i=g_selB.begin(); i!=g_selB.end(); )
	{
		Brush* b = *i;

		Brush newb0 = *b;
		Brush newb1 = *b;

		newb0.add(news0);
		newb1.add(news1);

		newb0.collapse();
		newb1.collapse();

		newb0.remaptex();
		newb1.remaptex();

		g_edmap.brush.push_back(newb0);
		std::list<Brush>::iterator j = map->brush.rbegin();
		//PruneB(m, &*j);
		if(!PruneB(map, &*j))
		{
			newsel.clear();
			//g_selB.push_back( &*j );
			newsel.push_back( &*j );
		}
#endif
	}
#endif
}
