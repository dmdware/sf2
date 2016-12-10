

#ifndef HOTQUEUE_H
#define HOTQUEUE_H

//todo make generic and move to /algo/

#include "pathnode.h"
#include "../platform.h"
#include "../algo/binheap.h"
#include "../sim/simdef.h"
#include "../algo/fibheap.h"
#include "../algo/heapkey.h"


// byte-align structures
#pragma pack(push, 1)

//max delta cost
//#define MAXDCOST		1
#define MAXDCOST		2
//#define MAXDCOST		3
//#define MAXDCOST		(3+3)	//the maximum change in cost between pathnode neighbours
//#define MAXDCOST		(8)	//less buckets and a power of 2, so bucket number can easily be obtained with bit shift
//also need to >>1 because of granularity <<1 applied to node cost
//#define BUCKETSZ		MAXDCOST		//the maximum range in cost between the pathnodes in a bucket
#define MAXDCOST_BITS	3

class Bucket
{
public:
	//BinHeap sort;
	//std::vector<PathNode*> unsort;

	//Bucket() : sort(CompareNodes)
	//{
	//}

	//void heapify();
	//PathNode* delmin();

	//int32_t uppercost;
	HeapKey uppercost;
	std::list<void*> unsort;
	 

#if 0
	std::unordered_map<void*, FibNode*> fstore;

	
  FibNode* push(void* k)
  {
    return add(k);
  }

  FibNode* findNode(void* k)
  {
    std::unordered_map<void*, FibNode*>::iterator mit
      = find(k);
	if (mit != fstore.end())
		return (*mit).second;
	return NULL;
  }

#if 1
	std::unordered_map<void*, FibNode*>::iterator find(void* k)
  {
    std::unordered_map<void*, FibNode*>::iterator mit
      =fstore.find(k);
    return mit;
  }
#endif
#endif

};

#define MAXCOST		(MAX_MAP*MAX_MAP*TILE_SIZE*TILE_SIZE/PATHNODE_SIZE/PATHNODE_SIZE*2)		//the absolute maximum possible cost for a pathnode
//#define MAXCOST		((MAXPATHN*MAXPATHN*2))		//the absolute maximum possible cost for a pathnode
//should be sqrt but that's not a const literal

//todo make generic and move to /algo/

//#define BUCKETS		(MAXCOST/MAXDCOST+1)	//too high

//400-500-700 without, binheap
//400-500 with, binheap
//300-500 with, fibheap
//#define HOTQUEUE		//used hot queue instead of bin heap or cheap list?

class HotQueue
{
public:
	//Bucket buckets[BUCKETS];	//too big
	//std::set<Bucket> bucket;
	//BinHeap buckets;

	//bit field for filled buckets
	//uint64_t m_usedbuck;
	//int32_t m_basecost;	//depends on distance between start and dest

	//int total, used;

	//todo split top buck when too large and lower bucks
	
#ifdef FIBHEAP
	FibHeap topbuck;
#else
	BinHeap topbuck;
#endif
	HeapKey uppercost;
	std::list<Bucket> bucks;

	void free();
	//void setbase(int32_t basecost);
	void setbase(HeapKey basecost);
	
	void fillup();
	void add(void* n);	//check where we will add this, sorted or unsorted, etc.
	void* delmin();
	bool hasmore();
	//void alloc(int32_t ncells);
	void heapify(void* n, HeapKey* oldk);

	HotQueue();
};

#pragma pack(pop)

#endif