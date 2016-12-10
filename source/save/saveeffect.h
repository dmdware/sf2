













#ifndef SAVEEFFECT_H
#define SAVEEFFECT_H

#include "../platform.h"

struct Effect;

void SaveEffect(Effect* e, FILE* fp);
void ReadEffect(Effect* e, std::vector<int>* triggerrefs, FILE* fp);

#endif