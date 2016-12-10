
#ifndef TRACEWORK_H
#define TRACEWORK_H

#include "../math/vec3f.h"
#include "brush.h"

#define TJ_TRACE_BOX		0
#define TJ_TRACE_SPHERE		1
#define TJ_TRACE_RAY		2

struct TraceJob	//assignment of job
{
public:
	std::list<Brush>* brush;
	int type;
	Vec3f vmin;
	Vec3f vmax;
	float radius;
	Vec3f extents;
	float maxstep;
};

struct TraceWork	//results
{
public:
	Vec3f clip;
	ecbool onground;
	ecbool atladder;
	ecbool stuck;
	ecbool trytostep;
	ecbool collided;
	/*
	traceratio: overwritten based on sub-start and sub-end in Trace, 
	don't use outside of trace functions for now, use clip instead for result.
	and it doesn't express sliding or stepping up.
	*/
	float traceratio;
	Vec3f collisionnormal;
};

void TraceRay(std::list<Brush>* brushes, TraceWork *tw, Vec3f start, Vec3f end);
void TraceBox(std::list<Brush>* brushes, TraceWork *tw, Vec3f start, Vec3f end, Vec3f vmin, Vec3f vmax, float maxstep);
void CheckBrush(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip, Brush* b);
void Trace(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip);
void TryToStep(TraceWork* tw, TraceJob* tj, Vec3f start, Vec3f end, float* traceratio, Vec3f* clip);

#endif	//TRACEWORK_H