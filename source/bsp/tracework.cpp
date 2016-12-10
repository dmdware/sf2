

#include "../save/edmap.h"
#include "tracework.h"
#include "../texture.h"
#include "../sim/order.h"
#include "../utils.h"
#include "../save/edmap.h"


void TraceRay(std::list<Brush>* brushes, TraceWork *tw, Vec3f start, Vec3f end)
{
	tw->clip = end;
	tw->atladder = ecfalse;
	tw->onground = ecfalse;
	tw->trytostep = ecfalse;
	tw->collided = ecfalse;

	TraceJob tj;
	tj.brush = brushes;
	tj.type = TJ_TRACE_RAY;
	tj.vmin = Vec3f(0,0,0);
	tj.vmax = Vec3f(0,0,0);
	tj.extents = Vec3f( 0,0,0);
	tj.maxstep = 0;

	Trace(tw, &tj, start, end, &tw->traceratio, &tw->clip);

	if(tw->collided && tw->trytostep)
	{
		//tj.start = tw->clip;
	}
}

void TraceBox(std::list<Brush>* brushes, TraceWork *tw, Vec3f start, Vec3f end, Vec3f vmin, Vec3f vmax, float maxstep)
{
	tw->clip = end;
	tw->atladder = ecfalse;
	tw->onground = ecfalse;
	tw->trytostep = ecfalse;
	tw->collided = ecfalse;

	TraceJob tj;
	tj.brush = brushes;
	tj.type = TJ_TRACE_BOX;
	tj.vmin = vmin;
	tj.vmax = vmax;
	tj.extents = Vec3f( 
		-vmin.x > vmax.x ? -vmin.x : vmax.x, 
		-vmin.y > vmax.y ? -vmin.y : vmax.y, 
		-vmin.z > vmax.z ? -vmin.z : vmax.z );
	tj.maxstep = maxstep;

	Trace(tw, &tj, start, end, &tw->traceratio, &tw->clip);

	if(tw->collided && tw->trytostep)
	{
		TryToStep(tw, &tj, tw->clip, end, &tw->traceratio, &tw->clip);
	}

//#define ANTIFALL
	
#ifdef ANTIFALL
	tw->stuck = ecfalse;

	tj.start = tw->clip;
	Trace(tw, &tj);

	if(tw->stuck)
		tw->clip = start;
#endif
}

#ifdef BCOLDEBUG
static ecbool debugb = ecfalse;
#endif

//#define EPSILON 0.1f
#define START_RATIO		-1	//-100.0f

