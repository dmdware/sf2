










#include "matrix.h"
#include "../platform.h"
#include "quaternion.h"
#include "vec4f.h"

Matrix::Matrix()
{
	reset();
}

Matrix::Matrix(Vec4f a, Vec4f b, Vec4f c, Vec4f d)
{
//#define M(row,col)  m[col*4+row]

//#undef M
}

Matrix::~Matrix()
{
}


void Matrix::InitCameraTransform(const Vec3f& Target, const Vec3f& Up)
{
    Vec3f N = Target;
    //N.Normalize();
    N = Normalize(N);
    Vec3f U = Up;
    //U.Normalize();
    U = Normalize(U);
    //U = U.Cross(N);
    U = Cross(U,N);
    //Vec3f V = N.Cross(U);
    Vec3f V = Cross(N,U);

    M(0,0) = U.x;   M(0,1) = U.y;   M(0,2) = U.z;   M(0,3) = 0.0f;
    M(1,0) = V.x;   M(1,1) = V.y;   M(1,2) = V.z;   M(1,3) = 0.0f;
    M(2,0) = N.x;   M(2,1) = N.y;   M(2,2) = N.z;   M(2,3) = 0.0f;
    M(3,0) = 0.0f;  M(3,1) = 0.0f;  M(3,2) = 0.0f;  M(3,3) = 1.0f;
}

void Matrix::InitTranslationTransform(float x, float y, float z)
{
    M(0,0) = 1.0f; M(0,1) = 0.0f; M(0,2) = 0.0f; M(0,3) = x;
    M(1,0) = 0.0f; M(1,1) = 1.0f; M(1,2) = 0.0f; M(1,3) = y;
    M(2,0) = 0.0f; M(2,1) = 0.0f; M(2,2) = 1.0f; M(2,3) = z;
    M(3,0) = 0.0f; M(3,1) = 0.0f; M(3,2) = 0.0f; M(3,3) = 1.0f;
}


void Matrix::InitPersProjTransform(const PersProjInfo& p)
{
    const float ar         = p.Width / p.Height;
    const float zRange     = p.zNear - p.zFar;
    const float tanHalfFOV = tanf((float)DEGTORAD(p.FOV / 2.0f));

    M(0,0) = 1.0f/(tanHalfFOV * ar); M(0,1) = 0.0f;            M(0,2) = 0.0f;            M(0,3) = 0.0;
    M(1,0) = 0.0f;                   M(1,1) = 1.0f/tanHalfFOV; M(1,2) = 0.0f;            M(1,3) = 0.0;
    M(2,0) = 0.0f;                   M(2,1) = 0.0f;            M(2,2) = (-p.zNear - p.zFar)/zRange ; M(2,3) = 2.0f*p.zFar*p.zNear/zRange;
    M(3,0) = 0.0f;                   M(3,1) = 0.0f;            M(3,2) = 1.0f;            M(3,3) = 0.0;
}

float Matrix::Determinant() const
{
	return M(0,0)*M(1,1)*M(2,2)*M(3,3) - M(0,0)*M(1,1)*M(2,3)*M(3,2) + M(0,0)*M(1,2)*M(2,3)*M(3,1) - M(0,0)*M(1,2)*M(2,1)*M(3,3)
		+ M(0,0)*M(1,3)*M(2,1)*M(3,2) - M(0,0)*M(1,3)*M(2,2)*M(3,1) - M(0,1)*M(1,2)*M(2,3)*M(3,0) + M(0,1)*M(1,2)*M(2,0)*M(3,3)
		- M(0,1)*M(1,3)*M(2,0)*M(3,2) + M(0,1)*M(1,3)*M(2,2)*M(3,0) - M(0,1)*M(1,0)*M(2,2)*M(3,3) + M(0,1)*M(1,0)*M(2,3)*M(3,2)
		+ M(0,2)*M(1,3)*M(2,0)*M(3,1) - M(0,2)*M(1,3)*M(2,1)*M(3,0) + M(0,2)*M(1,0)*M(2,1)*M(3,3) - M(0,2)*M(1,0)*M(2,3)*M(3,1)
		+ M(0,2)*M(1,1)*M(2,3)*M(3,0) - M(0,2)*M(1,1)*M(2,0)*M(3,3) - M(0,3)*M(1,0)*M(2,1)*M(3,2) + M(0,3)*M(1,0)*M(2,2)*M(3,1)
		- M(0,3)*M(1,1)*M(2,2)*M(3,0) + M(0,3)*M(1,1)*M(2,0)*M(3,2) - M(0,3)*M(1,2)*M(2,0)*M(3,1) + M(0,3)*M(1,2)*M(2,1)*M(3,0);
}

