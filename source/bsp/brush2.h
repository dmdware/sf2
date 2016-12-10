

#ifndef BRUSH2_H
#define BRUSH2_H

typedef struct BrushSide2
{
	int plane;
};

typedef struct Brush2
{
	int nsides;
	BrushSide2* sides;
};

void Brush2_init(Brush2* b);
void Brush2_free(Brush2* b);

void Brush2_add(Brush2* b, BrushSide2* s);
void Brush2_rem(Brush2* b, int sin);

void BrushSide2_init(BrushSide2* s);
void BrushSide2_free(BrushSide2* s);

#endif