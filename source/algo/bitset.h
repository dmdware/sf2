

#ifndef BITSET_H
#define BITSET_H

#include "../platform.h"

struct BitSet
{
	unsigned int *bits;
	int size;
};

typedef struct BitSet BitSet;

void BitSet_init(BitSet* bs);
void BitSet_free(BitSet* bs);
void BitSet_resize(BitSet* bs, int count);
void BitSet_set(BitSet* bs, int i);

#define BITSET_ON(bs,i)	(bs->bits[i >> 5] & (1 << (i & 31)))

int BitSet_on(BitSet* bs, int i);
void BitSet_clear(BitSet* bs, int i);
void BitSet_clearall(BitSet* bs);

#endif