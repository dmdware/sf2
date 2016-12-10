

#ifndef OCTREE_H
#define OCTREE_H

#include "../platform.h"
#include "../bsp/brush.h"

struct OcBrushRef
{
public:
	int brushindex;
	Brush* pbrush;
};

#define OCNODE_TOP_LEFT_FRONT		0
#define OCNODE_TOP_LEFT_BACK		1
#define OCNODE_TOP_RIGHT_BACK		2
#define OCNODE_TOP_RIGHT_FRONT		3
#define OCNODE_BOTTOM_LEFT_FRONT	4
#define OCNODE_BOTTOM_LEFT_BACK		5
#define OCNODE_BOTTOM_RIGHT_BACK	6
#define OCNODE_BOTTOM_RIGHT_FRONT	7

struct OcNode
{
public:
	std::list<OcBrushRef> brushes;
	OcNode *child[8];

	OcNode();
	~OcNode();
};

void ConstructOc(OcNode **ochead, std::list<Brush> &brushes);

#endif