Matrix& Matrix::inverse()
{
	// Compute the reciprocal determinant
	float det = Determinant();
	if(det == 0.0f)
	{
		// Matrix not invertible. Setting all elements to nan is not really
		// correct in a mathematical sense but it is easy to debug for the
		// programmer.
		/*const float nan = std::numeric_limits<float>::quiet_NaN();
		*this = Matrix(
			nan,nan,nan,nan,
			nan,nan,nan,nan,
			nan,nan,nan,nan,
			nan,nan,nan,nan);*/
        assert(0);
		return *this;
	}

	float invdet = 1.0f / det;

	Matrix res;
	res.M(0,0) = invdet  * (M(1,1) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(1,2) * (M(2,3) * M(3,1) - M(2,1) * M(3,3)) + M(1,3) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)));
	res.M(0,1) = -invdet * (M(0,1) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(0,2) * (M(2,3) * M(3,1) - M(2,1) * M(3,3)) + M(0,3) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)));
	res.M(0,2) = invdet  * (M(0,1) * (M(1,2) * M(3,3) - M(1,3) * M(3,2)) + M(0,2) * (M(1,3) * M(3,1) - M(1,1) * M(3,3)) + M(0,3) * (M(1,1) * M(3,2) - M(1,2) * M(3,1)));
	res.M(0,3) = -invdet * (M(0,1) * (M(1,2) * M(2,3) - M(1,3) * M(2,2)) + M(0,2) * (M(1,3) * M(2,1) - M(1,1) * M(2,3)) + M(0,3) * (M(1,1) * M(2,2) - M(1,2) * M(2,1)));
	res.M(1,0) = -invdet * (M(1,0) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(1,2) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(1,3) * (M(2,0) * M(3,2) - M(2,2) * M(3,0)));
	res.M(1,1) = invdet  * (M(0,0) * (M(2,2) * M(3,3) - M(2,3) * M(3,2)) + M(0,2) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(0,3) * (M(2,0) * M(3,2) - M(2,2) * M(3,0)));
	res.M(1,2) = -invdet * (M(0,0) * (M(1,2) * M(3,3) - M(1,3) * M(3,2)) + M(0,2) * (M(1,3) * M(3,0) - M(1,0) * M(3,3)) + M(0,3) * (M(1,0) * M(3,2) - M(1,2) * M(3,0)));
	res.M(1,3) = invdet  * (M(0,0) * (M(1,2) * M(2,3) - M(1,3) * M(2,2)) + M(0,2) * (M(1,3) * M(2,0) - M(1,0) * M(2,3)) + M(0,3) * (M(1,0) * M(2,2) - M(1,2) * M(2,0)));
	res.M(2,0) = invdet  * (M(1,0) * (M(2,1) * M(3,3) - M(2,3) * M(3,1)) + M(1,1) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(1,3) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(2,1) = -invdet * (M(0,0) * (M(2,1) * M(3,3) - M(2,3) * M(3,1)) + M(0,1) * (M(2,3) * M(3,0) - M(2,0) * M(3,3)) + M(0,3) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(2,2) = invdet  * (M(0,0) * (M(1,1) * M(3,3) - M(1,3) * M(3,1)) + M(0,1) * (M(1,3) * M(3,0) - M(1,0) * M(3,3)) + M(0,3) * (M(1,0) * M(3,1) - M(1,1) * M(3,0)));
	res.M(2,3) = -invdet * (M(0,0) * (M(1,1) * M(2,3) - M(1,3) * M(2,1)) + M(0,1) * (M(1,3) * M(2,0) - M(1,0) * M(2,3)) + M(0,3) * (M(1,0) * M(2,1) - M(1,1) * M(2,0)));
	res.M(3,0) = -invdet * (M(1,0) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)) + M(1,1) * (M(2,2) * M(3,0) - M(2,0) * M(3,2)) + M(1,2) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(3,1) = invdet  * (M(0,0) * (M(2,1) * M(3,2) - M(2,2) * M(3,1)) + M(0,1) * (M(2,2) * M(3,0) - M(2,0) * M(3,2)) + M(0,2) * (M(2,0) * M(3,1) - M(2,1) * M(3,0)));
	res.M(3,2) = -invdet * (M(0,0) * (M(1,1) * M(3,2) - M(1,2) * M(3,1)) + M(0,1) * (M(1,2) * M(3,0) - M(1,0) * M(3,2)) + M(0,2) * (M(1,0) * M(3,1) - M(1,1) * M(3,0)));
	res.M(3,3) = invdet  * (M(0,0) * (M(1,1) * M(2,2) - M(1,2) * M(2,1)) + M(0,1) * (M(1,2) * M(2,0) - M(1,0) * M(2,2)) + M(0,2) * (M(1,0) * M(2,1) - M(1,1) * M(2,0)));
	*this = res;

	return *this;
}

