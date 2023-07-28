/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islmem.h			Memory management

************************************************************************************/


#ifndef __ISLMEM_H__
#define __ISLMEM_H__


// MALLOC and FREE macros, defined for convenience

#define MALLOC(A) memoryAllocate(A,__FILE__,__LINE__)
#define FREE(A)	memoryFree(A, __FILE__, __LINE__)


/**************************************************************************
	FUNCTION:	memoryInitialise()
	PURPOSE:	Initialise memory for allocation
	PARAMETERS:	Base address, size of area, maximum number of allocs
	RETURNS:	0/1
**************************************************************************/

int memoryInitialise(unsigned long base, unsigned long size, unsigned long maxAllocs);


/**************************************************************************
	FUNCTION:	memoryDestroy()
	PURPOSE:	Destroy memory allocation data
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryDestroy();


/**************************************************************************
	FUNCTION:	memoryReset()
	PURPOSE:	Reset memory allocation
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryReset();


/**************************************************************************
	FUNCTION:	memoryCheck() (DEBUG BUILD ONLY)
	PURPOSE:	Check list of allocated blocks for overwriting problems
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryCheck();


/**************************************************************************
	FUNCTION:	memoryAllocate()
	PURPOSE:	Allocate block of memory from pool
	PARAMETERS:	Size of area, file name, line numebr
	RETURNS:	Base address of block or NULL
**************************************************************************/

void *memoryAllocate(unsigned long size, char *file, int lineno);


/**************************************************************************
	FUNCTION:	memoryFree()
	PURPOSE:	Free block of memory allocated from global pool
	PARAMETERS:	Base ptr, filename, line number
	RETURNS:	
**************************************************************************/

void memoryFree(void *blk, char *file, int line);


/**************************************************************************
	FUNCTION:	memoryShow()
	PURPOSE:	Send list of allocated blocks to debug output
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryShow();


/**************************************************************************
	FUNCTION:	memoryShowStats()
	PURPOSE:	Show memory usage statistics
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryShowStats();


#endif