void CheckBrush(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip, Brush* b)
{
	//float startratio = -1.0f;
	float startratio = START_RATIO;
	float endratio = 1.0f;
	ecbool startsout = ecfalse;
	ecbool getsout = ecfalse;

	Texture* brushtex = &g_texture[b->texture];

	if(brushtex->passthru)
		return;
	
#ifdef BCOLDEBUG
	int relativebrush = b - brush;

	//ecbool debugb = ecfalse;

	BrushSide* colbs;
#endif

	Vec3f collisionnormal;
	ecbool trytostep = ecfalse;

	for(int i = 0; i < b->nsides; i++)
	{
#if 0
		InfoMess("asd", "co2");
#endif

		BrushSide* s = &b->sides[i];
		Plane3f* p = &s->plane;

		float offset = 0;

		if(tj->type == TJ_TRACE_SPHERE)
			offset = tj->radius;
		
		//float startdistance = Dot(tj->absstart, p->normal) + (p->d + offset);
		//float enddistance = Dot(tj->absend, p->normal) + (p->d + offset);
		float startdistance = Dot(start, p->normal) + (p->d + offset);
		float enddistance = Dot(end, p->normal) + (p->d + offset);

		//float startdistance0 = Dot(start, p->normal) + (p->d);
		//float enddistance0 = Dot(end, p->normal) + (p->d);

		Vec3f voffset(0,0,0);

		if(tj->type == TJ_TRACE_BOX)
		{
#if 0
			InfoMess("asd", "co3");
#endif

			voffset.x = (p->normal.x < 0) ? tj->vmax.x : tj->vmin.x;
			voffset.y = (p->normal.y < 0) ? tj->vmax.y : tj->vmin.y;
			voffset.z = (p->normal.z < 0) ? tj->vmax.z : tj->vmin.z;
			
			startdistance = Dot(start + voffset, p->normal) + p->d;
			enddistance = Dot(end + voffset, p->normal) + p->d;
		}

		if(startdistance > 0)	startsout = ectrue;

		if(enddistance > 0)	getsout = ectrue;

		if(startdistance > 0 && enddistance > 0)
			return;

		//if(startdistance == enddistance)
		//	return;

		if(startdistance <= 0 && enddistance <= 0)
			continue;

		//crosses face
		if(startdistance > enddistance)	//closer to plane inside - advance startratio if more positive
		{
			float ratio1 = (startdistance - EPSILON) / (startdistance - enddistance);

			if(ratio1 > startratio)
			{
				startratio = ratio1;

				//tw->collisionnormal = p->normal;
				collisionnormal = p->normal;
				
#ifdef BCOLDEBUG
				colbs = s;
#endif

				if((start.x != end.x || start.z != end.z) /* && p->normal.y != 1 */ && p->normal.y >= 0.0f)
					//if((tj->start.x != tj->end.x || tj->start.z != tj->end.z))
				{
					//trytostep = ectrue;
					tw->trytostep = ectrue;

					//if(debugb)
					//	InfoMess("asd", "try step");
				}
			
				//tw->trytostep = trytostep ? ectrue : tw->trytostep;

				//if(tw->collisionnormal.y > 0.2f)
				//if(collisionnormal.y > 0.2f)
				//	tw->onground = ectrue;
			}
		}
		//farther out of this plane side - devance endratio if more negative/lesser
		else
		{
			float ratio = (startdistance + EPSILON) / (startdistance - enddistance);

			//collisionnormal = p->normal;

			if(ratio < endratio)
				endratio = ratio;
		}
	}

#ifdef BCOLDEBUG
	if(relativebrush == 462)
		debugb = ectrue;
#endif

	Texture* ptex = &g_texture[b->texture];

	if(ptex->ladder)
		tw->atladder = ectrue;

	if(!startsout)
	{
		if (!getsout) 
		{
			tw->stuck = ectrue;
			*traceratio = 0;
			
			tw->collided = ectrue;
			tw->collisionnormal = collisionnormal;
			//tw->trytostep = trytostep ? ectrue : tw->trytostep;
			
			if(collisionnormal.y > 0.2f)
				tw->onground = ectrue;

			goto reccol;
			return;
		}

		//return;
	}

	if(endratio < startratio)
	{
		//pass
	}

	if(startratio < endratio)
	{
		if(startratio > START_RATIO && startratio < *traceratio)
		//if(startratio > -1)
		{
			if(startratio < 0)
				startratio = 0;
			
#if 0
			g_order.push_back(OrderMarker(colbs->centroid, GetTicks(), 100));

			InfoMess("asd", "co");
#endif

			//g_selB.push_back(b);
			
			//g_applog<<"\t before traceratio:"<<*traceratio<<std::endl;

			*traceratio = startratio;
			
			//g_applog<<"\t after traceratio:"<<*traceratio<<std::endl;
			
			tw->collided = ectrue;
			tw->collisionnormal = collisionnormal;
			//tw->trytostep = trytostep ? ectrue : tw->trytostep;
			
			if(collisionnormal.y > 0.2f)
				tw->onground = ectrue;

			goto reccol;
		}
	}

	return;

reccol:
	;
#if 0	
	g_applog<<"-------"<<std::endl;
	g_applog<<"collision brush:"<<std::endl;

	g_applog<<"traceratio:"<<*traceratio<<std::endl;
	
	g_applog<<"from:"<<start.x<<","<<start.y<<","<<start.z<<std::endl;
	g_applog<<"to:"<<end.x<<","<<end.y<<","<<end.z<<std::endl;

	g_applog<<"trytostep: "<<trytostep<<" "<<tw->trytostep<<std::endl;
	
	startratio = -1.0f;
	endratio = 1.0f;
	startsout = ecfalse;
	getsout = ecfalse;

	for(int i = 0; i < b->nsides; i++)
	{
#if 0
		InfoMess("asd", "co2");
#endif

		BrushSide* s = &b->sides[i];
		Plane3f* p = &s->plane;
		
		g_applog<<std::endl<<"\t collision brush side:"<<std::endl;
		g_applog<<"\t\t plane: "<<p->normal.x<<","<<p->normal.y<<","<<p->normal.z<<":"<<p->d<<std::endl;

		float offset = 0;

		if(tj->type == TJ_TRACE_SPHERE)
			offset = tj->radius;
		
		//float startdistance = Dot(tj->absstart, p->normal) + (p->d + offset);
		//float enddistance = Dot(tj->absend, p->normal) + (p->d + offset);
		float startdistance = Dot(start, p->normal) + (p->d + offset);
		float enddistance = Dot(end, p->normal) + (p->d + offset);
		
		float startdistance0 = Dot(start, p->normal) + (p->d);
		float enddistance0 = Dot(end, p->normal) + (p->d);

		if(tj->type == TJ_TRACE_BOX)
		{
#if 0
			InfoMess("asd", "co3");
#endif
			Vec3f voffset;

			voffset.x = (p->normal.x < 0) ? tj->vmax.x : tj->vmin.x;
			voffset.y = (p->normal.y < 0) ? tj->vmax.y : tj->vmin.y;
			voffset.z = (p->normal.z < 0) ? tj->vmax.z : tj->vmin.z;

			g_applog<<std::endl<<"\t voffset: "<<voffset.x<<","<<voffset.y<<","<<voffset.z<<std::endl;
			
			startdistance = Dot(start + voffset, p->normal) + p->d;
			enddistance = Dot(end + voffset, p->normal) + p->d;
		}

		
		g_applog<<"\t startdistance:"<<startdistance<<std::endl;
		g_applog<<"\t enddistance:"<<enddistance<<std::endl;

		if(startdistance > 0)
		{
			startsout = ectrue;
			g_applog<<"\t starts out"<<std::endl;
		}

		if(enddistance > 0)
		{
			g_applog<<"\t gets out"<<std::endl;
			getsout = ectrue;
		}

		if(startdistance > 0 && enddistance > 0)
		{
			g_applog<<"\t start > 0 and end > 0 outside of brush"<<std::endl;
			break;
		}

		if(startdistance <= 0 && enddistance <= 0)
		{
			g_applog<<"\t start <= 0 and end <= 0 inside of brush"<<std::endl;
			continue;
		}

		if(startdistance > enddistance)
		{
			float ratio1 = (startdistance - EPSILON) / (startdistance - enddistance);
			
			g_applog<<"\t start > end  ratio1 = "<<ratio1<<std::endl;

			if(ratio1 > startratio)
			{
				g_applog<<"\t ratio1 > startratio = "<<ratio1<<std::endl;
				startratio = ratio1;
			}
		}
		else
		{
			float ratio = (startdistance + EPSILON) / (startdistance - enddistance);

			g_applog<<"\t start < end  ratio = "<<ratio<<std::endl;

			if(ratio < endratio)
			{
				g_applog<<"\t ratio < endratio = "<<ratio<<std::endl;
				endratio = ratio;
			}
		}
	}

	
	g_applog<<"-------"<<std::endl;
#endif
}

