













#ifndef HINT_H
#define HINT_H

#include "../platform.h"

struct Hint
{
public:
	std::string message;
	std::string graphic;
	int gwidth;
	int gheight;
    
	Hint();
	void reset();
};

extern Hint g_lasthint;

#endif