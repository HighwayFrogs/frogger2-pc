/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islmem.c			Memory management

************************************************************************************/

#include <stdio.h>
#include <string.h>
#include "islmem.h"

#define NULL 0

#define POLLHOST() asm("break 1024")


typedef struct _AllocBlockType {
	unsigned long	offset;					// where this block starts (offset from base)
    unsigned long	size;  					// size of this block
	unsigned char	inuse;					// is this block in use?
#ifdef _DEBUG
	char			file[15];				// filename where this block was malloc'd
	int				lineno;					// line number where this block was malloc'd
#endif
} AllocBlockType;


typedef struct {
	AllocBlockType	*blocks;				// array of control structs
	unsigned long	base;			   		// base address for global block
	unsigned long	size;			   		// size of area
	int				lastused;		   		// number of last used block
	int				maxAllocs;				// largest number of mallocs allowed
#ifdef _DEBUG
	int				total;					// total amount of memory malloc'd
	int				peak;					// peak amount of memory malloc'd
#endif
} AllocType;


static AllocType	pool;

static int			lastBlockAlloced;


#ifdef _DEBUG

void memoryGetLastAlloc()
{
	int				loop;
	unsigned char	*ucp;

	if (pool.lastused == -1)
		return;

	ucp = (unsigned char *)(pool.base+pool.size);
	for(loop=0; loop<=pool.lastused; loop++)
	{
		ucp -= pool.blocks[loop].size;
		if (pool.blocks[loop].inuse)
		{
			if(loop == lastBlockAlloced)
			{
				printf("\n  #%3d ---- @ 0x%x, %6d: %s/%d", loop,
						(int)ucp, pool.blocks[loop].size, pool.blocks[loop].file,pool.blocks[loop].lineno);
			}
		}
	}
}

#endif

/**************************************************************************
	FUNCTION:	memoryInitialise()
	PURPOSE:	Initialise memory for allocation
	PARAMETERS:	Base address, size of area, maximum number of allocs
	RETURNS:	0 on success, 1 on failure
**************************************************************************/

int memoryInitialise(unsigned long base, unsigned long size, unsigned long maxAllocs)
{
	if((sizeof(AllocBlockType) * maxAllocs) > size)
	{
		printf("\nmemoryInitialise: too many mallocs (%d = %d bytes!)", maxAllocs, (sizeof(AllocBlockType) * maxAllocs));
		return 1;
	}

	pool.base = base + (sizeof(AllocBlockType) * maxAllocs);
	pool.size = size - (sizeof(AllocBlockType) * maxAllocs);
	pool.blocks = (AllocBlockType *)base;
	pool.maxAllocs = maxAllocs;
	memoryReset();
	printf("\nmemoryInitialise: base = 0x%x, size = 0x%x", base, size);
	printf("\nmemoryInitialise: pool.base = 0x%x, pool.size = 0x%x, pool.top = 0x%x\n",pool.base, pool.size, pool.base+pool.size);

	lastBlockAlloced = -1;
	return 0;
}


/**************************************************************************
	FUNCTION:	memoryDestroy()
	PURPOSE:	Destroy memory allocation data
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

/*
static void ShowMemBytes(char *buf, unsigned char *src, int nbytes)
{
	while (nbytes-- > 0)
	{
		if (*src < ' ')
			*buf++ = '.';
		else
			*buf++ = *src;
		src++;
	}
}
*/


void memoryDestroy()
{
	int		loop;

#ifdef _DEBUG
	printf("\nmemoryDestroy: usage peaked @ %d bytes (%d Kb)", pool.peak, pool.peak>>10);
#endif
	
	for(loop=0; loop<=pool.lastused; loop++)
	{
		if (pool.blocks[loop].inuse)
		{
			printf("\nmemoryDestroy: leak @ 0x%x, size %d", 
					pool.base+pool.size-pool.blocks[loop].size-pool.blocks[loop].offset,
					pool.blocks[loop].size);
#ifdef _DEBUG
			printf("\nFrom %s:%d", pool.blocks[loop].file,pool.blocks[loop].lineno);
#endif
		}
	}
	pool.size = 0;
}


