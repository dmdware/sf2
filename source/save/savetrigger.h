













#ifndef SAVETRIGGER_H
#define SAVETRIGGER_H

#include "../platform.h"

struct Trigger;

void SaveTrigger(Trigger* t, FILE* fp);
void ReadTrigger(Trigger* t, std::vector<int>* triggerrefs, FILE* fp);
void SaveTriggers(FILE* fp);

#endif