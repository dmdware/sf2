










#ifndef MATRIX_H
#define MATRIX_H

#include "../platform.h"
#include "vec4f.h"
#include "3dmath.h"

struct Quaternion;
struct Vec4f;

struct PersProjInfo
{
    float FOV;
    float Width;
    float Height;
    float zNear;
    float zFar;
};


#define M(row,col)  m[col*4+row]
//#define M(row,col)  m[row*4+col]

struct Matrix
{
public:
	//	Matrix data, stored in column-major order
	float m[16];

	Matrix();
	Matrix(Vec4f a, Vec4f b, Vec4f c, Vec4f d);
	~Matrix();

	inline void set( const float *matrix )
	{
		memcpy( m, matrix, sizeof( float )*16 );
	}

	inline void reset()
	{
		memset( m, 0, sizeof( float )*16 );
		m[0] = m[5] = m[10] = m[15] = 1;
	}
	
	inline void InitIdentity()
	{
		M(0,0) = 1.0f;
		M(0,1) = 0.0f;
		M(0,2) = 0.0f;
		M(0,3) = 0.0f;
		M(1,0) = 0.0f;
		M(1,1) = 1.0f;
		M(1,2) = 0.0f;
		M(1,3) = 0.0f;
		M(2,0) = 0.0f;
		M(2,1) = 0.0f;
		M(2,2) = 1.0f;
		M(2,3) = 0.0f;
		M(3,0) = 0.0f;
		M(3,1) = 0.0f;
		M(3,2) = 0.0f;
		M(3,3) = 1.0f;
	}

	// constructor from Assimp matrix
	Matrix(const aiMatrix4x4& AssimpMatrix)
	{
		M(0,0) = AssimpMatrix.a1;
		M(0,1) = AssimpMatrix.a2;
		M(0,2) = AssimpMatrix.a3;
		M(0,3) = AssimpMatrix.a4;
		M(1,0) = AssimpMatrix.b1;
		M(1,1) = AssimpMatrix.b2;
		M(1,2) = AssimpMatrix.b3;
		M(1,3) = AssimpMatrix.b4;
		M(2,0) = AssimpMatrix.c1;
		M(2,1) = AssimpMatrix.c2;
		M(2,2) = AssimpMatrix.c3;
		M(2,3) = AssimpMatrix.c4;
		M(3,0) = AssimpMatrix.d1;
		M(3,1) = AssimpMatrix.d2;
		M(3,2) = AssimpMatrix.d3;
		M(3,3) = AssimpMatrix.d4;
	}

	Matrix(const aiMatrix3x3& AssimpMatrix)
	{
		M(0,0) = AssimpMatrix.a1;
		M(0,1) = AssimpMatrix.a2;
		M(0,2) = AssimpMatrix.a3;
		M(0,3) = 0.0f;
		M(1,0) = AssimpMatrix.b1;
		M(1,1) = AssimpMatrix.b2;
		M(1,2) = AssimpMatrix.b3;
		M(1,3) = 0.0f;
		M(2,0) = AssimpMatrix.c1;
		M(2,1) = AssimpMatrix.c2;
		M(2,2) = AssimpMatrix.c3;
		M(2,3) = 0.0f;
		M(3,0) = 0.0f           ;
		M(3,1) = 0.0f           ;
		M(3,2) = 0.0f           ;
		M(3,3) = 1.0f;
	}

	Matrix(float a00, float a01, float a02, float a03,
			 float a10, float a11, float a12, float a13,
			 float a20, float a21, float a22, float a23,
			 float a30, float a31, float a32, float a33)
	{
		M(0,0) = a00;
		M(0,1) = a01;
		M(0,2) = a02;
		M(0,3) = a03;
		M(1,0) = a10;
		M(1,1) = a11;
		M(1,2) = a12;
		M(1,3) = a13;
		M(2,0) = a20;
		M(2,1) = a21;
		M(2,2) = a22;
		M(2,3) = a23;
		M(3,0) = a30;
		M(3,1) = a31;
		M(3,2) = a32;
		M(3,3) = a33;
	}


void initscale(float ScaleX, float ScaleY, float ScaleZ)
{
    M(0,0) = ScaleX; M(0,1) = 0.0f;   M(0,2) = 0.0f;   M(0,3) = 0.0f;
    M(1,0) = 0.0f;   M(1,1) = ScaleY; M(1,2) = 0.0f;   M(1,3) = 0.0f;
    M(2,0) = 0.0f;   M(2,1) = 0.0f;   M(2,2) = ScaleZ; M(2,3) = 0.0f;
    M(3,0) = 0.0f;   M(3,1) = 0.0f;   M(3,2) = 0.0f;   M(3,3) = 1.0f;
}