/**************************************************************************
	FUNCTION:	memoryReset()
	PURPOSE:	Reset memory allocation
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryReset()
{
	int		loop;

	for(loop=0; loop<pool.maxAllocs; loop++) 		// initialise the block array
	{
		pool.blocks[loop].inuse = 0;
		pool.blocks[loop].offset = 0;
		pool.blocks[loop].size = 0;
	}

	pool.lastused = -1;

//	memset((char *)pool.base, 0xed, pool.size);

#ifdef _DEBUG
	pool.total = 0;
	pool.peak = 0;
#endif
}


#ifdef _DEBUG


/**************************************************************************
	FUNCTION:	memoryAllocate() (DEBUG VERSION)
	PURPOSE:	Allocate block of memory from pool (with debug info)
	PARAMETERS:	Size of area, ID string, filename, line number
	RETURNS:	Base address of block or NULL
**************************************************************************/

void *memoryAllocate(unsigned long size, char *file, int line)
{
	#define MEMORY_MARKBLOCK	dPtr = ret; \
								dPtr2 = dPtr+size/4-1; \
								*(dPtr) = 0xDEADBEEF; \
								*(dPtr+1) = (unsigned long)dPtr2; \
								*(dPtr+2) = 0xDEADBEEF; \
								*(dPtr2) = 0xEAEBECED; \
								dPtr += 3; \
								ret = dPtr;

	int				l,k;
	unsigned long	oldsize;
	void			*ret;
	unsigned long	*dPtr, *dPtr2;

	if ((size<=0) || (size>pool.size))						// silly :-)
	{
		printf("\nmemoryAllocate: failure @ %s:%d - block size illegal (%d bytes)", file, line, size);
    	return NULL;
	}

	size += 16;
	if ((size%4)!=0)
    	size += 4-(size%4);									// round to long boundary

	pool.total += size;
	if (pool.total>pool.peak)
		pool.peak = pool.total;
	if (pool.total>pool.size-100*1024)
	{
		printf("\nmemoryAllocate: Memory low warning");
		printf("\nTotal=%d/size=%d/peak=%d\n", pool.total,pool.size,pool.peak);
	}
	if (pool.total>pool.size)
	{
		memoryShow();
		memoryShowStats();
		printf("\nmemoryAllocate: failure @ %s:%d - Out of memory (wanted %d bytes)", file, line, size);
		printf("Total=%d/size=%d/peak=%d\n", pool.total,pool.size,pool.peak);
		while(1)
		{
			POLLHOST();
		}
	}

	if (pool.lastused==-1)									// first ever block claimed?
	{
		strncpy(pool.blocks[0].file, file, 14);
		pool.blocks[0].lineno = line;
		pool.blocks[0].inuse = 1;
		pool.blocks[0].size = size;
		pool.blocks[0].offset = 0;
		pool.lastused = 0;
		ret = (void *)(pool.base+pool.size-size);
		MEMORY_MARKBLOCK;
		lastBlockAlloced = 0;
		return ret;
	}

	for(l=0; l<pool.maxAllocs; l++)
	{
		if ((pool.blocks[l].inuse==0) && (pool.blocks[l].size>=size))
			break;
		if (l==pool.lastused+1)
			break; 
	}
	if (l==pool.maxAllocs)										// could find no available record (shouldn't happen)
	{
		
		printf("memoryAllocate: failure @ %s:%d - could find no available record\n", file,line);
		return NULL;
	}

	if (l==pool.lastused+1)									// add to the end of the list
	{
		oldsize = (pool.blocks[pool.lastused].offset+pool.blocks[pool.lastused].size);
		if (pool.size-oldsize<size)							// not enough space left at end of pool
			return NULL;
        pool.lastused++;
		strncpy(pool.blocks[l].file, file, 19);
		pool.blocks[l].lineno = line;
		pool.blocks[l].inuse = 1;
		pool.blocks[l].size = size;
		pool.blocks[l].offset = oldsize;
		ret = (void *)(pool.base+pool.size-size-pool.blocks[pool.lastused].offset);
		MEMORY_MARKBLOCK;
		lastBlockAlloced = l;
		return ret;
	}

	oldsize = pool.blocks[l].size;
	strncpy(pool.blocks[l].file, file, 19);
	pool.blocks[l].lineno = line;
	pool.blocks[l].size = size;
	pool.blocks[l].inuse = 1;
	if ((size < oldsize) && (pool.lastused < pool.maxAllocs-1))		// if the old block was larger and there is room to grow...
	{
		for(k=pool.lastused; k>l; k--)
			pool.blocks[k+1] = pool.blocks[k];
		k = l+1;
		pool.lastused++;        
		pool.blocks[k].inuse = 0;
		pool.blocks[k].size = oldsize-size;
		pool.blocks[k].offset = pool.blocks[l].offset+size; 
	}
	else if (size < oldsize)
    	pool.blocks[l].size = oldsize;						// can't split it, so to avoid drop-out :-(

	ret = (void *)(pool.base+pool.size-size-pool.blocks[l].offset);
	MEMORY_MARKBLOCK;
	lastBlockAlloced = l;
	return ret;
}


