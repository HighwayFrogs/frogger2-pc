/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islfile.c:		File handling

************************************************************************************/

#include <sys/types.h>
#include <libcd.h>
#include <libsn.h>
#include <stdio.h>
#include <string.h>
#include <islmem.h>
#include <islutil.h>
#include "islfile.h"

// FlatPacker magic number
#define FLA_MAGIC			0x32414c46			// Magic number 'FLA2' INTEL

#ifdef _DEBUG
char FILEIO_PCROOT[64];		//	"C:\\PSX\\MYGAME\\CD\\"
#else
char FILEIO_CDINDEX[64];	//	"\\MYGAME.DAT;1"
#endif


unsigned char fileIndex[8*2048];
typedef struct _FileIODataType
{
	int				DATsector;
	unsigned char	*index;
	unsigned char	CDresult[8];
	char			bkgFilename[256];
	int				bkgLoading;
	unsigned char	*bkgBuffer;
} FileIODataType;


FileIODataType	fileIO;
static char		sectorBuf[2048];

static char		lastFileLoaded[64];
static int		lastFileSize;


/* PC READING CODE *********************************************************************************/

#ifdef _DEBUG

/**************************************************************************
	FUNCTION:	filePCInitialise()
	PURPOSE:	Initialise PC file I/O
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void filePCInitialise(char *fileSystem)
{
	strcpy(FILEIO_PCROOT, fileSystem);
	if (PCinit())
		printf("Error initialising PC filing\n");
}


/**************************************************************************
	FUNCTION:	filePCLoad()
	PURPOSE:	Load in file from PC
	PARAMETERS:	Filename
	RETURNS:	Ptr to file data or NULL
**************************************************************************/

static unsigned char *filePCLoad(char *fName, int *length)
{
	long	fHandle, size;
	unsigned char	*buffer;
	char	pathName[256], *ptr;
	unsigned long	*lPtr;

	while(1)
	{
		// get full path name
		strcpy(pathName, FILEIO_PCROOT);
		strcat(pathName, fName);
		// try to open file
		if ((fHandle = PCopen(pathName, 0,0))==-1)
		{
			printf("Error opening PC file '%s'\n", pathName);
			return NULL;
		}
		// get first couple of words in file
		size = PClseek(fHandle, 0,2);
		PClseek(fHandle, 0,0);
		if (PCread(fHandle, sectorBuf, 8)!=8)
		{
			printf("Error reading PC file '%s'\n", fName);
			PCclose(fHandle);
			continue;
		}
		lPtr = (unsigned long *)sectorBuf;
		PClseek(fHandle, 0,0);
		// does it start with the magic number?
		if (lPtr[0]==FLA_MAGIC)
		{
			// allocate uncompressed amount
			buffer = MALLOC((((lPtr[1]+1024)>>11)+1)<<11);		// round up to next 2k boundary
			if(buffer)
			{
				ptr = buffer+(lPtr[1]-size)+1024;
				PCread(fHandle, ptr, size);
				utilDecompressBuffer(ptr+8, buffer);
				*length = lPtr[1];
			}
			else
			{
				printf("Error allocating PC file '%s' (FlatPacked)\n", fName);
				PCclose(fHandle);
				continue;
			}
		}
		else
		{
			// file is not compressed, so just allocate file size
			if ((buffer = MALLOC(((size>>11)+1)<<11))==NULL)
			{
				printf("Error allocating PC file '%s'\n", fName);
				PCclose(fHandle);
				continue;
			}
			if (PCread(fHandle, buffer, size)!=size)
			{
				printf("Error reading PC file '%s'\n", fName);
				FREE(buffer);
				PCclose(fHandle);
				continue;
			}
			*length = size;
		}
		PCclose(fHandle);
		break;
	}
	return buffer;
}


/**************************************************************************
	FUNCTION:	filePCLoadBinary()
	PURPOSE:	Load in file to specific location (for overlays etc.)
	PARAMETERS:	Filename and location
	RETURNS:	0 if success, 1 if fail
**************************************************************************/

