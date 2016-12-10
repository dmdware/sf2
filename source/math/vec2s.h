










#ifndef VEC2S_H
#define VEC2S_H

// byte-align structures
#pragma pack(push, 1)

struct Vec2s
{
public:
	short x, y;

	Vec2s()
	{
		x = y = 0;
	}

	Vec2s(short X, short Y)
	{
		x = X;
		y = Y;
	}

	Vec2s(const short* values)
	{
		set(values);
	}

	ecbool operator==(const Vec2s v) const
	{
		if(x == v.x && y == v.y)
			return ectrue;

		return ecfalse;
	}

	ecbool operator!=(const Vec2s v) const
	{
		if(x == v.x && y == v.y)
			return ecfalse;

		return ectrue;
	}

	Vec2s operator+(const Vec2s v) const
	{
		return Vec2s(v.x + x, v.y + y);
	}

	Vec2s operator-(const Vec2s v) const
	{
		return Vec2s(x - v.x, y - v.y);
	}

	Vec2s operator*(const short num) const
	{
		return Vec2s(x * num, y * num);
	}

	Vec2s operator*(const Vec2s v) const
	{
		return Vec2s(x * v.x, y * v.y);
	}

	Vec2s operator/(const short num) const
	{
		return Vec2s(x / num, y / num);
	}

	inline void set(const short* values)
	{
		x = values[0];
		y = values[1];
	}

	short& operator[](unsigned char in)
	{
		return *(short*)(((char*)this)+in);
	}
};

// Default alignment
#pragma pack(pop)

#endif
