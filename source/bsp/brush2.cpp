

#include "brush2.h"
#include "../platform.h"


void Brush2_init(Brush2* b)
{
	b->nsides = 0;
	b->sides = NULL;
}

void Brush2_free(Brush2* b)
{
	BrushSide2* s;
	BrushSide2* se;

	s = b->sides;
	se = b->sides + b->nsides;

	for(; s<se; ++s)
		BrushSide2_free(s);

	free(b->sides);
	b->sides = NULL;
	b->nsides = 0;
}

void Brush2_add(Brush2* b, BrushSide2* s)
{
	BrushSide2* news;

	news = (BrushSide2*)malloc( sizeof(BrushSide2) * (b->nsides+1) );
	
	memcpy(news, b->sides, sizeof(BrushSide2) * b->nsides);
	memcpy(news + b->nsides, s, sizeof(BrushSide2));
	free(b->sides);
	b->sides = news;
	++b->nsides;
}

void Brush2_rem(Brush2* b, int sin)
{
	BrushSide2* news;

	news = (BrushSide2*)malloc( sizeof(BrushSide2) * (b->nsides-1) );
	
	memcpy(news, b->sides, sizeof(BrushSide2) * sin);
	memcpy(news + sin, b->sides + sin + 1, sizeof(BrushSide2) * (b->nsides - sin - 1));
	free(b->sides);
	b->sides = news;
	--b->nsides;
}

void BrushSide2_init(BrushSide2* s)
{
	s->plane = -1;
}

void BrushSide2_free(BrushSide2* s)
{
	s->plane = -1;
}