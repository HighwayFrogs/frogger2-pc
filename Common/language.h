/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: language.h
	Programmer	: James Healey
	Date		: 31/03/99

----------------------------------------------------------------------------------------------- */

#ifndef LANGUAGE_H_INCLUDED
#define LANGUAGE_H_INCLUDED

#define NUM_LANGUAGES	5

#include "lang.h"
#ifdef __cplusplus


extern "C"
{
#endif

extern char *DIKStrings[256][LANG_NUMLANGS];

#ifdef __cplusplus
}
#endif

#endif
