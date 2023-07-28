/*

	This file is part of the M libraries,

	File		: mliReport.h
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MGEINFO_H_INCLUDED
#define MGEINFO_H_INCLUDED

#ifdef __cplusplus

extern "C"
{
#endif

/*	--------------------------------------------------------------------------------
	Function	: ddShowCaps
	Purpose		: Prints out the capabilities held in a DDCAPS structure
	Parameters	: pointer to caps structure
	Returns		: nothing
	Info		: 
*/

void ddShowCaps(DDCAPS *me);

/*	--------------------------------------------------------------------------------
	Function	: ddShowError
	Purpose		: Converts a DirectDraw Error to a string describing the error, and prints it
	Parameters	: error value
	Returns		: 
	Info		: 
*/

void ddShowError(HRESULT error);

#ifdef __cplusplus
}
#endif

#endif