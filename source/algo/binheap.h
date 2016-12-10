#ifndef BINHEAP_H
#define BINHEAP_H

#include "../platform.h"
#include "ecbool.h"
#include "vector.h"

/* http://www.sourcetricks.com/2011/06/c-heaps.html */

struct BinHeap
{
#if 0
	PathNode **heap;
	int nelements;
	int allocsz;
#else
	Vector heap;
	ecbool (*comparefunc)(void *a, void *b);
#endif
};

typedef struct BinHeap BinHeap;

void BinHeap_init(BinHeap *bh, ecbool (*comparef)(void *a, void *b));
void BinHeap_free(BinHeap *bh);
ecbool BinHeap_insert(BinHeap *bh, void *element);
void *BinHeap_delmin(BinHeap *bh);
ecbool BinHeap_hasmore(BinHeap *bh);
void BinHeap_alloc(BinHeap *bh, int ncells);
void BinHeap_freemem(BinHeap *bh);
void BinHeap_resetelems(BinHeap *bh);
void BinHeap_heapify(BinHeap *bh, void *element);

int BinHeap_left(BinHeap *bh, int parent);
int BinHeap_right(BinHeap *bh, int parent);
int BinHeap_parent(BinHeap *bh, int child);
void BinHeap_heapifyup(BinHeap *bh, int index);
void BinHeap_heapifydown(BinHeap *bh, int index);

#endif
