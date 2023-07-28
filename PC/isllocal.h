/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	isllocal.c:		Localalised text handling

	Note: Taken from VSS, $/Tools/PSX Libraries/ISLLocal with minor alterations-ds

************************************************************************************/

#ifndef __ISLLOCAL_H_
#define __ISLLOCAL_H_

#ifdef __cplusplus
extern "C" {
#endif

// use this macro to access the string you want
#define TEXTSTR(n) (gameTextStr[n])


//extern void	**gameTextStr; --ds

extern char **gameTextStr;

/**************************************************************************
	FUNCTION:	gameTextInitFromFile
	PURPOSE:	Read in the text from a gametext file
	PARAMETERS:	Filename, num strings, number of languages, current language
	RETURNS:	
**************************************************************************/

void gameTextInit(char *fName, int numStrings, int numLang, int currLang);


/**************************************************************************
	FUNCTION:	gameTextDestroy
	PURPOSE:	Free up text
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void gameTextDestroy();

#ifdef __cplusplus
}
#endif

#endif