static unsigned char filePCLoadBinary(char *fName, char *loc)
{
	long	fHandle, size;
	char	pathName[256];

	printf("\nLoading binary code file %s\n",fName);

	strcpy(pathName, FILEIO_PCROOT);
	strcat(pathName, fName);
	if ((fHandle = PCopen(pathName, 0,0))==-1)
	{
		printf("Error opening PC file '%s'\n", pathName);
		return 1;
	}
	size = PClseek(fHandle, 0,2);
	PClseek(fHandle, 0,0);
	
	if (PCread(fHandle, loc, size)!=size)
	{
		printf("Error reading PC file '%s'\n", fName);
		PCclose(fHandle);
		return 1;
	}
	PCclose(fHandle);

	return 0;
}


#endif


/* CD READING CODE *********************************************************************************/

#ifndef _DEBUG


/**************************************************************************
	FUNCTION:	fileCDreadIndex()
	PURPOSE:	Read index sectors from CD
	PARAMETERS:	Filename of DAT file
	RETURNS:	>0 if errors
**************************************************************************/

static int fileCDreadIndex(char *fName)
{
	CdlFILE	fp;

	if (CdSearchFile(&fp, fName)==NULL)									// Find file location
	{
		printf("Reading CD index: CdSearchFile() error\n");
		return 1;
	}
	fileIO.DATsector = CdPosToInt(&fp.pos);
	if (CdReadFile(fName, (unsigned long *)fileIO.index, 8*2048)==0)			// Read file
	{
		printf("Reading CD index: CdReadFile() error\n");
		return 2;
	}
	if (CdReadSync(0, fileIO.CDresult))										// Wait for read
	{
		printf("Reading CD index: Read error\n");
		return 3;
	}
	return 0;
}


/**************************************************************************
	FUNCTION:	fileCDInitialise()
	PURPOSE:	Initialise CD file I/O
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void fileCDInitialise(char *fileSystem)
{
	strcpy(FILEIO_CDINDEX, fileSystem);
	XAsetStatus(CdInit());

	fileIO.index = &fileIndex[0];//MALLOC(8*2048);

	while(fileCDreadIndex(FILEIO_CDINDEX))
	{
		printf("Problem reading CD index - retry\n");
	}

	printf("Read CD index\n");
}


/**************************************************************************
	FUNCTION:	fileCDDATgetLocation()
	PURPOSE:	Get location info from CD index
	PARAMETERS:	Filename, *first sector, *file length
	RETURNS:	
**************************************************************************/

void fileCDDATgetLocation(char *fName, long *sector, long *length)
{
	long	crc, *iPtr;
	char	name[40];
	int		loop;

	strcpy(name, fName);
	utilUpperStr(name);
	crc = utilStr2CRC(name);
	iPtr = (int *)fileIO.index;
	loop = iPtr[1];
	iPtr += 2;
//	printf("Looking for '%s' (0x%x) %d files\n", name, crc, loop);
	while(loop>0)
	{
		if (iPtr[0]==crc)
		{
			*sector = iPtr[1]+fileIO.DATsector+8;
			*length = iPtr[2];
//			printf("Found file %d [%d]\n", *sector, *length);
			return;
		}
		iPtr += 3;
		loop--;
	}
	*sector = *length = -1;
	printf("'%s' not found in DAT file\n", fName);
}


/**************************************************************************
	FUNCTION:	fileCDGetLocation()
	PURPOSE:	Get location info from CD index
	PARAMETERS:	Index, filename, *first sector, *file length, *file number
	RETURNS:	>0 if errors
**************************************************************************

static long fileCDGetLocation(unsigned char *index, char *filespec, long *firstsec, long *size, long *number)
{
	long	noof, min,max, pos, ret;
	BYTE	*cp, *ep;

	noof = *(long*)index;
	index += 4;
	cp = (BYTE*)strrchr((char *)filespec, '\\');
	if(cp == NULL)
		cp = filespec;
	else
		cp++;
	min = 0;
	max = noof-1;
	for(;;)
	{
		pos = (min + max) >> 1;
		ep = index + (pos << 5);
		ret = strcmp(ep, cp);
		if(ret == 0)
			break;
		if(min == max)
		{
			printf("Error file %s not in DAT\n", filespec);
			return 1;
		}
		if(ret<0)
			min = (min != pos) ? pos : pos + 1;
		else
			max = pos;
	}
	*firstsec = *(long*)(ep + 16) + fileIO.DATsector;
	*size = *(long*)(ep + 20);
	if((long)number!=0)
		*number = pos;
	return 0;
}*/

