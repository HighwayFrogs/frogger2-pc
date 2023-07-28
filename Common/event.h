/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.
	
	  
	File		: event.h
	Programmer	: Jim Hubbard
	Date		: 30/06/99
	
----------------------------------------------------------------------------------------------- */

#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED


/*----- [ DEFINES ] ----------------------------------------------------------------------------*/
#define TRIGGER_ONCE		(1<<0)
#define TRIGGER_DELAY		(1<<1)
#define TRIGGER_ALWAYS		(1<<2)
#define TRIGGER_FIRED		(1<<3)
#define TRIGGER_RISING		(1<<4)
#define TRIGGER_LOGIC		(1<<8)		// argument 1 is a trigger and must be freed!
#define TRIGGER_LOGIC2		(1<<9)		// argument 2 is a trigger and must be freed!
#define TRIGGER_FREE		(1<<15)		// This trigger is on the free list


#define MAX_EVENTDATA		4
#define TRIGGER_BLOCKSIZE	16
#define TRIGGER_NUMINITIAL	64


/*----- [ STRUCTURES ] -------------------------------------------------------------------------*/

/*
*	EVENT - Contains info on what happens when its parent trigger is activated
*/
typedef struct TAGEVENT
{
	void (*func) (struct TAGEVENT *), *data[MAX_EVENTDATA];

} EVENT;


/*
*	TRIGGER - Activates when a condition becomes true
*/
typedef struct TAGTRIGGER
{
	struct TAGTRIGGER *next, *prev;

	// Trigger parameters
	void *data[MAX_EVENTDATA];
	// Triggers only have one event these days
	EVENT event;
	// Trigger callback func
	int (*func)(struct TAGTRIGGER *);
	// Timing stuff
	unsigned short flags;
	unsigned long count;
	unsigned long delay;

} TRIGGER;


/*
*	TRIGGERBLOCK - Contains an array of triggers. Semi-dynamic allocation
*/
typedef struct TAGTRIGGERBLOCK
{
	struct TAGTRIGGERBLOCK *next, *prev;

	TRIGGER triggers[TRIGGER_BLOCKSIZE];

} TRIGGERBLOCK;


/*
*	TRIGGERLIST - Contains a large array of triggers. When these are used up it starts dynamically allocating trigger blocks.
*/
typedef struct
{
	// Static heads of the allocated and available lists
	TRIGGER head, avail;
	// Basic array of triggers
	TRIGGER *base;
	// Dynamically allocated blocks of triggers
	TRIGGERBLOCK *extra;

	short count;	// Current total number of active triggers
	short alloced;	// Maximum allocated triggers
	short blocks;	// Number of extra TRIGGERBLOCKs
	short garbage;	// Which array we're garbage collecting for this frame

} TRIGGERLIST;


#ifdef __cplusplus
extern "C" {
#endif

/*----- [ DATA ] ------------------------------------------------------------------------------*/

extern TRIGGERLIST triggerList;


/*----- [ PROTOTYPES ] ------------------------------------------------------------------------*/

TRIGGER * MakeTrigger( int (*func)(TRIGGER *t), void *arg1, void *arg2, void *arg3, void *arg4 );
void AttachEvent( TRIGGER *trigger, unsigned short flags, unsigned long time, 
				 void (*func)(EVENT *e), void *arg1, void *arg2, void *arg3, void *arg4 );

void KillAllTriggers( );
TRIGGER *AllocateTrigger( );
void DeallocateTrigger( TRIGGER *t );
void AllocTriggerBlock( );
void DeallocTriggerBlock( TRIGGERBLOCK *tb );

void InitTriggerList( );
void FreeTriggerList( );

void UpdateEvents( );
void TriggerGarbageCollect( );

#ifdef __cplusplus
}
#endif
#endif
