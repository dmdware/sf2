

#include "../platform.h"
#include "brush.h"
#include "../math/plane3f.h"
#include "../math/3dmath.h"
#include "../math/line.h"
#include "../math/polygon.h"
#include "../utils.h"
#include "../sim/map.h"
#include "../save/edmap.h"

Brush& Brush::operator=(const Brush& original)
{
	/*
	int nsides;
	BrushSide* sides;
	*/

	//g_log<<"copy edbrush nsides="<<original.nsides<<endl;
	//g_log.flush();

	if(sides)
	{
		delete [] sides;
		sides = NULL;
	}

	if(sharedv)
	{
		delete [] sharedv;
		sharedv = NULL;
	}

	nsides = original.nsides;
	sides = new BrushSide[nsides];
	for(int i=0; i<nsides; i++)
		sides[i] = original.sides[i];
	nsharedv = original.nsharedv;
	sharedv = new Vec3f[nsharedv];
	for(int i=0; i<nsharedv; i++)
		sharedv[i] = original.sharedv[i];

	texture = original.texture;
	
	if(door)
	{
		delete door;
		door = NULL;
	}

	if(original.door)
	{
		door = new EdDoor();
		*door = *original.door;
	}

	return *this;
}

Brush::Brush(const Brush& original)
{
	sides = NULL;
	nsides = 0;
	sharedv = NULL;
	nsharedv = 0;
	texture = 0;
	door = NULL;
	*this = original;
}

Brush::Brush()
{
	sides = NULL;
	nsides = 0;
	sharedv = NULL;
	nsharedv = 0;
	texture = 0;
	door = NULL;

	//MessageBox(g_hWnd, "ed b constr", "asd", NULL);
}

Brush::~Brush()
{
#if 0
	g_log<<"~Brush"<<endl;
#endif

	if(sides)
	{
		delete [] sides;
		sides = NULL;
	}

	if(sharedv)
	{
		delete [] sharedv;
		sharedv = NULL;
	}

	nsharedv = 0;
	nsides = 0;

	if(door)
	{
		delete door;
		door = NULL;
	}
}

//#define SELECT_DEBUG

#ifdef SELECT_DEBUG
Brush* g_debugb = NULL;
#endif

Vec3f Brush::traceray(Vec3f line[])
{
	float startRatio = -1.0f;
	float endRatio = 1.0f;
	Vec3f intersection = line[1];

	for(int i=0; i<nsides; i++)
	{
		BrushSide* s = &sides[i];

		float startD = Dot(line[0], s->plane.normal) + s->plane.d;
		float endD = Dot(line[1], s->plane.normal) + s->plane.d;

#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			g_log<<"side "<<i<<endl;
			g_log<<"startD="<<startD<<", endD="<<endD<<endl;
			g_log.flush();
		}

		if(g_debugb == this)
		{
			g_log<<"traceray plane=("<<s->plane.normal.x<<","<<s->plane.normal.y<<","<<s->plane.normal.z<<"d="<<s->plane.d<<") startD="<<startD<<" endD="<<endD<<endl;
			g_log.flush();
		}
#endif

		if(startD > 0 && endD > 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD > 0 && endD > 0"<<endl;
				g_log.flush();
			}
#endif

			return line[1];
		}

		if(startD <= 0 && endD <= 0)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD <= 0 && endD <= 0"<<endl;
				g_log.flush();
			}
#endif

			continue;
		}

		if(startD > endD)
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"startD > endD"<<endl;
				g_log.flush();
			}
#endif

			// This gets a ratio from our starting point to the approximate collision spot
			float ratio1 = (startD - EPSILON) / (startD - endD);
			
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"ratio1 ="<<ratio1<<endl;
				g_log.flush();
			}
#endif

			if(ratio1 > startRatio)
			{
				startRatio = ratio1;
				
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"ratio1 > startRatio == "<<startRatio<<endl;
					g_log.flush();
				}
#endif
			}
		}
		else
		{
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"else startD <= endD"<<endl;
				g_log.flush();
			}
#endif

			float ratio = (startD + EPSILON) / (startD - endD);
			
#ifdef SELECT_DEBUG
			if(g_debugb == this)
			{
				g_log<<"ratio ="<<ratio<<endl;
				g_log.flush();
			}
