













#ifndef SAVEHINT_H
#define SAVEHINT_H

#include "../platform.h"

struct Hint;

void SaveHint(Hint* h, FILE* fp);
void ReadHint(Hint* h, FILE* fp);
void SaveLastHint(FILE* fp);
void ReadLastHint(FILE* fp);

#endif