/**************************************************************************
	FUNCTION:	memoryFree() (DEBUG VERSION)
	PURPOSE:	Free block of memory allocated from global pool
	PARAMETERS:	Base ptr, source file, line no
	RETURNS:	
**************************************************************************/

void memoryFree(void *blk, char *file, int line)
{
	unsigned char	*tp;
	int				l, k, bad = 0;
	unsigned long	*chkPtr;

	if (pool.lastused == -1)								// nothing to free
		return;

	blk -= 12;
	tp = (unsigned char *)(pool.base+pool.size);
	for(l=0; l<=pool.lastused; l++)
		{
		tp -= pool.blocks[l].size;
		if (tp==blk)
			break;
		}

	if (l>pool.lastused)										// could not find the block to allocate
		return;

	pool.blocks[l].inuse = 0;

	pool.total -= pool.blocks[l].size;
	chkPtr = blk;
	if (*(chkPtr)!=0xDEADBEEF)
	{
		printf("memoryFree: failure @ %s:%d - BADCODE: Beginning(1) overwritten\n", file,line);
		bad = 1;
	}
	if (*(chkPtr+2)!=0xDEADBEEF)
	{
		printf("memoryFree: failure @ %s:%d - BADCODE: Beginning(2) overwritten\n", file,line);
		bad = 1;
	}
	if (!bad)
	{
		chkPtr = (unsigned long *) *(chkPtr+1);
		if (*(chkPtr)!=0xEAEBECED)
		{
			printf("\nmemoryFree: failure @ %s:%d - BADCODE: End overwritten", file,line);
			bad = 1;
		}
	}

	if (bad)
	{
		printf("\nBlock malloc @ %s:%d", pool.blocks[l].file, pool.blocks[l].lineno);
	}

	if (l==pool.lastused)
	{
		pool.lastused--;											// last item in list
		if ((pool.lastused>=0) && (pool.blocks[pool.lastused].inuse == 0))	// was the previous one empty aswell?
			pool.lastused--;
		return;
	}

	if (pool.blocks[l+1].inuse == 0)								// merge with next one
	{
		pool.blocks[l].size += pool.blocks[l+1].size;
		for(k=l+1; k<pool.lastused; k++)
			pool.blocks[k] = pool.blocks[k+1];
        pool.blocks[pool.lastused--].inuse = 0;
	}

	if ((l>0) && (pool.blocks[l-1].inuse == 0))				// merge with previous one
	{
		pool.blocks[l-1].size += pool.blocks[l].size;
		for(k=l; k<pool.lastused; k++)
			pool.blocks[k] = pool.blocks[k+1];
		pool.blocks[pool.lastused--].inuse = 0;
	}
}


