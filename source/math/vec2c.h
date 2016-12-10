










#ifndef VEC2C_H
#define VEC2C_H

struct Vec2c
{
public:
	signed char x, y;

	Vec2c()
	{
		x = y = 0;
	}

	Vec2c(signed char X, signed char Y)
	{
		x = X;
		y = Y;
	}

	Vec2c(const signed char* values)
	{
		set(values);
	}

	ecbool operator==(const Vec2c v) const
	{
		if(x == v.x && y == v.y)
			return ectrue;

		return ecfalse;
	}

	ecbool operator!=(const Vec2c v) const
	{
		if(x == v.x && y == v.y)
			return ecfalse;

		return ectrue;
	}

	Vec2c operator+(const Vec2c v) const
	{
		return Vec2c(v.x + x, v.y + y);
	}

	Vec2c operator-(const Vec2c v) const
	{
		return Vec2c(x - v.x, y - v.y);
	}

	Vec2c operator*(const signed char num) const
	{
		return Vec2c(x * num, y * num);
	}

	Vec2c operator*(const Vec2c v) const
	{
		return Vec2c(x * v.x, y * v.y);
	}

	Vec2c operator/(const signed char num) const
	{
		return Vec2c(x / num, y / num);
	}

	inline void set(const signed char* values)
	{
		x = values[0];
		y = values[1];
	}
};

#endif
