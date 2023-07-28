/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: sndview.h
	Programmer	: James Healey
	Date		: 21/04/98

----------------------------------------------------------------------------------------------- */

#ifndef SNDVIEW_H_INCLUDED
#define SNDVIEW_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

extern void RunSndView ( void );


#ifdef N64_VERSION

extern unsigned long sfxNum;
extern unsigned long musNum;

#endif

#ifdef __cplusplus
}
#endif

#endif