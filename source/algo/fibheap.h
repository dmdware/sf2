


#ifndef FIBHEAP_H
#define FIBHEAP_H

#include "../platform.h"
#include "../utils.h"
#include "../debug.h"
#include "../math/fixmath.h"
#include "heapkey.h"

//todo build key into the payload

// https://github.com/beniz/fiboheap/blob/master/fiboheap.h
// https://github.com/beniz/fiboheap/blob/master/fiboqueue.h

//#define FIBHEAP		//use fib heap instead of bin heap?

// byte-align structures
#pragma pack(push, 1)

struct FibNode
{
public:
	FibNode(void *pl)
		:/* key(k), */ mark(ecfalse),p(NULL),left(NULL),right(NULL),child(NULL),degree(-1),payload(pl)
	{
	}

	~FibNode()
	{
	}

	//int key;
	ecbool mark;
	FibNode *p;
	FibNode *left;
	FibNode *right;
	FibNode *child;
	int degree;
	void *payload;
};

struct PathNode;

struct FibHeap
{
public:
	int n;
	FibNode *minn;
	
  //std::unordered_multimap<void*, FibNode*> fstore;
  std::map<void*, FibNode*> fstore;

	FibHeap()
		:n(0),minn(NULL)
	{
	}

	~FibHeap()
	{
		free();
	}


	void free()
	{
		// delete all nodes.
		delete_fibnodes(minn);
		minn = NULL;
		fstore.clear();
		n = 0;
	}

	void setbase(HeapKey basecost){}

	void fillup(){}

	void alloc(int ncells){}

	//todo replace while(ectrue) loops with goto; for less comparisons

	void delete_fibnodes(FibNode *x);

	FibNode* add(void* pl);

	FibNode* extract_min();

	void* delmin();
	void consolidate();

	void fib_heap_link( FibNode* y, FibNode* x );

	FibNode* find(void* payload, FibNode* x);

	void heapify(void* payload);

	//void check(const char* comment, FibNode* x);

	void decrease_key( FibNode* x );

	void cut( FibNode* x, FibNode* y );

	void cascading_cut( FibNode* y );

#if 0
	/*
	* set to infinity so that it hits the top of the heap, then easily remove.
	*/
	void remove_fibnode( FibNode* x )
	{
		//todo fixme
#undef min
		decrease_key(x,std::numeric_limits<int>::min());
		FibNode *fn = extract_min();
		delete fn;
	}
#endif

	//todo add heapify

	ecbool hasmore();

	
#if 0
	FibNode* push(void* k, void *pl)
  {
    FibNode *x = push(k,pl);
    fstore.insert(std::pair<void*, FibNode*>(k,x));
    return x;
  }
#endif

#if 0
	void decrease_key(FibNode *x, void* k)
  {
    typename std::unordered_map<void*, FibNode*>::iterator mit
      = find(x->payload);
    fstore.erase(mit);
    fstore.insert(std::pair<void*, FibNode*>(k,x));
    decrease_key(x,k);
  }
#endif

  FibNode* push(void* k)
  {
    return add(k);
  }

  FibNode* findNode(void* k)
  {
    //std::unordered_map<void*, FibNode*>::iterator mit
    std::map<void*, FibNode*>::iterator mit
      = find(k);
	if (mit != fstore.end())
		return (*mit).second;
	return NULL;
  }

#if 1
	std::map<void*, FibNode*>::iterator find(void* k)
  {
    std::map<void*, FibNode*>::iterator mit
      =fstore.find(k);
    return mit;
  }
#endif


};

#pragma pack(pop)

#endif