	void postmult( const Matrix& matrix );
	void postmult2( const Matrix& matrix );

	//	Set the translation of the current matrix. Will erase any prev values.
	void translation( const float *translation );

	//	Set the inverse translation of the current matrix. Will erase any prev values.
	void invtrans( const float *translation );

	void scale( const float *scale );

	//	Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
	void rotrad( const float *angles );

	//	Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
	void rotdeg( const float *angles );

	//	Make a rotation matrix from a quaternion. The 4th row and column are unmodified.
	void rotquat( const Quaternion& quat );

	//	Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	void invrotrad( const float *angles );

	//	Make an inverted rotation matrix from Euler angles. The 4th row and column are unmodified.
	void invrotdeg( const float *angles );

	inline float* get(int row, int col)
	{
		return &m[ row + col*4 ];
	}

	//	Translate a vector by the inverse of the translation part of this matrix.
	void inverseTranslateVect( float *pVect );

	//	Rotate a vector by the inverse of the rotation part of this matrix.
	void inverseRotateVect( float *pVect );

	Matrix& inverse();

	Matrix Transpose() const
	{
		Matrix n;

		for (unsigned int i = 0 ; i < 4 ; i++) {
			for (unsigned int j = 0 ; j < 4 ; j++) {
				n.M(i,j) = M(j,i);
			}
		}

		return n;
	}

	
void InitCameraTransform(const Vec3f& Target, const Vec3f& Up);

void InitPersProjTransform(const PersProjInfo& p);

	void InitTranslationTransform(float x, float y, float z);;

float Determinant() const;
	
	inline Matrix operator*(const Matrix& Right) const
	{
//#define M(row,col)  m[col*4+row]
////#define M(row,col)  m[row*4+col]
#if 1
		Matrix Ret;

		for (unsigned int i = 0 ; i < 4 ; i++) {
			for (unsigned int j = 0 ; j < 4 ; j++) {
				Ret.M(i,j) = M(i,0) * Right.M(0,j) +
							  M(i,1) * Right.M(1,j) +
							  M(i,2) * Right.M(2,j) +
							  M(i,3) * Right.M(3,j);
			}
		}

		return Ret;
#else
		//Matrix Ret = *this;
		//Ret.postmult2(Right);

		//Matrix Ret = Right;
		//Ret.postmult2(*this);

		//Matrix Ret = *this;
		//Ret.postmult(Right);

		//Matrix Ret = Right;
		//Ret.postmult2(*this);

		return Ret;

#endif
#if 0
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

#if 0
	//not skel postmult()

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
#endif
#endif

	}

	Vec4f operator*(const Vec4f& v) const
	{
		Vec4f r;

		r.x = M(0,0)* v.x + M(0,1)* v.y + M(0,2)* v.z + M(0,3)* v.w;
		r.y = M(1,0)* v.x + M(1,1)* v.y + M(1,2)* v.z + M(1,3)* v.w;
		r.z = M(2,0)* v.x + M(2,1)* v.y + M(2,2)* v.z + M(2,3)* v.w;
		r.w = M(3,0)* v.x + M(3,1)* v.y + M(3,2)* v.z + M(3,3)* v.w;

		return r;
	}

	operator const float*() const
	{
		return &(M(0,0));
	}

	Matrix operator*(const float f)
	{
		Matrix m2(*this);

		for(int i=0; i<16; i++)
			m2.m[i] *= f;

		return m2;
	}

	Matrix& operator+=(const Matrix& other)
	{
		for(int i=0; i<16; i++)
			m[i] += other.m[i];

		return *this;
	}
};

#endif
