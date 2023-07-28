/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	isllocal.c:		Localalised text handling

	Note: Taken from VSS, $/Tools/PSX Libraries/ISLLocal with minor alterations -ds

************************************************************************************/

//#include "..\islmem\islmem.h"
//#include "..\islfile\islfile.h"

#include <stdio.h>
#include "isllocal.h"

#include <islmem.h>
#include <islfile.h>

char		**gameTextStr;
static void *gameTextBuffer;

//static int		oldLang, currStr, incDelay, holdTime, bkg = 0;


/**************************************************************************
	FUNCTION:	gameTextInit
	PURPOSE:	Read in the text from a gametext file
	PARAMETERS:	Filename, no of strings, number of languages, current language
	RETURNS:	
**************************************************************************/

void gameTextInit(char *fName, int numStrings, int numLang, int currLang)
{
	char	*bufPtr, *compactBuf;
	int		loop, lang, len = 0;


	gameTextStr = (char **)MALLOC(sizeof(char*) * numStrings);

	gameTextBuffer = fileLoad(fName,0);
	bufPtr = (char *)gameTextBuffer;
	for(loop=0; loop<numStrings; loop++)
	{
		for(lang=-2; lang<numLang; lang++)
		{
			if (*bufPtr=='\"')
				bufPtr++;
			if (lang==currLang)
				gameTextStr[loop] = bufPtr;
			while((*bufPtr!=13) && (*bufPtr!=10) && (*bufPtr!='\t') && (*bufPtr!='\0'))
				bufPtr++;
			if (*(bufPtr-1)=='\"')
				*(bufPtr-1) = '\0';
			*bufPtr++ = '\0';
			if ((*bufPtr==13)||(*bufPtr==10))
				bufPtr++;
			if (lang==currLang)
				len += strlen(gameTextStr[loop])+1;	// add 1 for null terminator! - ds
		}
	}

	bufPtr = compactBuf = (char*)MALLOC(len+numStrings);
//	utilPrintf("small game text = %d\n", len+numStrings);
	for(loop=0; loop<numStrings; loop++)
	{
		strcpy(bufPtr, gameTextStr[loop]);
		gameTextStr[loop] = bufPtr;
		bufPtr += strlen(gameTextStr[loop])+1;
//		printf("[%d][%s]\n", loop, gameTextStr[loop]);
	}
	FREE(gameTextBuffer);
	gameTextBuffer = compactBuf;
	compactBuf = (char*)MALLOC(len+numStrings);
	memcpy(compactBuf, gameTextBuffer, len+numStrings);
	for(loop=0; loop<numStrings; loop++)
	{
		gameTextStr[loop] -= (unsigned long)gameTextBuffer;
		gameTextStr[loop] += (unsigned long)compactBuf;
	}
	FREE(gameTextBuffer);
	gameTextBuffer = compactBuf;
/*
	gameTextTimings = (int *)fileLoad("GAMETEXT.BIN");
	len = (TEXTSTR_MAX-TEXTSTR_MISSION_CITY1A)*4;
	compactBuf = MALLOC(len);
	memcpy(compactBuf, ((char *)gameTextTimings)+len*currLang, len);
	FREE(gameTextTimings);
	gameTextTimings = compactBuf;*/
}


/**************************************************************************
	FUNCTION:	gameTextDestroy
	PURPOSE:	Free up text
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void gameTextDestroy()
{
	FREE(gameTextBuffer);
	FREE(gameTextStr);
//	FREE(gameTextTimings);
}

