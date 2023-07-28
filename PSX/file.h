/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islfile.c:		File handling

************************************************************************************/

#ifndef __ISLFILE_H__
#define __ISLFILE_H__


/**************************************************************************
	FUNCTION:	fileInitialise()
	PURPOSE:	Initialise file I/O
	PARAMETERS:	filesystem to use
	RETURNS:	
**************************************************************************/

void fileInitialise(char *fileSystem);

/**************************************************************************
	FUNCTION:	fileLoadBinary()
	PURPOSE:	Load in file to specific location (for overlays etc.)
	PARAMETERS:	Filename and location
	RETURNS:	0 if success, 1 if fail
**************************************************************************/

unsigned char fileLoadBinary(char *fName, char *loc);


/**************************************************************************
	FUNCTION:	fileLoad()
	PURPOSE:	Load in file
	PARAMETERS:	Filename, ptr to length var
	RETURNS:	Ptr to file data or NULL
**************************************************************************/

unsigned char *fileLoad(unsigned char *fName, int *length);


/**************************************************************************
	FUNCTION:	fileGetLastLoaded()
	PURPOSE:	Prints out the name and size of the last loaded file
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void fileGetLastLoaded();


// Just for Fred :)
extern char FILEIO_PCROOT[64];

void fileCDDATgetLocation(char *fName, long *sector, long *length);


#endif

