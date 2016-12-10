

#ifndef CHEAPLIST_H
#define CHEAPLIST_H

//#include "pathnode.h"
#include "../platform.h"
#include "../algo/binheap.h"
#include "../algo/heapkey.h"

//todo make generic and move to /algo/

//#define CHEAPLIST	//use cheap list optimization?


// byte-align structures
#pragma pack(push, 1)

#define CHEAPITEMS		15

class PathNode;

class CheapList
{
public:

	//binheap for comparison
	//BinHeap binheap;

	//the highest and lowest cost ends of the ~15 sorted items
	//int32_t mostcost;
	//int32_t leastcost;
	HeapKey mostcost;
	HeapKey leastcost;

	//todo try vector for these, compare to list, deque
	std::list<void*> sort;
	std::list<void*> unsort;	//the list of unsorted nodes not in the cheap list binary heap
	//int32_t limit;	//the highest cost in the cheap list binary heap, used to find the next item to add to cheap list and update limit

	void free();
	void fillup();
	void add(void* n);	//check where we will add this, sorted or unsorted, etc.
	void* delmin();
	bool hasmore();
	//void alloc(int32_t ncells);
	void heapify(void* n);
	
	void setbase(HeapKey basecost){}

	int total;
	int used;

	CheapList();
	~CheapList();
};

//extern CheapList g_cheaplist;

//int32_t g_limitcost;
/*
the highest cost in the cheap list binary heap. if a new node becomes available that is lower than this, it gets added to the cheap list.
the assumption is that we always have and keep updating the highest cost in the cheap list binary heap and know when we can add from new nodes.
when the cheap list runs out, the assumption being that we used all the cheapest nodes possible, we get 15 of the cheapest from the unsorted list and update limitcost.
*/

#pragma pack(pop)

#endif