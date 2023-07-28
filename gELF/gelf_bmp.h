// -------------------------------------------------------------
// This file is part of GELF, (c) 1997 Interactive Studios Ltd.
//
// gelf_bmp.cpp - BMP file support
// -------------------------------------------------------------

#ifndef __GELF_BMP_H
#define __GELF_BMP_H


/*	--------------------------------------------------------------------------------
	Function : gelfInfo_BMP
	Purpose : Acquire image info from a BMP file
	Parameters : * filename,
				 * ptr for returned X size (can be NULL)
				 * ptr for returned Y size (can be NULL)
				 * ptr for returned source BPP (can be NULL)
	Returns : valid? (1=YES, 0=NO)
	Info : 
*/

int gelfInfo_BMP(char *fname, int *rxdim, int *rydim, int *rbpp);


/*	--------------------------------------------------------------------------------
	Function : gelfLoad_BMP
	Purpose : Load a BMP format image into memory
	Parameters : * filename,
				 * ptr to buffer (NULL for dynamic),
				 * ptr to palette ptr (*ptr should be NULL for dynamic, ((int)-1) for dont care)
				 * ptr for returned X size (can be NULL)
				 * ptr for returned Y size (can be NULL)
				 * ptr for returned source BPP (can be NULL)
				 * desired storage format
				 * desired data orientation
	Returns : ptr to image buffer, or NULL for failure (message sent to error handler)
	Info : can handle all known BMP formats, all colour depths
*/

void *gelfLoad_BMP(char *fname, void *imgbuf, void **palptr, int *rxdim, int *rydim, int *rbpp, int storeformat, int dataorient);



#endif	// __GELF_BMP_H
