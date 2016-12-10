










#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "../platform.h"

// http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
// http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/au-memorymanager-pdf.pdf


struct MemPool
{
private:
	struct MemUnit							//The type of the node of linkedlist.
	{
		struct MemUnit *pPrev, *pNext;
	};

	void*			pMemBlock;			//The address of memory pool.

	//Manage all unit with two linkedlist.
	struct MemUnit*	pAllocatedMemBlock;	//Head pointer to Allocated linkedlist.
	struct MemUnit*	pFreeMemBlock;		//Head pointer to Free linkedlist.

	int	ulUnitSize;			//Memory unit size. There are much unit in memory pool.
	int	ulBlockSize;			//Memory pool size. Memory pool is make of memory unit.
	int nUnits;

public:
	MemPool();
	~MemPool();

	void			allocsys(int nunits, int unitsz);
	void*           alloc();	//Allocate memory unit
	void            freeunit( void* p );										//Free memory unit
	void			resetunits();
	void			freesysmem();
};

#endif
