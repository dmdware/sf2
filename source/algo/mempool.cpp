










#include "mempool.h"
#include "../platform.h"
#include "../utils.h"

MemPool::MemPool()
{
	pMemBlock = NULL;
	pAllocatedMemBlock = NULL;
	pFreeMemBlock = NULL;
}

/*==============================================================================
MemPool:
Constructor of this struct. It allocate memory block from system and create
a static double linked std::list to manage all memory unit.

Parameters:
[in]ulUnitNum
The number of unit which is a part of memory block.

[in]ulUnitSize
The size of unit.
//=============================================================================
*/
void MemPool::allocsys(int nunits, int unitsz)
{
	freesysmem();

	ulBlockSize = nunits * (unitsz + sizeof(struct MemUnit));
	ulUnitSize = unitsz;

	pMemBlock = malloc(ulBlockSize);			//Allocate a memory block.

	if(!pMemBlock)
		OUTOFMEM();

	resetunits();
}

/*==============================================================================
~MemPool():
Destructor of this struct. Its task is to free memory block.
//=============================================================================
*/
MemPool::~MemPool()
{
	freesysmem();
}


/*==============================================================================
Alloc:
To allocate a memory unit. If memory pool can`t provide proper memory unit,
will call system function.

Parameters:
[in]ulSize
Memory unit size.

[in]bUseMemPool
Whether use memory pool.

Return Values:
Return a pointer to a memory unit.
//=============================================================================
*/
void* MemPool::alloc()
{
	if( pMemBlock == NULL || pFreeMemBlock == NULL)
	{
#if 0
		return malloc(ulSize);
#else
		return NULL;
#endif
	}

	//Now FreeList isn`t empty
	struct MemUnit *pCurUnit = pFreeMemBlock;

	pFreeMemBlock = pCurUnit->pNext;			//Get a unit from free linkedlist.
	if(NULL != pFreeMemBlock)
	{
		pFreeMemBlock->pPrev = NULL;
	}

	pCurUnit->pNext = pAllocatedMemBlock;

	if(NULL != pAllocatedMemBlock)
	{
		pAllocatedMemBlock->pPrev = pCurUnit;
	}
	pAllocatedMemBlock = pCurUnit;

	return (void *)((char *)pCurUnit + sizeof(struct MemUnit) );
}


/*==============================================================================
Free:
To free a memory unit. If the pointer of parameter point to a memory unit,
then insert it to "Free linked std::list". Otherwise, call system function "free".

Parameters:
[in]p
It point to a memory unit and prepare to free it.

Return Values:
none
//=============================================================================
*/
void MemPool::freeunit( void* p )
{
#if 0
	if(pMemBlock<p && p<(void *)((char *)pMemBlock + ulBlockSize) )
	{
#endif
		struct MemUnit *pCurUnit = (struct MemUnit *)((char *)p - sizeof(struct MemUnit) );

		pAllocatedMemBlock = pCurUnit->pNext;
		if(NULL != pAllocatedMemBlock)
		{
			pAllocatedMemBlock->pPrev = NULL;
		}

		pCurUnit->pNext = pFreeMemBlock;
		if(NULL != pFreeMemBlock)
		{
			pFreeMemBlock->pPrev = pCurUnit;
		}

		pFreeMemBlock = pCurUnit;
#if 0
	}
	else
	{
		free(p);
	}
#endif
}

void MemPool::resetunits()
{
	if(!pMemBlock)
		return;

	for(int i=0; i<nUnits; i++)	//Link all mem unit .
	{
		struct MemUnit *pCurUnit = (struct MemUnit *)( (char *)pMemBlock + i*(ulUnitSize + sizeof(struct MemUnit)) );

		pCurUnit->pPrev = NULL;
		pCurUnit->pNext = pFreeMemBlock;		//Insert the new unit at head.

		if(NULL != pFreeMemBlock)
		{
			pFreeMemBlock->pPrev = pCurUnit;
		}
		pFreeMemBlock = pCurUnit;
	}
}

void MemPool::freesysmem()
{
	if(pMemBlock)
	{
		free(pMemBlock);
		pMemBlock = NULL;
	}

	pMemBlock = NULL;
	pAllocatedMemBlock = NULL;
	pFreeMemBlock = NULL;
}
