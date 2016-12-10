








/*
simplified memory pool
given that
we keep allocating to the end of a growing pool
and only free the elements altogether, or back up to a certain point
*/

#ifndef STACKPOOL_H
#define STACKPOOL_H

#include "../platform.h"

// http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
// http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/au-memorymanager-pdf.pdf


struct StackPool
{
public:
	void*			pMemBlock;			//The address of memory pool.

	int totalsz;	//	MAXPATHN * sizeof(PathNode)
	int freestart;

	StackPool();
	~StackPool();

	void			allocsys(int totalsz);
	void*           alloc(int unitsz);	//Allocate memory unit
	void            freeunit( void* p );	//Free memory unit
	void			resetunits();
	void			freesysmem();
};

#endif
