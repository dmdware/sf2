

#include "cheaplist.h"
#include "pathnode.h"
#include "../utils.h"

//CheapList g_cheaplist;


// byte-align structures
#pragma pack(push, 1)

CheapList::CheapList()// : binheap(CompareNodes)
{
}

CheapList::~CheapList()
{
}

void CheapList::free()
{
	sort.clear();
	unsort.clear();

	//binheap.free();

	used = total = 0;
}

//todo get rid of this useless wrapper
bool CheapList::hasmore()
{
#if 0
	if(binheap.hasmore() != (sort.size() || unsort.size()))
	{
		char m[123];
		sprintf(m, "hasmore diff bh%d v%d :: cl%d s%d us%d", 
			(int)binheap.hasmore(),
			(int)binheap.heap.size(),
			(int)(sort.size() || unsort.size()),
			(int)sort.size(),
			(int)unsort.size() );
		InfoMess(m,m);
	}
#endif

	return sort.size() || unsort.size();
}

/*
important note: how equality cases are dealt with determines whether this will return the same nodes off the list
of equal cost. even then it might not be possible to duplicate the exact results of the bin heap.
i think the bin heap will prefer newer added nodes over older ones even if they are of same cost.
so i should try to duplicate that with the cheap list. take note of the function CompareNodes used in the bin heap.
the only exception to the newest-first-out rule might be where an item's score is updated in the class and its position reevaluated.
in the bin heap it might stop before bubbling up above an equal cost item. in the cheap list its removed and then re-added like a new item.
also order of newer-or-older might be lost when an item is discarded from the sorted cheap list to the front of the unsorted.
just don't expect the same results between bin heap and cheap list, even though they are the same a*.
idea: what if we always shuffle the older items to the BACK of the old list when discarding them from the sorted list? 
hm. we read from front to back, so actually it would be added last. and it might infinite loop unless we waited to add it after we went through the unsorted list in fillup().
also if i have different behaviour in heapify, to prefer older equal-cost items, it might get closer to bin heap's exact results.
edit: tried it. still seems to be the same result. heapify doesn't account for much.
another idea: maybe add an "age" counter? and compare like (a->cost < b->cost || (a->cost == b->cost && a->age < b->age))
this might have to be added to binheap too to give the same results.
*/

//given that item n is somewhere in "sort" or "unsort" and its score changed, remove it from its place, and re-add it at the appropriate spot
void CheapList::heapify(void* n)
{
	//find and remove
	for(std::list<void*>::iterator it=sort.begin(); it!=sort.end(); it++)
	{
		void* n2 = *it;

		if(n2 != n)
			continue;

		sort.erase(it);
		goto readd;
	}

	for(std::list<void*>::iterator it=unsort.begin(); it!=unsort.end(); it++)
	{
		void* n2 = *it;

		if(n2 != n)
			continue;

		unsort.erase(it);
		goto readd;
	}

	//re-add
readd:

	total--;

#if 1
	add(n);
#else

	std::list<PathNode*>::iterator it2;

	//will we add it somewhere?
	//if(n->cost < mostcost ||
	if(n->cost <= mostcost ||
		sort.size() < CHEAPITEMS)
	{
		//too many cheap items?
		if(sort.size() > CHEAPITEMS)
		{
			//pop back
			std::list<Widget*>::iterator prevlastit = sort.rbegin();
			PathNode* prevlast = *prevlastit;
			unsort.push_front(prevlast);
			sort.erase( --(prevlastit.base()) );	//need to convert reverse iterator for erase
		}

		//will we add it to the front?
		//if(n->cost < leastcost)
		if(n->cost <= leastcost)
		{
			sort.push_front(n);
			leastcost = n->cost;

			//did we pop the back? need to update most-cost?
			//if(sort.size() >= CHEAPITEMS)
			goto uphigh;

			//continue;
		}

		it2=sort.begin();

		//find the lowest cost in the sorted list that is higher than the one given in "n"
		while(it2!=sort.end())
		{
			const PathNode* n2 = *it2;
			
			//if(n2->cost > n->cost)
			if(n2->cost >= n->cost)
				break;

			it2++;
		}

		//insert before it2 position
		sort.insert(it2, n);

uphigh:

		const std::list<Widget*>::iterator lastit = sort.rbegin();
		const PathNode* lastn = *lastit;
		mostcost = lastn->cost;

		goto end;
	}

	unsort.push_back(n);

	end:
#endif

	//binheap.heapify(n);
}

void CheapList::fillup()
{
	std::list<void*>::iterator it = unsort.begin();
	
	//leastcost = 0x7fffffff;
	//mostcost = 0x7fffffff;
	//assume unsigned
	memset(&leastcost, 0xffffffff, sizeof(HeapKey));
	memset(&mostcost, 0xffffffff, sizeof(HeapKey));

	while(it != unsort.end())
	{
		void* n = *it;
		std::list<void*>::iterator it2;

		//will we add it somewhere?
		//if(n->cost < mostcost || //this one gives different results from bin heap
		//if(n->cost <= mostcost ||
		if( HEAPCOMPARE( n, &mostcost ) || //this one gives different results from bin heap
			sort.size() < CHEAPITEMS)
		{
#if 00000000
			//too many cheap items?
			if(sort.size() > CHEAPITEMS)
			{
				//pop back
				std::list<void*>::iterator prevlastit = sort.rbegin();
				void* prevlast = *prevlastit;
				unsort.push_front(prevlast);
				sort.erase( --(prevlastit.base()) );	//need to convert reverse iterator for erase
			}
#endif

			//advance to next item to check
			it = unsort.erase(it);

			//will we add it to the front?
			//if(n->cost < leastcost)
			//if(n->cost <= leastcost)
			if( HEAPCOMPARE( n, &leastcost ) )
			{
				sort.push_front(n);
				//leastcost = n->cost;
				leastcost = *(HeapKey*)n;

				//did we pop the back? need to update most-cost?
				//if(sort.size() >= CHEAPITEMS)
				goto uphigh;

				//continue;
			}

			it2=sort.begin();

			//find the lowest cost in the sorted list that is higher than the one given in "n"
			while(it2!=sort.end())
			{
				const void* n2 = *it2;
				
				//if(n2->cost > n->cost)
				//if(n2->cost >= n->cost)
				if( HEAPCOMPARE( n, n2 ) )
					break;

				it2++;
			}

			//insert before it2 position
			sort.insert(it2, n);

uphigh:

			const std::list<void*>::reverse_iterator lastit = sort.rbegin();
			const void* lastn = *lastit;
			//mostcost = lastn->cost;
			mostcost = *(HeapKey*)lastn;

			continue;
		}

#if 0
		if(sort.size() < CHEAPITEMS)
		{
			it = unsort.erase(it);
			sort.push_back(n);
			mostcost = n->cost;
			continue;
		}
#endif

		it++;
	}
}

