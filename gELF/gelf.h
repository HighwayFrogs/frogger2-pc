// -------------------------------------------------------------
// This file is part of GELF, (c) 1997 Interactive Studios Ltd.
//
// gelf.h - Top level code plus include other headers
// -------------------------------------------------------------

#ifndef __GELF_GELF_H
#define __GELF_GELF_H

// --------------------------------------------
// make sure we have any system headers we need

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <crtdbg.h>


// --------------------
// Constants and macros

#define gelfMalloc(SIZE, LABEL)		(gelf.mallocfunc(SIZE, LABEL))
#define gelfFree(BLOCKPTR)			(gelf.freefunc(BLOCKPTR))


// image storage formats
enum {
	GELF_IFORMAT_DEFAULT,
	GELF_IFORMAT_1BPP,
	GELF_IFORMAT_2BPP,
	GELF_IFORMAT_4BPP,
	GELF_IFORMAT_8BPP,
	GELF_IFORMAT_16BPP555,
	GELF_IFORMAT_16BPP565,
	GELF_IFORMAT_24BPPRGB,
	GELF_IFORMAT_24BPPBGR,
	GELF_IFORMAT_32BPPARGB,
	GELF_IFORMAT_32BPPABGR
};


// image data orientations
enum {
	GELF_IMAGEDATA_TOPDOWN,
	GELF_IMAGEDATA_BOTTOMUP
};


// --------------------
// Types and structures

//typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef signed char			schar;
typedef signed short		sshort;
typedef signed int			sint;
typedef signed long			slong;


// --------------------------
// include all format headers

#include "gelf_bmp.h"
#include "gelf_gif.h"
#include "gelf_iff.h"
#include "gelf_lbm.h"
#include "gelf_pcx.h"
#include "gelf_ppm.h"
#include "gelf_tga.h"




// ----------
// Prototypes

/*	--------------------------------------------------------------------------------
	Function : gelfInit
	Purpose : Initialise the GELF library
	Parameters : 
	Returns : 
	Info :
*/

void gelfInit();


/*	--------------------------------------------------------------------------------
	Function : gelfShutdown
	Purpose : Shutdown the GELF library
	Parameters : 
	Returns : 
	Info :
*/

void gelfShutdown();


/*	--------------------------------------------------------------------------------
	Function : gelfRegisterMalloc
	Purpose : register custom memory managment routines
	Parameters : malloc routine, free routine
	Returns : 
	Info :
*/

void gelfRegisterMalloc(void *(*mallocfunc)(int size, char *label), void (*freefunc)(void *ptr));


/*	--------------------------------------------------------------------------------
	Function : gelfRegisterError
	Purpose : register custom error handler
	Parameters : error handler
	Returns : 
	Info :
*/

void gelfRegisterError(void (*errorhandler)(char *format, ...));


/*	--------------------------------------------------------------------------------
	Function : gelfSetReadBufferSize
	Purpose : set the desired image read buffer size
	Parameters : new size in Kb (0 = default, currently 128Kb)
	Returns : 
	Info : sizes under 8Kb will be ignored
*/

void gelfSetReadBufferSize(int newsize);


/*	--------------------------------------------------------------------------------
	Function : gelfDefaultMalloc
	Purpose : default memory allocator
	Parameters : block size required, label for block
	Returns : ptr to memory, or NULL
	Info :
*/

void *gelfDefaultMalloc(int size, char *label);


/*	--------------------------------------------------------------------------------
	Function : gelfDefaultFree
	Purpose : default memory free routine
	Parameters : ptr to block
	Returns : 
	Info :
*/

void gelfDefaultFree(void *ptr);


/*	--------------------------------------------------------------------------------
	Function : gelfDefaultError
	Purpose : default error handler
	Parameters : as for printf
	Returns : 
	Info : sends messages to the debugger window
*/

void gelfDefaultError(char *format, ...);


/*	--------------------------------------------------------------------------------
	Function : gelfError
	Purpose : send a message to the error handler
	Parameters : as for printf
	Returns : 
	Info :
*/

void gelfError(char *format, ...);


/*	--------------------------------------------------------------------------------
	Function : gelfFormatToBPP
	Purpose : convert a desired image pixel format to a bits-per-pixel value
	Parameters : format ID, default BPP
	Returns : BPP
	Info :
*/

int gelfFormatToBPP(int format, int defbpp);


#endif	// __GELF_GELF_H
