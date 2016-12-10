










#include "../platform.h"
#include "matrix.h"
#include "vec4f.h"
#include "vec3f.h"

Vec4f::Vec4f(const Vec3f V, const float W)
{
	x = V.x;
	y = V.y;
	z = V.z;
	w = W;
}

void Vec4f::transform( const Matrix& m )
{
	double vector[4];
	const float *matrix = m.m;

#if 1	//w is already 1 for position coordinates
	vector[0] = x*matrix[0]+y*matrix[4]+z*matrix[8]+matrix[12];
	vector[1] = x*matrix[1]+y*matrix[5]+z*matrix[9]+matrix[13];
	vector[2] = x*matrix[2]+y*matrix[6]+z*matrix[10]+matrix[14];
	vector[3] = x*matrix[3]+y*matrix[7]+z*matrix[11]+matrix[15];
#else
	vector[0] = x*matrix[0]+y*matrix[4]+z*matrix[8]+matrix[12]*w;
	vector[1] = x*matrix[1]+y*matrix[5]+z*matrix[9]+matrix[13]*w;
	vector[2] = x*matrix[2]+y*matrix[6]+z*matrix[10]+matrix[14]*w;
	vector[3] = x*matrix[3]+y*matrix[7]+z*matrix[11]+matrix[15]*w;
#endif

	x = vector[0];
	y = vector[1];
	z = vector[2];
	w = vector[3];
}

void Vec4f::transform3( const Matrix& m )
{
	double vector[3];
	const float *matrix = m.m;

	vector[0] = x*matrix[0]+y*matrix[4]+z*matrix[8];
	vector[1] = x*matrix[1]+y*matrix[5]+z*matrix[9];
	vector[2] = x*matrix[2]+y*matrix[6]+z*matrix[10];

	x = vector[0];
	y = vector[1];
	z = vector[2];
}
