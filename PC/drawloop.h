/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: drawloop.h
	Programmer	: David Swift
	Date		: 16 Jun 00

----------------------------------------------------------------------------------------------- */

#ifndef DRAWLOOP_H_INCLUDED
#define DRAWLOOP_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

void CalcViewMatrix(long uDate);
long DrawLoop(void);

#ifdef __cplusplus
}
#endif

#endif
