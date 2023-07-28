#ifndef __DECOMP_H__
#define __DECOMP_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************

   ISL Flatpacker II decompression                             R.Hackett 1997

   (c) Copyright Interactive Studios Ltd.

********************************************************************************

   FLA file format:

   0     DWORD    Magic number 'FLA2'
   4     DWORD    Original file length (Intel-endian)
   8-    -        Compressed data

   Remember to check file header for BOTH magic number and length before
   calling decompression routine or it will crash!  Sometimes FLA files are
   merely copies of the original where losses would be made post-compression.

*******************************************************************************/


// TYPE DEFINITIONS FOR MS VISUAL C
#ifndef UBYTE
typedef unsigned char	UBYTE;			// Unsigned byte (8-bits)
#endif
//#ifndef WORD
//typedef short			WORD;			// Signed word (16-bits)
//#endif

/*	--------------------------------------------------------------------------------
	FUNCTION:	DecompressBuffer
	PURPOSE:	Decompress data from input buffer into output buffer
	PARAMETERS:	Input buffer start, Output buffer start
	RETURNS:	Length of uncompressed data
	INFO:
*/

int DecompressBuffer(UBYTE *inBuf, UBYTE *outBuf);

//#ifndef PC_VERSION
//void memset(char *source, char val, int size);
//#endif



#ifdef __cplusplus
}
#endif

#endif
