

#ifndef HEAPKEY_H
#define HEAPKEY_H

#include "../platform.h"

//#define HEAPKEYAGE

// byte-align structures
#pragma pack(push, 1)

//HeapKey must be at the top of the payload item
struct HeapKey
{
	unsigned short cost;
#ifdef HEAPKEYAGE
	unsigned short age;
#endif
};

#pragma pack(pop)

//what condition means a goes after b?
//#define HEAPCOMPARE(a,b)	( (((HeapKey*)a)->cost > ((HeapKey*)b)->cost) || ((((HeapKey*)a)->cost == ((HeapKey*)b)->cost) && (((HeapKey*)a)->age >= ((HeapKey*)b)->age)) )	//prefer younger tie-breaker
#define HEAPCOMPARE(a,b)	( (((HeapKey*)a)->cost >= ((HeapKey*)b)->cost) )
//#define HEAPCOMPARE(a,b)	( (((HeapKey*)a)->cost > ((HeapKey*)b)->cost) || ((((HeapKey*)a)->cost == ((HeapKey*)b)->cost) && (((HeapKey*)a)->age <= ((HeapKey*)b)->age)) )	//prefer older tie-breaker

#define HEAPKEY(a)			( &((PathNode*)a)->heapkey )
//#define HEAPKEY(a)		( a )

#endif