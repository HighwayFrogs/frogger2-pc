/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: training.h
	Programmer	: David Swift
	Date		: 11 May 00

----------------------------------------------------------------------------------------------- */

#ifndef TRAINING_H_INCLUDED
#define TRAINING_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TRAININGMODE
{
	SPRITEOVERLAY *bg;
	TEXTOVERLAY *txtover[2];
	char text[1024];
} TRAININGMODE;

extern TRAININGMODE *train;

void InitTrainingMode();
void RunTrainingMode(void);
void TrainingHint(const char* str);

#ifdef __cplusplus
}
#endif

#endif