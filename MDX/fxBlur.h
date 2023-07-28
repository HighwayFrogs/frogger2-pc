/*

	This file is part of the M libraries,

	File		: fxBlur.h
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef FXBLUR_H_INCLUDED
#define FXBLUR_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

void fxInitBlur(void);
void fxFreeBlur();
void fxBlurSurface(LPDIRECTDRAWSURFACE7 screen);

#ifdef __cplusplus
}
#endif

#endif