#endif

			if(ratio < endRatio)
			{
				endRatio = ratio;
				
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"ratio < endRatio == "<<endRatio<<endl;
					g_log.flush();
				}
#endif
			}
		}
	}

	if(startRatio < endRatio)
	{
#ifdef SELECT_DEBUG
		if(g_debugb == this)
		{
			g_log<<"startRatio ("<<startRatio<<") < endRatio ("<<endRatio<<")"<<endl;
			g_log.flush();
		}
#endif

		if(startRatio > -1)
		{
#ifdef SELECT_DEBUG
				if(g_debugb == this)
				{
					g_log<<"startRatio > -1"<<endl;
					g_log.flush();
				}
#endif

			if(startRatio < 0)
				startRatio = 0;
			
#ifdef SELECT_DEBUG
			if(g_debugb == NULL)
				g_debugb = this;
#endif

			return line[0] + (line[1]-line[0]) * startRatio;
		}
	}

	return line[1];
}

void Brush::prunev(ecbool* invalidv)
{
	for(int i=0; i<nsides; i++)
	{
		BrushSide* s = &sides[i];

		for(int j=0; j<nsharedv; j++)
		{
			if(!PointOnOrBehindPlane(sharedv[j], s->plane))
				invalidv[j] = ectrue;
		}
	}
}

void Brush::moveto(Vec3f newp)
{
	Vec3f currp;

	for(int i=0; i<nsides; i++)
	{
		BrushSide* s = &sides[i];
		currp = currp + s->centroid;
	}

	currp = currp / (float)nsides;

	Vec3f delta = newp - currp;

	for(int i=0; i<nsides; i++)
	{
		BrushSide* s = &sides[i];

		Vec3f pop = PointOnPlane(s->plane);
		pop = pop + delta;
		s->plane.d = PlaneDistance(s->plane.normal, pop);

		for(int j=0; j<2; j++)
		{
			pop = PointOnPlane(s->tceq[j]);
			pop = pop + delta;
			s->tceq[j].d = PlaneDistance(s->tceq[j].normal, pop);
		}
	}

	//collapse();
	colshv();
	colva();
	coloutl();
	remaptex();
}

void Brush::add(BrushSide b)
{
#if 0
	g_log<<"addside before: "<<endl;
	for(int vertindex = 0; vertindex < b.drawva.numverts; vertindex++)
	{
		Vec3f vert = b.drawva.vertices[vertindex];
		g_log<<"\taddvert: "<<vert.x<<","<<vert.y<<","<<vert.z<<endl;
	}
#endif

	BrushSide* newsides = new BrushSide[nsides + 1];
	
	if(nsides > 0)
	{
		//memcpy(newsides, sides, sizeof(BrushSide)*nsides);
		for(int i=0; i<nsides; i++)
			newsides[i] = sides[i];
		delete [] sides;
	}
	
	newsides[nsides] = b;
	sides = newsides;
	nsides ++;
	
#if 0
	g_log<<"addside after: "<<endl;
	for(int vertindex = 0; vertindex < b.drawva.numverts; vertindex++)
	{
		Vec3f vert = b.drawva.vertices[vertindex];
		g_log<<"\taddvert: "<<vert.x<<","<<vert.y<<","<<vert.z<<endl;
	}
#endif
}


void Brush::setsides(int nsides, BrushSide* sides)
{
	if(sides)
	{
		delete [] sides;
		sides = NULL;
		nsides = 0;
	}

	for(int i=0; i<nsides; i++)
		add(sides[i]);
}

void Brush::getsides(int* nsides, BrushSide** sides)
{
	*nsides = nsides;

	if(*sides)
	{
		delete [] *sides;
		*sides = NULL;
	}

	if(nsides > 0)
	{
		*sides = new BrushSide[nsides];

		for(int i=0; i<nsides; i++)
		{
			(*sides)[i] = sides[i];
		}
	}
}

//#define REMOVESIDE_DEBUG