/**************************************************************************
	FUNCTION:	fileCDloadDATfile()
	PURPOSE:	Load in file from CD DAT file
	PARAMETERS:	Filename, buffer
	RETURNS:	Ptr to data or NULL
**************************************************************************/

static unsigned long *fileCDloadDATfile(char *fName, int *length)
{
	long	sec, sz;
	CdlFILE	fp;
	unsigned char	*buffer, *ptr;
	unsigned long	*lPtr;
	int		rtn;

	fileCDDATgetLocation(fName, &sec, &sz);
	
	if (sec==-1)
		return NULL;

	CdIntToPos(sec, &fp.pos);

	if (CdControl(CdlSetloc, (unsigned char *)&fp.pos, fileIO.CDresult)==0)		// Seek to file
	{
		printf("Load file: CDControl() error\n");
		return NULL;
	}

	if (fileIO.CDresult[0]==0)											// Check seek OK
	{
		printf("Load file: Seek error\n");
		return NULL;
	}

	if (CdRead(1, (unsigned long *)sectorBuf, CdlModeSpeed)==0)					// Start read
	{
		printf("Load file: CdRead() error\n");
		return NULL;
	}

	lPtr = (unsigned long *)sectorBuf;

	while((rtn=CdReadSync(1, fileIO.CDresult))>0);						// Wait for first sector
	
	if (rtn<0)
	{
		printf("Load file: CdReadSync() error\n");
		return NULL;
	}

	CdIntToPos(sec, &fp.pos);

	if (CdControl(CdlSetloc, (unsigned char *)&fp.pos, fileIO.CDresult)==0)		// Seek to file
	{
		printf("Load file: CDControl() error\n");
		return NULL;
	}
	
	if (fileIO.CDresult[0]==0)											// Check seek OK
	{
		printf("Load file: Seek error\n");
		return NULL;
	}

	if (lPtr[0]==FLA_MAGIC)												// Check compression
	{																	// Compressed file
		buffer = MALLOC(lPtr[1]+2048);
		
		if (!buffer)
		{
			printf("Can't malloc buffer space for file read: %s\n",fName);
			return NULL;
		}
		
		ptr = buffer+(lPtr[1]-(((sz>>11)+1)<<11))+2048;
		
		if (CdRead((sz>>11)+1, ptr, CdlModeSpeed)==0)					// Start read
		{
			printf("Load file: CdRead() error\n");
			FREE(buffer);
			return NULL;
		}
		
		while((rtn=CdReadSync(1, fileIO.CDresult))>0);					// Wait for whole file
		
		if (rtn<0)
		{
			printf("Load file: CdReadSync()[2] error\n");
			FREE(buffer);
			return NULL;
		}

		utilDecompressBuffer(ptr+8, buffer);
		*length = lPtr[1];
	}
	else
	{																	// Uncompressed file
		if ((buffer = MALLOC(((sz>>11)+1)<<11))==NULL)			// Get RAM
		{
			printf("Load file: Not enough RAM\n");
			return NULL;
		}

		if (CdRead((sz>>11)+1, buffer, CdlModeSpeed)==0)				// Start read
		{
			printf("Load file: CdRead() error\n");
			FREE(buffer);
			return NULL;
		}

		while((rtn=CdReadSync(1, fileIO.CDresult))>0);					// Wait for whole file

		*length = sz;
	}

	return buffer;
}


/**************************************************************************
	FUNCTION:	fileCDcheckLoaded()
	PURPOSE:	Load in file from CD
	PARAMETERS:	
	RETURNS:	0 = Ready	<0 = Errors		>0 = Not ready yet
**************************************************************************/

static int fileCDcheckLoaded()
{
	return CdReadSync(1, fileIO.CDresult);
}


/**************************************************************************
	FUNCTION:	fileCDLoad()
	PURPOSE:	Load in file from CD
	PARAMETERS:	Filename
	RETURNS:	Ptr to file data or NULL
**************************************************************************/

static unsigned char *fileCDLoad(char *fName, int *length)
{
	unsigned long	*buffer;

	while (1)
	{
		if ((buffer = fileCDloadDATfile(fName, length))!=NULL)
		{
				return (unsigned char *)buffer;
		}		
		printf("Problem reading file - retry\n");
	}
	return (unsigned char *)buffer;
}