void Trace(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip)
{
	*traceratio = 1;

	//work goes here
	for(std::list<Brush>::iterator bit=tj->brush->begin(); bit!=tj->brush->end(); bit++)
		CheckBrush(tw, tj, start, end, traceratio, clip, &*bit);

	if(*traceratio == 1)
	{
		*clip = end;
		return;
	}
#if 1
	else if(*traceratio == 0)
	{
		*clip = start;
		return;
	}
#endif
	else
	{
		Vec3f newpos = start + ((end - start) * *traceratio);

		if(tj->type == TJ_TRACE_RAY)
		{
			*clip = newpos;
			return;
		}

		//if(*traceratio > 0.0f)
		{
			Vec3f move = end - newpos;

			float distance = Dot(move, tw->collisionnormal);
		
			Vec3f endpos = end - tw->collisionnormal*distance;
			//Vec3f endpos = tj->end;	// - tw->collisionnormal*distance;

			//if(traceRatio > 0.0f)
			if(endpos != end)
			{
	#if 0
				float subratio = 1;
				Trace(tw, tj, newpos, endpos, &subratio, &newpos);
				*traceratio = subratio * (MAG_VEC3F(endpos - start) / MAG_VEC3F(end - start))
	#else
				Trace(tw, tj, newpos, endpos, traceratio, &newpos);
	#endif
			}
			else
				newpos = endpos;
		}

#if 0
		if(tw->collisionnormal.y > 0.2f || tw->onground)
			tw->onground = ectrue;
		else
			tw->onground = ecfalse;
#endif

		*clip = newpos;
	}
}

void TryToStep(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip)
{
	//Vec3f movedir = Normalize(end - start);
	Vec3f subclip;

	for(float height = 1; height <= tj->maxstep; height += 1)
	//float height = 1;
	{
		tw->collided = ecfalse;
		tw->trytostep = ecfalse;
		
		Vec3f stepstart = Vec3f(start.x, start.y + height, start.z);
		Vec3f stepend = Vec3f(end.x, end.y + height, end.z);

		Trace(tw, tj, stepstart, stepend, traceratio, &subclip);
		
#ifdef BCOLDEBUG
		if(debugb)
		{
			g_applog<<"height: "<<height<<endl;
			g_applog.flush();
		}
#endif

		if(!tw->collided)
		{
			*clip = subclip;

#ifdef BCOLDEBUG
		if(debugb)
		{
			g_applog<<"no col!: "<<height<<" ("<<stepstart.x<<","<<stepstart.z<<")->("<<stepend.x<<","<<stepend.z<<")"<<endl;
			g_applog<<"no col!: "<<height<<" ("<<tj->start.x<<","<<tj->start.z<<")->("<<tj->end.x<<","<<tj->end.z<<")"<<endl;
			g_applog<<"no col!: "<<height<<" ("<<tj->absstart.x<<","<<tj->absstart.z<<")->("<<tj->absend.x<<","<<tj->absend.z<<")"<<endl;
			g_applog.flush();
		}
	if(debugb)
		debugb = ecfalse;
#endif

			return;
		}
	}

	*clip = start;
	
#ifdef BCOLDEBUG
		if(debugb)
		{
			g_applog<<"still col"<<endl;
			g_applog.flush();
		}

	if(debugb)
		debugb = ecfalse;
#endif
}