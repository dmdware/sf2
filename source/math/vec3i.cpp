










#include "../platform.h"
#include "vec3i.h"
#include "matrix.h"
#include "physics.h"
#include "fixmath.h"
#include "../utils.h"

void Vec3i::transform( const Matrix& m )
{
	double vector[4];
	const float *matrix = m.m;

	vector[0] = x*matrix[0]+y*matrix[4]+z*matrix[8]+matrix[12];
	vector[1] = x*matrix[1]+y*matrix[5]+z*matrix[9]+matrix[13];
	vector[2] = x*matrix[2]+y*matrix[6]+z*matrix[10]+matrix[14];
	vector[3] = x*matrix[3]+y*matrix[7]+z*matrix[11]+matrix[15];

	x = vector[0];
	y = vector[1];
	z = vector[2];
}

void Vec3i::transform3( const Matrix& m )
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

ecbool Close(Vec3i a, Vec3i b)
{
	if(iabs(a.x - b.x) <= CLOSE_EPSILON && iabs(a.y - b.y) <= CLOSE_EPSILON && iabs(a.z - b.z) <= CLOSE_EPSILON)
		return ectrue;

	return ecfalse;
}