void Brush::removeside(int i)
{
#ifdef REMOVESIDE_DEBUG
	g_log<<"remove side 1 "<<i<<endl;
	g_log.flush();
#endif

	BrushSide* newsides = new BrushSide[nsides-1];

#if 0
	memcpy(&newsides[0], &sides[0], sizeof(BrushSide)*i);
	memcpy(&newsides[i], &sides[i+1], sizeof(BrushSide)*(nsides-i-1));

	nsides --;
	sides = newsides;
#endif
	
#ifdef REMOVESIDE_DEBUG
	g_log<<"remove side 2 "<<i<<endl;
	g_log.flush();
#endif

	for(int j=0; j<i; j++)
		newsides[j] = sides[j];
	
#ifdef REMOVESIDE_DEBUG
	g_log<<"remove side 3 "<<i<<endl;
	g_log.flush();
#endif
		
	for(int j=i+1; j<nsides; j++)
		newsides[j-1] = sides[j];
	
#ifdef REMOVESIDE_DEBUG
	g_log<<"remove side 4 "<<i<<endl;
	g_log.flush();
#endif

	nsides --;

	delete [] sides;

	sides = newsides;

#ifdef REMOVESIDE_DEBUG
	g_log<<"removed side "<<i<<endl;
	g_log.flush();
#endif
}

void Brush::remaptex()
{
	for(int i=0; i<nsides; i++)
		sides[i].remaptex();
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float radius, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-radius, 0, 0), Vec3f(-1, 0, 0));	//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(radius, 0, 0), Vec3f(1, 0, 0));	//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -radius), Vec3f(0, 0, -1));	//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, radius), Vec3f(0, 0, 1));	//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const float hwx, const float hwz, const float height)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, height, 0), Vec3f(0, 1, 0));	//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, 0, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(-hwx, 0, 0), Vec3f(-1, 0, 0));		//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(hwx, 0, 0), Vec3f(1, 0, 0));		//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, -hwz), Vec3f(0, 0, -1));		//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, hwz), Vec3f(0, 0, 1));		//back
}

void MakeHull(Vec3f* norms, float* ds, const Vec3f pos, const Vec3f vmin, const Vec3f vmax)
{
	MakePlane(&norms[0], &ds[0], pos + Vec3f(0, vmax.y, 0), Vec3f(0, 1, 0));		//up
	MakePlane(&norms[1], &ds[1], pos + Vec3f(0, vmin.y, 0), Vec3f(0, -1, 0));		//down
	MakePlane(&norms[2], &ds[2], pos + Vec3f(vmin.x, 0, 0), Vec3f(-1, 0, 0));		//left
	MakePlane(&norms[3], &ds[3], pos + Vec3f(vmax.x, 0, 0), Vec3f(1, 0, 0));		//right
	MakePlane(&norms[4], &ds[4], pos + Vec3f(0, 0, vmin.z), Vec3f(0, 0, -1));		//front
	MakePlane(&norms[5], &ds[5], pos + Vec3f(0, 0, vmax.z), Vec3f(0, 0, 1));		//back
}

ecbool HullsIntersect(Vec3f* hull1norms, float* hull1dist, int hull1planes, Vec3f* hull2norms, float* hull2dist, int hull2planes)
{
	return ecfalse;
}

// line intersects convex hull?
ecbool LineInterHull(const Vec3f* line, Plane3f* planes, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
    {
		Vec3f inter;
        if(LineInterPlane(line, planes[i].normal, -planes[i].d, &inter))
        {
			ecbool allin = ectrue;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(inter, planes[j]))
				{
					allin = ecfalse;
					break;
				}
			}
			if(allin)
			{
				return ectrue;
			}
        }
    }

    return ecfalse;
}

// line intersects convex hull?
ecbool LineInterHull(const Vec3f* line, const Vec3f* norms, const float* ds, const int numplanes)
{
	for(int i=0; i<numplanes; i++)
    {
		Vec3f inter;
        if(LineInterPlane(line, norms[i], -ds[i], &inter))
        {
			ecbool allin = ectrue;
			for(int j=0; j<numplanes; j++)
			{
				if(i == j)
					continue;

				if(!PointOnOrBehindPlane(inter, norms[j], ds[j]))
				{
					allin = ecfalse;
					break;
				}
			}
			if(allin)
			{
				return ectrue;
			}
        }
    }

    return ecfalse;
}