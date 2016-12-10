










#ifndef VEC2UC_H
#define VEC2UC_H

struct Vec2uc
{
public:
	unsigned char x, y;

	Vec2uc()
	{
		x = y = 0;
	}

	Vec2uc(unsigned char X, unsigned char Y)
	{
		x = X;
		y = Y;
	}

	Vec2uc(const unsigned char* values)
	{
		set(values);
	}

	ecbool operator==(const Vec2uc v) const
	{
		if(x == v.x && y == v.y)
			return ectrue;

		return ecfalse;
	}

	ecbool operator!=(const Vec2uc v) const
	{
		if(x == v.x && y == v.y)
			return ecfalse;

		return ectrue;
	}

	Vec2uc operator+(const Vec2uc v) const
	{
		return Vec2uc(v.x + x, v.y + y);
	}

	Vec2uc operator-(const Vec2uc v) const
	{
		return Vec2uc(x - v.x, y - v.y);
	}

	Vec2uc operator*(const unsigned char num) const
	{
		return Vec2uc(x * num, y * num);
	}

	Vec2uc operator*(const Vec2uc v) const
	{
		return Vec2uc(x * v.x, y * v.y);
	}

	Vec2uc operator/(const unsigned char num) const
	{
		return Vec2uc(x / num, y / num);
	}

	inline void set(const unsigned char* values)
	{
		x = values[0];
		y = values[1];
	}
};

#endif
