










#ifndef VEC4F_H
#define VEC4F_H

struct Vec3f;

struct Matrix;

struct Vec4f
{
public:
	float x, y, z, w;

	Vec4f() {}

	Vec4f(float X, float Y, float Z, float W)
	{
		x = X;
		y = Y;
		z = Z;
		w = W;
	}

	Vec4f(const float* values)
	{
		set(values);
	}

	Vec4f(const Vec3f V, const float W);

	ecbool operator==(const Vec4f v)
	{
		if(x == v.x && y == v.y && z == v.z && w == v.w)
			return ectrue;

		return ecfalse;
	}

	ecbool operator!=(const Vec4f v)
	{
		if(x == v.x && y == v.y && z == v.z && w == v.w)
			return ecfalse;

		return ectrue;
	}

	Vec4f operator+(const Vec4f v)
	{
		return Vec4f(v.x + x, v.y + y, v.z + z, v.w + w);
	}

	Vec4f operator-(const Vec4f v)
	{
		return Vec4f(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	Vec4f operator*(const float num)
	{
		return Vec4f(x * num, y * num, z * num, w);
	}

	Vec4f operator*(const Vec4f v)
	{
		return Vec4f(x * v.x, y * v.y, z * v.z, w);
	}

	Vec4f operator/(const float num)
	{
		return Vec4f(x / num, y / num, z / num, w);
	}

	inline void set(const float* values)
	{
		x = values[0];
		y = values[1];
		z = values[2];
		w = values[3];
	}

	void transform(const Matrix& m);
	void transform3(const Matrix& m);
};

#endif
