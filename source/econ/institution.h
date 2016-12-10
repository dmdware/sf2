

#ifndef INSTITUTION_H
#define INSTITUTION_H

#include "../sim/resources.h"

struct Institution
{
public:
	int global[RESOURCES];
	int local[RESOURCES];
	float color[4];
};

#define INST_STATE		0
#define INST_FIRM		1
#define INST_TYPES		2

extern const char* INST_STR[INST_TYPES];

//every 4th institution/player is a state, including index 0
#define FIRMSPERSTATE	3

#endif