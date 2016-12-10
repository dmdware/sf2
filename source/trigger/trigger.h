













#ifndef TRIGGER_H
#define TRIGGER_H

#include "../platform.h"
#include "condition.h"
#include "effect.h"

struct Trigger
{
public:
	char name[CONDITION_LEN+1];
	ecbool enabled;
    
	std::vector<Condition> conditions;
	std::vector<Effect> effects;
    
	Trigger* prev;
	Trigger* next;
    
	Trigger();
	ecbool checkallmet();	//returns ectrue if map switched
	ecbool execute();	//returns ectrue if map switched
	void resetconditions();
};

//typedef Trigger* pTrigger;
extern Trigger* g_scripthead;


void Click_MessageR(int res);
int CountTriggers();
int TriggerID(Trigger* trigger);
void FreeScript();
Trigger* GetScriptTail();
ecbool UniqueTrigger(const char* name);
Trigger* GetTrigger(int which);

#endif