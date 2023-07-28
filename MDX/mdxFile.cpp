/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: islFile.h
	Programmer	: David Swift
	Date		: 3 July 00

	Rewritten to emulate the ISL fileLoad routine for PSX - mostly just for flatpacker
	support!

----------------------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <memory.h>
#include <windows.h>

#include "islfile.h"
#include "mdxException.h"

// FlatPacker magic number
#define FLA_MAGIC			0x32414c46			// Magic number 'FLA2' INTEL

#define	HISTORY_SIZE		4096				// flatpacker stuff
#define MASK_HISTORY		(HISTORY_SIZE-1)
#define MASK_UPPER			(0xF0)
#define MASK_LOWER			(0x0F)
#define SHIFT_UPPER			16
#define LSR_UPPER			4
#define MAX_COMP_LEN		17

// hmmm...

static unsigned char	*outputBufPtr;			// flatpacker temp storage
static int				outputBufLen;
static unsigned char	LZhistory[HISTORY_SIZE];
static unsigned short	LZhistoryOff;

/**************************************************************************
	FUNCTION:	DecompressOutputByte()
	PURPOSE:	decompresses a byte of data
	PARAMETERS:	the byte of data
	RETURNS:
**************************************************************************/

static inline void DecompressOutputByte(unsigned char data)
{
	*outputBufPtr++ = data;
	outputBufLen++;
	LZhistory[LZhistoryOff] = data;
	LZhistoryOff = (LZhistoryOff+1) & MASK_HISTORY;
}


/**************************************************************************
	FUNCTION:	utilDecompressBuffer()
	PURPOSE:	Decompress data from input buffer into output buffer
	PARAMETERS:	Input buffer start, Output buffer start
	RETURNS:	Length of uncompressed data
**************************************************************************/

int utilDecompressBuffer(unsigned char *inBuf, unsigned char *outBuf)
{
	unsigned short	tag, count, offset, loop;

	outputBufPtr = outBuf;								// Initialise output
	outputBufLen = 0;
	
	LZhistoryOff = 0;									// Clear history
	memset(LZhistory, 0, HISTORY_SIZE);

	while(1)
	{
		tag = *inBuf++;
		for(loop=0; loop!=8; loop++)
		{
			if (tag & 0x80)
			{
				if ((count=*inBuf++) == 0)
				{
					return outputBufLen;				// Finished now
				}
				else
				{										// Copy from history
					offset = HISTORY_SIZE-(((MASK_UPPER & count)*SHIFT_UPPER)+(*inBuf++));
					count &= MASK_LOWER;
					count += 2;
					while (count!=0)
					{
						DecompressOutputByte(LZhistory[(LZhistoryOff+offset) & MASK_HISTORY]);
						count--;
					}
				}
			}
			else
			{
				DecompressOutputByte(*inBuf++);			// Copy data byte
			}
			tag += tag;
		}
	}
	return outputBufLen;
}


BYTE *mdxFileLoad(const char *filename, const char *baseDirectory, int *bytesRead)
{
	BYTE *buffer;
	DWORD size, read;
	HANDLE h;

	if(baseDirectory)
		SetCurrentDirectory(baseDirectory);
	h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) return NULL;
	size = GetFileSize(h, NULL);
	buffer = (BYTE*)AllocMem(size);
	ReadFile(h, buffer, size, &read, NULL);
	CloseHandle(h);

	DWORD *data = (DWORD*)buffer;
	if (data[0] == FLA_MAGIC)	// file is compressed! make another buffer, decompress and return that
	{
		BYTE *dbuffer = (BYTE*)AllocMem(data[1]);
		read = utilDecompressBuffer(((BYTE*)buffer)+8, dbuffer);
		
		FreeMem(buffer);
		buffer = dbuffer;
	}

	if (bytesRead) *bytesRead = read;
	return buffer;
}

