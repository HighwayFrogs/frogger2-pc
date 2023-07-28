

#ifndef __BFF_LOAD_H__
#define __BFF_LOAD_H__

#include "types.h"
#include "map_draw.h"
#include <isltex.h>
#include <islmem.h>





// All the types of object that you stick into a BFF need to start with a standard header
// which allows the BFF routines to seek out specific objects on request.

// "id" is long something like "PSA0" or "COL1" for PSA and collision data,
// for identifying the TYPE of the block it is.

// "len" is the length of the block, including the header.

// "crc" is the CRC of the name of this specific entry, to distinguish it from any
// other blocks of the same type.

typedef struct BFF_HeaderTag
{
	unsigned long id;
	unsigned long len;
	unsigned long crc;
}BFF_Header;


// ===================================================================================
// General BFF functions, not specific to the frogger-map stuff
BFF_Header *BFF_LoadFile(char *filename);
void BFF_ReleaseFile(BFF_Header *header);

// Set CRC to zero if you know there's only one instance of that type in the BFF.
BFF_Header *BFF_FindObject(int id, unsigned long crc);
BFF_Header *BFF_FindNamedObject(int id, char *name);

int BFF_IsInBFF(void *addr);

// ===================================================================================
// I tend to stick these in a global header file, but they're not in the AM shell, so
// I've defined them here

#ifndef ADD2POINTER
	#define ADD2POINTER(a,b) ( (void *) ( (int)(a) + (b))  )
	#define SUBPOINTERS(a,b) (((int)((int)(a) - (int)(b))))
#endif
#ifndef CRASH
//	#define CRASH while(0==0)POLLHOST();
	#define CRASH while(0==0);
#endif

#endif