/**************************************************************************
	FUNCTION:	memoryCheck() (DEBUG BUILD ONLY)
	PURPOSE:	Check list of allocated blocks for overwriting problems
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryCheck()
{
	int				loop, bad = 0, beenBad = 0;
	unsigned char	*ucp;
	unsigned long	*chkPtr;


	if (pool.lastused == -1)
		return;

	ucp = (unsigned char *)(pool.base+pool.size);
	for(loop=0; loop<=pool.lastused; loop++)
	{
		ucp -= pool.blocks[loop].size;
		if (pool.blocks[loop].inuse)
		{
			chkPtr = (unsigned long *) ucp;
			if (*(chkPtr)!=0xDEADBEEF)
			{
				printf("\nmemoryCheck: BADCODE: Beginning(1) overwritten");
				bad = 1;
			}
			if (*(chkPtr+2)!=0xDEADBEEF)
			{
				printf("\nmemoryCheck: BADCODE: Beginning(2) overwritten");
				bad = 1;
			}
			chkPtr = (unsigned long *) *(chkPtr+1);
			if (*(chkPtr)!=0xEAEBECED)
			{
				printf("\nmemoryCheck: BADCODE: End overwritten");
				bad = 1;
			}
			if (bad)
			{
				beenBad = 1;
				printf("\nBlock malloc @ %s:%d (0x%x)", pool.blocks[loop].file,pool.blocks[loop].lineno,
					pool.base+pool.size-pool.blocks[loop].size-pool.blocks[loop].offset);
				/*				
				while (1)
				{
					POLLHOST();
				}
				*/
			}

			bad = 0;
		}
	}
	if (!beenBad)
		printf("\nmemoryCheck: checks out OK");


}

#else


/**************************************************************************
	FUNCTION:	memoryAllocate() (RELEASE VERSION)
	PURPOSE:	Allocate block of memory from pool
	PARAMETERS:	Size of area, file name, line no
	RETURNS:	Base address of block or NULL
**************************************************************************/

void *memoryAllocate(unsigned long size, char *file, int lineno)
{
	int		l,k;
	unsigned long	oldsize;
	void	*ret;

	if ((size%4)!=0)
    	size += 4-(size%4);									// round to long boundary

 	if ((size<=0) || (size>pool.size))						// silly :-)
	{
		printf("\nmemoryAllocate: failure @ %s:%d - block size illegal (%d bytes)", file, lineno, size);
    	return NULL;
	}

	if (pool.lastused==-1)									// first ever block claimed?
	{
		pool.blocks[0].inuse = 1;
		pool.blocks[0].size = size;
		pool.blocks[0].offset = 0;
		pool.lastused = 0;
		ret = (void *)(pool.base+pool.size-size);

		return ret;
	}

	for(l=0; l<pool.maxAllocs; l++)
	{
		if ((pool.blocks[l].inuse==0) && (pool.blocks[l].size>=size))
			break;
		if (l==pool.lastused+1)
			break; 
	}
	if (l==pool.maxAllocs)				// could find no available record
	{
		printf("memoryAllocate: failure @ %s:%d - could find no available record\n", file,lineno);
		while(1)
		{
			POLLHOST();
		}
		return NULL;
	}

	if (l==pool.lastused+1)									// add to the end of the list
	{
		oldsize = (pool.blocks[pool.lastused].offset+pool.blocks[pool.lastused].size);
		if (pool.size-oldsize<size)							// not enough space left at end of pool
		{
			printf("memoryAllocate: failure @ %s:%d - No memory left\n", file, lineno);
			while(1)
			{
				POLLHOST();
			}
			return NULL;
		}
        pool.lastused++;
		pool.blocks[l].inuse = 1;
		pool.blocks[l].size = size;
		pool.blocks[l].offset = oldsize;
		ret = (void *)(pool.base+pool.size-size-pool.blocks[pool.lastused].offset);

		return ret;
	}

	oldsize = pool.blocks[l].size;
	pool.blocks[l].size = size;
	pool.blocks[l].inuse = 1;
	if ((size < oldsize) && (pool.lastused < pool.maxAllocs-1))		// if the old block was larger and there is room to grow...
	{
		for(k=pool.lastused; k>l; k--)
			pool.blocks[k+1] = pool.blocks[k];
		k = l+1;
		pool.lastused++;        
		pool.blocks[k].inuse = 0;
		pool.blocks[k].size = oldsize-size;
		pool.blocks[k].offset = pool.blocks[l].offset+size; 
	}
	else if (size < oldsize)
    	pool.blocks[l].size = oldsize;						// can't split it, so to avoid drop-out :-(

	ret = (void *)(pool.base+pool.size-size-pool.blocks[l].offset);

	return ret;
}


