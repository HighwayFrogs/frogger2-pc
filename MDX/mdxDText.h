/*

	This file is part of the M libraries,

	File		: mliReport.h
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXDTEXT_H_INCLUDED
#define MDXDTEXT_H_INCLUDED

#ifdef __cplusplus

extern "C"
{
#endif

void cp(char *format, ...);
void PrintConsole(void);
void UpdateConsole(int charCode);

#ifdef __cplusplus
}
#endif

#endif