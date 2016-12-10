#include "binheap.h"
//#include "../path/pathnode.h"
#include "../utils.h"
#include "vector.h"

void BinHeap_init(BinHeap *bh, ecbool (*comparef)(void *a, void *b))
{
#if 0
	heap = NULL;
	allocsz = 0;
	nelements = 0;
#endif
    Vector_init(&bh->heap);
	bh->comparefunc = comparef;
}

void BinHeap_free(BinHeap *bh)
{
	BinHeap_freemem(bh);
}

//not used right now? like some other funcs here
void BinHeap_alloc(BinHeap *bh, int ncells)
{
#if 0
	freemem();

	heap = new PathNode *[ ncells ];
	allocsz = ncells;
	nelements = 0;
#endif
}

void BinHeap_freemem(BinHeap *bh)
{
#if 0
	if(heap)
	{
		delete [] heap;
		heap = NULL;
	}

	allocsz = 0;
	nelements = 0;
#else
    Vector_free(&bh->heap);
#endif
}

void BinHeap_resetelems(BinHeap *bh)
{
#if 0
	nelements = 0;
#else
    Vector_free(&bh->heap);
#endif
}

ecbool BinHeap_insert(BinHeap *bh, void *element)
{
#if 0

	if(nelements >= allocsz)
		return ecfalse;

	heap[nelements] = element;
	nelements++;
	heapifyup(nelements - 1);
#else
    Vector *heap = &bh->heap;
	Vector_pushback(heap, element);
	Vector_heapifyup(heap, heap->total - 1);
#endif

	return ectrue;
}

ecbool BinHeap_hasmore(BinHeap *bh)
{
#if 0
	return nelements > 0;
#else
    Vector *heap = &bh->heap;
    return (ecbool)(heap->total > 0);
#endif
}

/*
 Don't call deletemin if hasmore() is ecfalse;
 that would cause heap's total to become negative!
 */
void *BinHeap_delmin(BinHeap *bh)
{
#if 0
	PathNode *pmin = heap[0];
	heap[0] = heap[nelements - 1];
	nelements--;
	heapifydown(0);
	return pmin;
#else
    Vector *heap;
    void *pmin;
    
    heap = &bh->heap;
    pmin = heap->items[0];
    
	heap->items[0] = heap->items[heap->total - 1];
	Vector_popback(heap);
	BinHeap_heapifydown(bh, 0);
    
	return pmin;
#endif
}

void BinHeap_heapify(BinHeap *bh, void *element)
{
#if 1
    Vector *heap;
    ecbool found;
    int i;
    void *iter;
    void **items;
    
    heap = &bh->heap;
    
	if(heap->total == 0)
		return;
    
    items = heap->items;

	if(element)
	{
        found = ecfalse;

		for(iter=items[0], i=0; items[i]; i++, iter=items[i])
			if(iter == element)
			{
				found = ectrue;
				break;
			}

		if(found)
		{
			BinHeap_heapifydown(bh, i);
			BinHeap_heapifydown(bh, i);
		}
		return;
	}
	for(i = (int)floor(heap->total/2); i; i--)
	{
		BinHeap_heapifydown(bh, i);
	}
	return;
#endif
}

void BinHeap_heapifyup(BinHeap *bh, int index)
{
    void *temp;
    Vector *heap;
    void **items;
    
    heap = &bh->heap;
    items = heap->items;

	while ( ( index > 0 ) &&
           ( BinHeap_parent(bh, index) >= 0 ) &&
           bh->comparefunc(items[BinHeap_parent(bh, index)], items[index])
           / *( heap[parent(index)]->score > heap[index]->score ) */ )
	{
		temp = items[parent(index)];
		items[BinHeap_parent(bh, index)] = items[index];
		items[index] = temp;
		index = BinHeap_parent(bh, index);
	}
}

void BinHeap_heapifydown(BinHeap *bh, int index)
{
    int child;
    void *temp;
    Vector *heap;
    void **items;
    
    child = BinHeap_left(bh, index);
    heap = &bh->heap;
    items = heap->items;
    
	if ( ( child > 0 ) &&
        ( BinHeap_right(bh, index) > 0 ) &&
        bh->comparefunc(items[child], items[BinHeap_right(bh, index)])
	              / * ( heap[child]->score > heap[right(index)]->score ) */ )
	{
		child = BinHeap_right(bh, index);
	}
    
	if ( child > 0 )
	{
		temp = items[index];
		items[index] = items[child];
		items[child] = temp;
		BinHeap_heapifydown(bh, child);
	}
}

int BinHeap_left(BinHeap *bh, int parent)
{
	int i = ( parent << 1 ) + 1; // 2 * parent + 1
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < bh->heap.total ) ? i : -1;
#endif
}

int BinHeap_right(BinHeap *bh, int parent)
{
	int i = ( parent << 1 ) + 2; // 2 * parent + 2
#if 0
	return ( i < nelements ) ? i : -1;
#else
	return ( i < bh->heap.total ) ? i : -1;
#endif
}

int BinHeap_parent(BinHeap *bh, int child)
{
    int i;
    
	if (child != 0)
	{
		i = (child - 1) >> 1;
		return i;
	}
	return -1;
}