void Matrix::inverseRotateVect( float *pVect )
{
	float vec[3];

	vec[0] = pVect[0]*m[0]+pVect[1]*m[1]+pVect[2]*m[2];
	vec[1] = pVect[0]*m[4]+pVect[1]*m[5]+pVect[2]*m[6];
	vec[2] = pVect[0]*m[8]+pVect[1]*m[9]+pVect[2]*m[10];

	memcpy( pVect, vec, sizeof( float )*3 );
}

void Matrix::inverseTranslateVect( float *pVect )
{
	pVect[0] = pVect[0]-m[12];
	pVect[1] = pVect[1]-m[13];
	pVect[2] = pVect[2]-m[14];
}

//no longer used for light matrix concatenation (and skeletal animation?)
void Matrix::postmult( const Matrix& matrix )
{
	float newMatrix[16];

#if 0

	const float *m1 = m, *m2 = matrix.m;

	newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
	newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
	newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
	newMatrix[3] = 0;

	newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
	newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
	newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
	newMatrix[7] = 0;

	newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
	newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
	newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
	newMatrix[11] = 0;

	newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
	newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
	newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
	newMatrix[15] = 1;

#else

	const float *a = m, *b = matrix.m;

	newMatrix[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3];
	newMatrix[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3];
	newMatrix[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
	newMatrix[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

	newMatrix[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7];
	newMatrix[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7];
	newMatrix[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
	newMatrix[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

	newMatrix[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11];
	newMatrix[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11];
	newMatrix[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
	newMatrix[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

	newMatrix[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15];
	newMatrix[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15];
	newMatrix[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	newMatrix[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

#endif

	set( newMatrix );
}

//used only for light matrix (and skeletal animation?), doesn't work with perspective projection because that is not an affine transformation
void Matrix::postmult2( const Matrix& matrix )
{
	float newMatrix[16];

#if 1

	const float *m1 = m, *m2 = matrix.m;

	newMatrix[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
	newMatrix[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
	newMatrix[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
	newMatrix[3] = 0;

	newMatrix[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
	newMatrix[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
	newMatrix[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
	newMatrix[7] = 0;

	newMatrix[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
	newMatrix[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
	newMatrix[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
	newMatrix[11] = 0;

	newMatrix[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
	newMatrix[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
	newMatrix[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
	newMatrix[15] = 1;

#else

	const float *a = m, *b = matrix.m;

	newMatrix[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3];
	newMatrix[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3];
	newMatrix[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
	newMatrix[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

	newMatrix[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7];
	newMatrix[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7];
	newMatrix[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
	newMatrix[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

	newMatrix[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11];
	newMatrix[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11];
	newMatrix[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
	newMatrix[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

	newMatrix[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15];
	newMatrix[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15];
	newMatrix[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	newMatrix[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

#endif

	set( newMatrix );
}

void Matrix::translation( const float *translation )
{
#if 0
	m[0] = m[5] =  m[10] = m[15] = 1.0;
	m[1] = m[2] = m[3] = m[4] = 0.0;
	m[6] = m[7] = m[8] = m[9] = 0.0;
	m[11] = 0.0;

#elif 1

	m[12] = translation[0];
	m[13] = translation[1];
	m[14] = translation[2];

#elif 0

#define M(row,col)  m[col*4+row]
	//#define M(row,col)  m[row*4+col]
	/*
	// http://stackoverflow.com/questions/13293469/why-does-my-translation-matrix-needs-to-be-transposed
	1, 0, 0, 0
	0, 1, 0, 0
	0, 0, 1, 0
	x, y, z, 1
	*/
	M(0,0) = 1;
	M(0,1) = 0;
	M(0,2) = 0;
	M(0,3) = translation[0];
	M(1,0) = 0;
	M(1,1) = 1;
	M(1,2) = 0;
	M(1,3) = translation[1];
	M(2,0) = 0;
	M(2,1) = 0;
	M(2,2) = 1;
	M(2,3) = translation[2];
	M(3,0) = 0;
	M(3,1) = 0;
	M(3,2) = 0;
	M(3,3) = 1;
#undef M

#endif
}

void Matrix::invtrans( const float *translation )
{
	m[12] = -translation[0];
	m[13] = -translation[1];
	m[14] = -translation[2];
}

void Matrix::scale( const float *scale )
{
	//reset();

	m[0] = scale[0];
	m[5] = scale[1];
	m[10] = scale[2];
	m[15] = 1;
}

void Matrix::rotdeg( const float *angles )
{
	float vec[3];
	vec[0] = ( float )( angles[0]*180.0/M_PI );
	vec[1] = ( float )( angles[1]*180.0/M_PI );
	vec[2] = ( float )( angles[2]*180.0/M_PI );
	rotrad( vec );
}

void Matrix::invrotdeg( const float *angles )
{
	float vec[3];
	vec[0] = ( float )( angles[0]*180.0/M_PI );
	vec[1] = ( float )( angles[1]*180.0/M_PI );
	vec[2] = ( float )( angles[2]*180.0/M_PI );
	invrotrad( vec );
}

void Matrix::rotrad( const float *angles )
{
	double cr = cos( angles[0] );
	double sr = sin( angles[0] );
	double cp = cos( angles[1] );
	double sp = sin( angles[1] );
	double cy = cos( angles[2] );
	double sy = sin( angles[2] );

	m[0] = ( float )( cp*cy );
	m[1] = ( float )( cp*sy );
	m[2] = ( float )( -sp );

	double srsp = sr*sp;
	double crsp = cr*sp;

	m[4] = ( float )( srsp*cy-cr*sy );
	m[5] = ( float )( srsp*sy+cr*cy );
	m[6] = ( float )( sr*cp );

	m[8] = ( float )( crsp*cy+sr*sy );
	m[9] = ( float )( crsp*sy-sr*cy );
	m[10] = ( float )( cr*cp );
}

void Matrix::invrotrad( const float *angles )
{
	double cr = cos( angles[0] );
	double sr = sin( angles[0] );
	double cp = cos( angles[1] );
	double sp = sin( angles[1] );
	double cy = cos( angles[2] );
	double sy = sin( angles[2] );

	m[0] = ( float )( cp*cy );
	m[4] = ( float )( cp*sy );
	m[8] = ( float )( -sp );

	double srsp = sr*sp;
	double crsp = cr*sp;

	m[1] = ( float )( srsp*cy-cr*sy );
	m[5] = ( float )( srsp*sy+cr*cy );
	m[9] = ( float )( sr*cp );

	m[2] = ( float )( crsp*cy+sr*sy );
	m[6] = ( float )( crsp*sy-sr*cy );
	m[10] = ( float )( cr*cp );
}

void Matrix::rotquat( const Quaternion& quat )
{
	m[0] = ( float )( 1.0 - 2.0*quat[1]*quat[1] - 2.0*quat[2]*quat[2] );
	m[1] = ( float )( 2.0*quat[0]*quat[1] + 2.0*quat[3]*quat[2] );
	m[2] = ( float )( 2.0*quat[0]*quat[2] - 2.0*quat[3]*quat[1] );

	m[4] = ( float )( 2.0*quat[0]*quat[1] - 2.0*quat[3]*quat[2] );
	m[5] = ( float )( 1.0 - 2.0*quat[0]*quat[0] - 2.0*quat[2]*quat[2] );
	m[6] = ( float )( 2.0*quat[1]*quat[2] + 2.0*quat[3]*quat[0] );

	m[8] = ( float )( 2.0*quat[0]*quat[2] + 2.0*quat[3]*quat[1] );
	m[9] = ( float )( 2.0*quat[1]*quat[2] - 2.0*quat[3]*quat[0] );
	m[10] = ( float )( 1.0 - 2.0*quat[0]*quat[0] - 2.0*quat[1]*quat[1] );
}

