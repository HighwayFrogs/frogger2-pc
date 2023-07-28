/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: islFile.h
	Programmer	: David Swift
	Date		: 3 July 00

	Rewritten to emulate the ISL fileLoad routine for PSX - mostly just for flatpacker
	support!

----------------------------------------------------------------------------------------------- */

#ifndef MDXFILE_INCLUDED
#define MDXFILE_INCLUDED

unsigned char *mdxFileLoad(const char *file, const char *baseDir, int *length);

#endif

