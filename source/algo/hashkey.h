

#ifndef HASHKEY_H
#define HASHKEY_H

#include "../platform.h"

// byte-align structures
#pragma pack(push, 1)

struct HashKey
{
	unsigned short nx;
	unsigned short ny;
};

#define HASHINT	unsigned int

#pragma pack(pop)

#define HASHKEY(a)			( &((PathNode*)a)->hashkey )
//#define HASHKEY(a)		( a )

#endif