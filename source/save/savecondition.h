













#ifndef SAVECONDITION_H
#define SAVECONDITION_H

#include "../platform.h"

struct Condition;

void SaveCondition(Condition* c, FILE* fp);
void ReadCondition(Condition* c, FILE* fp);

#endif