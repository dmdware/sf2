










#include "stackpool.h"
#include "../platform.h"
#include "../utils.h"

StackPool::StackPool()
{
	pMemBlock = NULL;
	freestart = 0;
	totalsz = 0;
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
void StackPool::allocsys(int totalsz)
{
	freesysmem();

	totalsz = totalsz;
	freestart = 0;

	pMemBlock = malloc(totalsz);			//Allocate a memory block.

	if(!pMemBlock)
		OUTOFMEM();

	resetunits();
}

/*==============================================================================
~MemPool():
Destructor of this struct. Its task is to free memory block.
//=============================================================================
*/
StackPool::~StackPool()
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
void* StackPool::alloc(int unitsz)
{
	const int end = freestart + unitsz;

	if( /* pMemBlock == NULL || */ end >= totalsz)
	//if(ectrue)
	{
#if 0
		return malloc(unitsz);
#else
		return NULL;
#endif
	}

	//will this cause alignment issues if trying to cast to a misaligned struct with alignment of eg 4 bytes?
	void* memunit = (void*)( ((char*)pMemBlock) + freestart);

	freestart = end;

	return memunit;
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
void StackPool::freeunit( void* p )
{
#if 0
	if(pMemBlock<=p && p<(void*)( ((char*)pMemBlock) + totalsz) )
	{
	}
	else
	{
		free(p);
	}
#endif
}

void StackPool::resetunits()
{
	//if(!pMemBlock)
	//	return;

	freestart = 0;
}

void StackPool::freesysmem()
{
	if(pMemBlock)
	{
		free(pMemBlock);
		pMemBlock = NULL;
	}

	pMemBlock = NULL;
	totalsz = 0;
	freestart = 0;
}
