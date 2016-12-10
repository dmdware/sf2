













#ifndef EFFECT_H
#define EFFECT_H

#include "../platform.h"
#include "condition.h"

#define EFFECT_NONE				0
#define EFFECT_ENABTRIGGER		1
#define EFFECT_DISABTRIGGER		2
#define EFFECT_MESSAGE			3
#define EFFECT_VICTORY			4
#define EFFECT_LOSS				5
#define EFFECT_MESSAGEIMG		6
#define EFFECT_RESETCONDITIONS	7
#define EFFECT_EXECUTETRIGGER	8

struct Trigger;

struct Effect
{
public:
	char name[CONDITION_LEN+1];
	int type;
	Trigger* trigger;
	std::string textval;
	int imgdw;
	int imgdh;
    
	Effect();
};


ecbool UniqueEffect(const char* name);
Effect* GetChosenEffect();

#endif