/**************************************************************************
	FUNCTION:	fileCDloadDATbinary()
	PURPOSE:	Load in file from CD DAT file
	PARAMETERS:	Filename, buffer
	RETURNS:	Ptr to data or NULL
**************************************************************************/

static unsigned char fileCDloadDATbinary(char *fName, int *loc)
{
	long	sec, sz;
	CdlFILE	fp;
	int		rtn;
	unsigned char	*ptr;

	fileCDDATgetLocation(fName, &sec, &sz);
	if (sec==-1)
		return 1;
	CdIntToPos(sec, &fp.pos);
	if (CdControl(CdlSetloc, (unsigned char *)&fp.pos, fileIO.CDresult)==0)		// Seek to file
	{
		printf("Load file: CDControl() error\n");
		return 1;
	}
	if (fileIO.CDresult[0]==0)											// Check seek OK
	{
		printf("Load file: Seek error\n");
		return 1;
	}
	if (CdRead((sz>>11), (unsigned long *)loc, CdlModeSpeed)==0)				// Start read
	{
		printf("Load file: CdRead() error\n");
		return 1;
	}

	while((rtn=fileCDcheckLoaded())>0);									// Wait for read

	CdIntToPos(sec+(sz>>11), &fp.pos);
	if (CdControl(CdlSetloc, (unsigned char *)&fp.pos, fileIO.CDresult)==0)		// Seek to file
	{
		printf("Load file: CDControl() error\n");
		return 1;
	}
	if (fileIO.CDresult[0]==0)											// Check seek OK
	{
		printf("Load file: Seek error\n");
		return 1;
	}
	if (CdRead(1, (unsigned long *)sectorBuf, CdlModeSpeed)==0)					// Start read last bit
	{
		printf("Load file: CdRead() error\n");
		return 1;
	}

	while((rtn=fileCDcheckLoaded())>0);									// Wait for read

	ptr = loc;
	ptr += ((sz>>11)<<11);
	memcpy(ptr, sectorBuf, sz & 0x7ff);
	return 0;
}

#endif



/* EXTERNAL INTERFACE ******************************************************************************/

#ifdef _DEBUG

void fileGetLastLoaded()
{
	printf("\nLast file: %s (%d bytes)\n",lastFileLoaded, lastFileSize);
	memoryGetLastAlloc();
}

#endif


/**************************************************************************
	FUNCTION:	fileInitialise()
	PURPOSE:	Initialise file I/O
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void fileInitialise(char *fileSystem)
{
#ifndef _DEBUG
	fileCDInitialise(fileSystem);
#else
	filePCInitialise(fileSystem);
#endif
	
	lastFileLoaded[0] = 0;
	lastFileSize = 0;
}


/**************************************************************************
	FUNCTION:	fileLoad()
	PURPOSE:	Load in file
	PARAMETERS:	Filename, ptr to length var
	RETURNS:	Ptr to file data or NULL
**************************************************************************/

unsigned char *fileLoad(unsigned char *fName, int *length)
{
	unsigned char	*data;
	int		len;

	utilUpperStr(fName);

#ifndef _DEBUG
	data = fileCDLoad(fName, &len);
#else
	data = filePCLoad(fName, &len);
#endif
	if (data==NULL)
		return NULL;

/*
	lPtr = (unsigned long *)data;
	if (lPtr[0] == FLA_MAGIC)
	{
		realData = MALLOC(lPtr[1]);
		if (realData==NULL)
			printf("Can't allocate memory for file read: %s\n",fName);
		else
		{
			utilDecompressBuffer(data+8, realData);
			FREE(data);
			printf("READ FILE: %s [compressed]\n", fName);
			return realData;
		}
	}
*/

	printf("READ FILE: %s\n", fName);
	if (length!=NULL)
		*length = len;

	// quick fix so people know what the hell is getting loaded and how big
	strcpy(lastFileLoaded, fName);
	lastFileSize = len;

	return data;
}


/**************************************************************************
	FUNCTION:	fileLoadBinary()
	PURPOSE:	Load in file to specific location (for overlays etc.)
	PARAMETERS:	Filename and location
	RETURNS:	0 if success, 1 if fail
**************************************************************************/

unsigned char fileLoadBinary(char *fName, char *loc)
{
#ifndef _DEBUG
	return fileCDloadDATbinary(fName, loc);
#else
	return filePCLoadBinary(fName, loc);
#endif
}


