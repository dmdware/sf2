

#ifndef LEAF_H
#define LEAF_H

#include "brush.h"

struct Leaf
{
public:
	std::list<int> planes;
	Brush brush;
};

#endif