void CheapList::add(void* n)
{
#if 0
		char m[123];
		sprintf(m, "add before bh%d v%d :: cl%d s%d us%d", 
			(int)binheap.hasmore(),
			(int)binheap.heap.size(),
			(int)(sort.size() || unsort.size()),
			(int)sort.size(),
			(int)unsort.size() );
		InfoMess(m,m);
#endif

		total++;

	//binheap.add(n);

	std::list<void*>::iterator it2;

	//will we add it somewhere?
	//if(n->cost < mostcost ||
	//if(n->cost <= mostcost ||
	if( HEAPCOMPARE( n, &mostcost ) ||
		sort.size() < CHEAPITEMS)
	{
#if 000000000000
		//too many cheap items?
		if(sort.size() > CHEAPITEMS)
		{
			//pop back
			std::list<Widget*>::iterator prevlastit = sort.rbegin();
			void* prevlast = *prevlastit;
			unsort.push_front(prevlast);
			sort.erase( --(prevlastit.base()) );	//need to convert reverse iterator for erase
		}
#endif
		//will we add it to the front?
		//if(n->cost < leastcost)
		//if(n->cost <= leastcost)
		if( HEAPCOMPARE( n, &leastcost ) )
		{
			sort.push_front(n);
			//leastcost = n->cost;
			leastcost = *(HeapKey*)n;

			//did we pop the back? need to update most-cost?
			//if(sort.size() >= CHEAPITEMS)
			goto uphigh;

			//continue;
		}

		it2=sort.begin();

		//find the lowest cost in the sorted list that is higher than the one given in "n"
		while(it2!=sort.end())
		{
			const void* n2 = *it2;
			
			//if(n2->cost > n->cost)
			//if(n2->cost >= n->cost)
			if( HEAPCOMPARE( n, n2 ) )
				break;

			it2++;
		}

		//insert before it2 position
		sort.insert(it2, n);

uphigh:

		const std::list<void*>::reverse_iterator lastit = sort.rbegin();
		const void* lastn = *lastit;
		//mostcost = lastn->cost;
		mostcost = *(HeapKey*)lastn;

		goto end;

		return;
	}

	unsort.push_back(n);
	
end:
	;
#if 0
		//char m[123];
		sprintf(m, "add after bh%d v%d :: cl%d s%d us%d", 
			(int)binheap.hasmore(),
			(int)binheap.heap.size(),
			(int)(sort.size() || unsort.size()),
			(int)sort.size(),
			(int)unsort.size() );
		InfoMess(m,m);
#endif
}

void* CheapList::delmin()
{
	//PathNode* r0 = (PathNode*)binheap.delmin();

	used++;

	std::list<void*>::iterator front = sort.begin();

	if(front != sort.end())
	{
		void* r = *front;

#if 1
		sort.erase(front);
#else
		front = sort.erase(front);

		if(front != sort.end())
		{
			leastcost = (*front)->cost;
		}
#if 0
		else
		{
			fillup();
		}
#endif
#endif
		
#if 0
	if(r0 != r)
	{
		char m[123];
		sprintf(m, "deletemin1 diff bh%d c%d :: cl%d c%d", 
			(int)(r0!=NULL),
			(int)(r0?r0->cost:-1),
			(int)(r!=NULL),
			(int)(r?r->cost:-1) );
		InfoMess(m,m);
	}
#endif

		return r;
	}

	//else we need to fill up again
	fillup();
	
	front = sort.begin();

	if(front != sort.end())
	{
		void* r = *front;
		
#if 1
		sort.erase(front);
#else
		front = sort.erase(front);

		if(front != sort.end())
		{
			leastcost = (*front)->cost;
		}
#if 0
		else
		{
			fillup();
		}
#endif
#endif
		
#if 0
	if(r0 != r)
	{
		char m[123];
		sprintf(m, "deletemin2 diff bh%d c%d :: cl%d c%d", 
			(int)(r0!=NULL),
			(int)(r0?r0->cost:-1),
			(int)(r!=NULL),
			(int)(r?r->cost:-1) );
		InfoMess(m,m);
	}
#endif

		return r;
	}

#if 0
	if(r0 != NULL)
	{
		char m[123];
		sprintf(m, "deletemin3 diff bh%d c%d :: cl%d c%d", 
			(int)(r0!=NULL),
			(int)(r0?r0->cost:-1),
			(int)(NULL!=NULL),
			(int)(NULL?-1:-1) );
		InfoMess(m,m);
	}
#endif

	//else there's no more items
	return NULL;
}

#pragma pack(pop)