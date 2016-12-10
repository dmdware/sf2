










#ifndef VEC2F_H
#define VEC2F_H

struct Vec2f
{
public:
	float x, y;

	Vec2f()
	{
		x = y = 0;
	}

	Vec2f(float X, float Y)
	{
		x = X;
		y = Y;
	}


	ecbool operator==(const Vec2f v) const
	{
		if(x == v.x && y == v.y)
			return ectrue;

		return ecfalse;
	}

	ecbool operator!=(const Vec2f v) const
	{
		if(x == v.x && y == v.y)
			return ecfalse;

		return ectrue;
	}

	Vec2f operator+(const Vec2f v) const
	{
		return Vec2f(v.x + x, v.y + y);
	}

	Vec2f operator-(const Vec2f v) const
	{
		return Vec2f(x - v.x, y - v.y);
	}

	Vec2f operator*(const float num) const
	{
		return Vec2f(x * num, y * num);
	}

	Vec2f operator*(const Vec2f v) const
	{
		return Vec2f(x * v.x, y * v.y);
	}

	Vec2f operator/(const float num) const
	{
		return Vec2f(x / num, y / num);
	}
};

#endif
