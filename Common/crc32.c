/*
	This file is part of ObjEdit2, Copyright 1997 Interactive Studios Ltd

	File        : crc32.cpp
	Description : CRC generating code
	Author      : R. Hackett
	Date        : 15.02.97
*/

//#include <string.h>
#include "crc32.h"

#ifndef PSX_VERSION
#include "string.h"
#endif

#define POLYNOMIAL 0x04c11db7L
#define CRC_MAGIC_NUMBER 'ÿ'

static unsigned long CRCtable[256];

unsigned long objectCRC = 0;

/*	--------------------------------------------------------------------------------
	Function : InitCRCTable
	Purpose : Initialise the CRC table
	Parameters : 
	Returns : 
	Info : 
*/

void InitCRCTable()
{
	register int i, j;
	register unsigned long CRCaccum;

	for (i=0; i<256; i++)
		{
		CRCaccum = ((unsigned long)i<<24);
		for (j=0; j<8; j++)
			{
			if (CRCaccum & 0x80000000L)
				CRCaccum = (CRCaccum<<1)^POLYNOMIAL;
			else
				CRCaccum = (CRCaccum<<1);
			}
		CRCtable[i] = CRCaccum;
		}
}


/*	--------------------------------------------------------------------------------
	Function : UpdateCRC
	Purpose : update a CRC for specified data
	Parameters : existing CRC, data ptr, size of data in bytes
	Returns : 
	Info : 


	Modified 08/07/99 by David Swift
	To allow "Magic number" strings starting with 0xFF followed by a hex number
	which is directly returned.
*/

unsigned long UpdateCRC(char *ptr)
{
	register int i, j;
//	int code;
	int size = strlen(ptr);
	unsigned long CRCaccum = 0;

	/* If the first byte is a "magic number", return the global CRC value thing */
	if (*ptr == CRC_MAGIC_NUMBER)
	{
		return objectCRC;
	}
	else
	{
		for (j=0; j<size; j++)
			{
			i = ((int)(CRCaccum>>24)^(*ptr++))&0xff;
			CRCaccum = (CRCaccum<<8)^CRCtable[i];
			}
		return CRCaccum;
	}
}

/*	--------------------------------------------------------------------------------
	Function : UpdateCRC
	Purpose : update a CRC for specified data
	Parameters : existing CRC, data ptr, size of data in bytes
	Returns : 
	Info : 
*/
unsigned long UpdateCRCData(char *ptr,int numBytes)
{
	register int i, j;
	unsigned long CRCaccum = 0;

	for (j=0; j<numBytes; j++)
		{
		i = ((int)(CRCaccum>>24)^(*ptr++))&0xff;
		CRCaccum = (CRCaccum<<8)^CRCtable[i];
		}
	return CRCaccum;
}
