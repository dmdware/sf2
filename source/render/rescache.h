

#ifndef RESCACHE_H
#define RESCACHE_H

#include "../platform.h"

extern FILE* g_rescachefp;

#define RESCACHE		"rescache/bc"
#define RESCACHEVER		1

extern ecbool g_rescacheread;

ecbool LoadResCache(const char* relative);
ecbool MakeResCache(const char* relative);
void AddResCache(const char* identstr);

#endif