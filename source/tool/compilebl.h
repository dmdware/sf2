

#ifndef COMPILEBL_H
#define COMPILEBL_H

#include "../math/vec2i.h"
#include "../math/vec2f.h"
#include "../platform.h"
#include "../texture.h"
#include "../save/edmap.h"
#include "../save/modelholder.h"

#define BUILDINGM_VERSION                1.0f

#define TAG_BUILDINGM                {'D', 'M', 'D', 'M', 'L'}

struct TexFitInfo
{
public:
	Vec2i tiletimes;
	Vec2f newdim;
	Vec2i bounds[2];        //duplicate of TexFitRow member, needed for accessing directly without iterating rows

	TexFitInfo();
};

struct TexFit
{
public:
	unsigned int texindex;
	Vec2i bounds[2];
};

struct TexFitRow
{
public:
	std::list<TexFit> fits;
	Vec2i bounds[2];

	TexFitRow();
};

struct EdBuilding;

//void CompileModel(const char* fullfile, EdMap* map, std::list<ModelHolder> &modelholders);
int Max2Pow(int lowerbound);
int Max2Pow32(int lowerbound);

#endif
