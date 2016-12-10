

#include "../platform.h"
#include "edmap.h"
#include "../math/triangle.h"

class CutBrushSide
{
public:
	unsigned int diffusem;
	unsigned int specularm;
	unsigned int normalm;
	Plane3f tceq[2];
	std::list<Triangle> frag;
};

class CutBrush
{
public:
	std::list<CutBrushSide> side;
};

extern float g_defrenderpitch;
extern float g_defrenderyaw;
extern int g_1tilewidth;
extern int g_renderframe;

bool AllBeh(Brush* b, Plane3f* p, float epsilon=EPSILON);
void ToCutSide(CutBrushSide* cuts, BrushSide* eds);
void CompileMap(const char* full, EdMap* map);
void ResetView(bool checkupscale);
