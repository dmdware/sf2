
#ifndef CAMERA_H
#define CAMERA_H

#include "vec3f.h"
#include "quaternion.h"
#include "fixmath.h"

struct Camera 
{
	Vec3i pos;					
	Vec3i view;			
	Vec3i up;		
	Vec3i strafe;		
	Vec3i vel;
	Vec3i orientv;	// pitch, yaw, roll
	Quaternion orientq;	//orientation quaternion
	ecbool grounded;
};

typedef struct Camera Camera;

void Cam_lookpos(Vec3f *v, Camera *c);

extern Camera g_cam;

#endif