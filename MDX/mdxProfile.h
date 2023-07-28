/*

	This file is part of the M libraries,

	File		: 
	Programmer	: Matthew Cloy
	Date		: 

----------------------------------------------------------------------------------------------- */

#ifndef MDXPROFILE_H_INCLUDED
#define MDXPROFILE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

void StartGraph(void);
void EndGraph(long maxVal);
void SampleGraph(long value);

void ClearTimers(void);
void HoldTimers(void);
void StartTimer(int number,char *name);
void EndTimer(int number);
void PrintTimers(void);
void InitProfile(void);


#ifdef __cplusplus
}
#endif

#endif