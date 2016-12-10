

#include "bitset.h"



void BitSet_init(BitSet* bs)
{
	bits = NULL;
	size = 0;
}

void BitSet_free(BitSet* bs)
{
	free(bs->bits);
	bs->bits = NULL;
	bs->size = 0;
}

void BitSet_resize(BitSet* bs, int count)
{
	bs->size = count / 32 + 1;

	if(bs->bits)
	{
		free(bs->bits);
		bs->bits = NULL;
	}

	bs->bits = (unsigned int*)malloc(sizeof(unsigned int)*size);
	BitSet_clearall(bs);
}

void BitSet_set(BitSet* bs, int i)
{
	bs->bits[i >> 5] |= (1 << (i & 31));
	//bits[i / 32] |= (1 << (i % 32));
}

int BitSet_on(BitSet* bs, int i)
{
	return bs->bits[i >> 5] & (1 << (i & 31));
}

void BitSet_clear(BitSet* bs, int i)
{
	bs->bits[i >> 5] &= ~(1 << (i & 31));
}

void BitSet_clearall(BitSet* bs)
{
	memset(bs->bits, 0, sizeof(unsigned int) * bs->size);
}