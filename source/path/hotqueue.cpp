

#include "hotqueue.h"
#include "pathnode.h"


// byte-align structures
#pragma pack(push, 1)


HotQueue::HotQueue()
{
	free();
}


void HotQueue::free()
{
	bucks.clear();
	topbuck.free();

	//total = used = 0;
}

void HotQueue::setbase(HeapKey basecost)
{
	//uppercost = basecost + MAXDCOST;
	uppercost = basecost;
}
	
void HotQueue::fillup()
{
	//assuming this func is internal and will only be called if HotQueue::hasmore() is true and !topbuck.hasmore()
	//if(!bucks.size())
	//	return;

	Bucket* b = &*bucks.begin();

	uppercost = b->uppercost;

	for(std::list<void*>::iterator nit=b->unsort.begin(); nit!=b->unsort.end(); nit++)
	{
		void* n = *nit;

		topbuck.add(n);
	}

	bucks.pop_front();
}

void HotQueue::add(void* n)
{
	//total++;

	//char m[123];
	//sprintf(m, "o%d o%d", (int)&(((PathNode*)NULL)->heapkey.age), (int)&(((PathNode*)NULL)->heapkey.cost));
	//InfoMess(m,m);

	//if(n->cost <= uppercost)
	if( HEAPCOMPARE( &uppercost, HEAPKEY(n) ) )
	{
		topbuck.add(n);
		return;
	}

	for(std::list<Bucket>::iterator it=bucks.begin(); it!=bucks.end(); it++)
	{
		Bucket* b = &*it;

		//if(n->cost > b->uppercost)
		//if( HEAPCOMPARE( n, &b->uppercost ) )
		if( !HEAPCOMPARE( &b->uppercost, HEAPKEY(n) ) )
			continue;

		//todo add to front, so that when adding to bin heap front-to-back, the earliest won't have to stay on top and the latest ones bubbled down?
		//b->unsort.push_back(n);
		b->unsort.push_front(n);
		return;
	}

	bucks.push_back(Bucket());
	Bucket* b = &*bucks.rbegin();
	
	//b->uppercost = n->cost + MAXDCOST;
	b->uppercost = *HEAPKEY(n);
	//b->uppercost = n->cost;
	b->unsort.push_back(n);
}

void* HotQueue::delmin()
{
	//used++;

	if(!topbuck.hasmore())
		fillup();
	
	//assuming HotQueue::hasmore() was called before calling delmin()
	//if(!topbuck.hasmore())
	//	return NULL;

	return topbuck.delmin();
}

bool HotQueue::hasmore()
{
	return topbuck.hasmore() || bucks.size();
}

void HotQueue::heapify(void* n, HeapKey* oldk)
{
	//total--;

	//find and remove
#if !defined(FIBHEAP)
	for(std::vector<void*>::iterator nit=topbuck.heap.begin(); nit!=topbuck.heap.end(); nit++)
	{
		void* n2 = *nit;

		if(n2 != n)
			continue;

		topbuck.heapify(n, oldk);
		//todo we already found it, so improve that somehow
		return;
	}
#else
#if 0
	if(topbuck.hasmore())
	{
		FibNode* fn = topbuck.find(n, topbuck.minn);
		
		if(fn)
		{
			//void* n2 = fn->payload;
			//topbuck.decrease_key(fn, n2->cost);
			topbuck.decrease_key(fn);
			return;
		}
	}
#else
#ifdef FIBHEAP
	if(topbuck.hasmore() && HEAPCOMPARE(&uppercost, oldk))
	{
	FibNode* fn = topbuck.findNode(n);

	if(fn)
	{
		topbuck.decrease_key(fn);
		return;
	}
	}
#endif
#endif
#endif
	
	for(std::list<Bucket>::iterator bit=bucks.begin(); bit!=bucks.end(); bit++)
	{
		Bucket* b = &*bit;

		if( !HEAPCOMPARE(&b->uppercost, oldk) )
			continue;

#if 1
		for(std::list<void*>::iterator nit=b->unsort.begin(); nit!=b->unsort.end(); nit++)
		{
			void* n2 = *nit;

			if(n2 != n)
				continue;

			b->unsort.erase(nit);

			if(!b->unsort.size())
				bucks.erase(bit);
			
			goto readd;
			//todo we already found it, so improve that somehow
		}
			
		//goto readd;
		break;
#else
#endif
	}

	//readd
readd:

	add(n);
}

#pragma pack(pop)


