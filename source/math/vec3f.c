










#include "vec3f.h"

void Vec3f_transform(Vec3f *v, const float *m)
{
	float vector[4];

	vector[0] = v->x*m[0]+v->y*m[4]+v->z*m[8]+m[12];
	vector[1] = v->x*m[1]+v->y*m[5]+v->z*m[9]+m[13];
	vector[2] = v->x*m[2]+v->y*m[6]+v->z*m[10]+m[14];
	vector[3] = v->x*m[3]+v->y*m[7]+v->z*m[11]+m[15];

	v->x = vector[0];
	v->y = vector[1];
	v->z = vector[2];
}

void Vec3f_transform3(Vec3f *v, const float *m)
{
	float vector[3];

	vector[0] = v->x*m[0]+v->y*m[4]+v->z*m[8];
	vector[1] = v->x*m[1]+v->y*m[5]+v->z*m[9];
	vector[2] = v->x*m[2]+v->y*m[6]+v->z*m[10];

	v->x = vector[0];
	v->y = vector[1];
	v->z = vector[2];
}