/**************************************************************************
	FUNCTION:	memoryFree() (RELEASE VERSION)
	PURPOSE:	Free block of memory allocated from global pool
	PARAMETERS:	Base ptr, filename, line number
	RETURNS:	
**************************************************************************/

void memoryFree(void *blk, char *file, int line)
{
	unsigned char	*tp;
	int		l, k;

	if (pool.lastused == -1)								// nothing to free
		return;

//	blk -= 12;
	tp = (unsigned char *)(pool.base+pool.size);
	for(l=0; l<=pool.lastused; l++)
		{
		tp -= pool.blocks[l].size;
		if (tp==blk)
			break;
		}

	if (l>pool.lastused)										// could not find the block to allocate
		return;

	pool.blocks[l].inuse = 0;

	if (l==pool.lastused)
	{
		pool.lastused--;											// last item in list
		if ((pool.lastused>=0) && (pool.blocks[pool.lastused].inuse == 0))	// was the previous one empty aswell?
			pool.lastused--;
		return;
	}

	if (pool.blocks[l+1].inuse == 0)								// merge with next one
	{
		pool.blocks[l].size += pool.blocks[l+1].size;
		for(k=l+1; k<pool.lastused; k++)
			pool.blocks[k] = pool.blocks[k+1];
        pool.blocks[pool.lastused--].inuse = 0;
	}

	if ((l>0) && (pool.blocks[l-1].inuse == 0))				// merge with previous one
	{
		pool.blocks[l-1].size += pool.blocks[l].size;
		for(k=l; k<pool.lastused; k++)
			pool.blocks[k] = pool.blocks[k+1];
		pool.blocks[pool.lastused--].inuse = 0;
	}
}

#endif


/**************************************************************************
	FUNCTION:	memoryShow()
	PURPOSE:	Send list of allocated blocks to debug output
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryShow()
{
	int				loop;
	unsigned char	*ucp;

    printf("\nmemoryShow: %d mallocs", pool.lastused+1);

	if (pool.lastused == -1)
		return;

	ucp = (unsigned char *)(pool.base+pool.size);
	for(loop=0; loop<=pool.lastused; loop++)
	{
		ucp -= pool.blocks[loop].size;
		if (pool.blocks[loop].inuse)
		{
#ifdef _DEBUG
			printf("\n  #%3d ---- @ 0x%x, %6d: %s/%d", loop,
					(int)ucp, pool.blocks[loop].size, pool.blocks[loop].file,pool.blocks[loop].lineno);
#else
			printf("\n  #%3d ---- @ 0x%x, %6d", loop, (int)ucp, pool.blocks[loop].size);
#endif
		}
		else
		{
			printf("\n  #%3d FREE @ 0x%x, %6d", loop,
					(int)ucp, pool.blocks[loop].size);
		}
	}
}


/**************************************************************************
	FUNCTION:	memoryShowStats()
	PURPOSE:	Show memory usage statistics
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void memoryShowStats()
{
	int		loop, inuse;
	unsigned long	used, freed;

	used = 0;
	freed = 0;
	inuse = 0;
	for(loop=0; loop<=pool.lastused; loop++)
	{
		if (pool.blocks[loop].inuse)
		{
			used += pool.blocks[loop].size;
			inuse++;
		}
		else
			freed += pool.blocks[loop].size;
	}
	printf("\nmemoryShowStats:");
	printf("\nTotal RAM:   %d Kb", pool.size>>10);
	printf("\nMallocs available: %d, %d used (%d perc usage)", pool.lastused+1, inuse, (inuse*100)/(pool.lastused+1));
	printf("\nUsed RAM:    %dKb, %dKb used (%dKb free %d perc gaps)", (used+freed)>>10,used>>10,freed>>10, (freed*100)/used);
}

