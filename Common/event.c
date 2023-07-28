/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.
	
	  
	File		: event.c
	Programmer	: Jim Hubbard
	Date		: 30/06/99

	Info		: A trigger has a condition function that is checked every iteration.
					It also has an array of associated data objects that are needed
					by the function, and a list of events whose action functions are 
					triggered when the condition function is true.

				: An event has a pointer to an action function, and an array of 
					associated data objects.

				: An event is associated with a trigger. More than one event can be
					attached to a trigger, and more than one trigger can have the same
					event attached to it.
	
----------------------------------------------------------------------------------------------- */

#include "event.h"
#include "types.h"
#include <islmem.h>
#include <islutil.h>
#include "Main.h"
#include "script.h"

TRIGGERLIST triggerList;
int triggersReset = 0;


/*	--------------------------------------------------------------------------------
	Function 	: CreateTrigger
	Purpose 	: Make a trigger structure 
	Parameters 	: Pointer to checking function, a number of pointers to data objects
	Returns 	: Created trigger structure
	Info 		:
*/
TRIGGER *MakeTrigger( int (*func)(TRIGGER *t), void *arg1, void *arg2, void *arg3, void *arg4 )
{
	TRIGGER *trigger = AllocateTrigger( );
	
	trigger->func = func;
	trigger->data[0] = arg1;
	trigger->data[1] = arg2;
	trigger->data[2] = arg3;
	trigger->data[3] = arg4;

	trigger->flags = 0;
	trigger->delay = 0;
	trigger->count = 0;

	return trigger;
}


/*	--------------------------------------------------------------------------------
	Function 	: AttachEvent
	Purpose 	: Adds trigger to list if it doesn't have any events attached already,
					and adds the event to a list in the trigger.
	Parameters 	: Trigger, event to attach to trigger
	Returns 	: 
	Info 		:
*/
void AttachEvent( TRIGGER *trigger, unsigned short flags, unsigned long time, 
				 void (*func)(EVENT *e), void *arg1, void *arg2, void *arg3, void *arg4 )
{
	// Set trigger vars
	trigger->flags |= flags;
	if( flags & TRIGGER_DELAY )
		trigger->delay = time;

	// Attach the event to the trigger
	trigger->event.func = func;
	trigger->event.data[0] = arg1;
	trigger->event.data[1] = arg2;
	trigger->event.data[2] = arg3;
	trigger->event.data[3] = arg4;

	// And add it to the active list
	trigger->prev = &triggerList.head;
	trigger->next = triggerList.head.next;
	triggerList.head.next->prev = trigger;
	triggerList.head.next = trigger;
}


/*	--------------------------------------------------------------------------------
	Function 	: InitTriggerList
	Purpose 	: Initialise the main list of triggers
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void InitTriggerList( )
{
	int i;

	// Set up circular list heads
	triggerList.head.next = triggerList.head.prev = &triggerList.head;
	triggerList.avail.next = triggerList.avail.prev = &triggerList.avail;

	// Various counters
	triggerList.count = 0;
	triggerList.alloced = TRIGGER_NUMINITIAL;
	triggerList.blocks = 0;
	triggerList.garbage = 0;

	// Alloc base array
	triggerList.base = (TRIGGER *)MALLOC0( sizeof(TRIGGER)*TRIGGER_NUMINITIAL );
	triggerList.extra = NULL;

	// Add all initial triggers to free list
	for( i=0; i<TRIGGER_NUMINITIAL; i++ )
	{
		triggerList.base[i].prev = &triggerList.avail;
		triggerList.base[i].next = triggerList.avail.next;
		triggerList.avail.next->prev = &triggerList.base[i];
		triggerList.avail.next = &triggerList.base[i];

		triggerList.base[i].flags |= TRIGGER_FREE;
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: FreeTriggerList
	Purpose 	: Kill all triggers and deallocate list data
	Parameters 	: 
	Returns 	: 
	Info 		:
*/
void FreeTriggerList( )
{
	TRIGGERBLOCK *tb = triggerList.extra, *next;

	// First, remove all active triggers
	KillAllTriggers( );

	// Then deallocate data
	while( tb )
	{
		next = tb->next;
		FREE( tb );
		tb = next;
	}

	if( triggerList.base )
		FREE( triggerList.base );

	// And reinit some variables so it don't crash
	triggerList.count = 0;
	triggerList.alloced = 0;
	triggerList.blocks = 0;
	triggerList.garbage = 0;

	triggerList.head.next = triggerList.head.prev = &triggerList.head;
	triggerList.avail.next = triggerList.avail.prev = &triggerList.avail;
}


/*	--------------------------------------------------------------------------------
	Function 	: KillAllTriggers
	Purpose 	: Remove all triggers from list
	Parameters 	: 
	Returns 	: 
	Info 		: Deletes the triggers, not just removes them. Check your data pointers!
*/
void KillAllTriggers( )
{
	//utilPrintf("Freeing trigger list (%d entries)\n", triggerList.count);

	while( triggerList.head.next && triggerList.head.next != &triggerList.head )
		DeallocateTrigger( triggerList.head.next );

	triggersReset = 1;
}


/*	--------------------------------------------------------------------------------
	Function 	: AllocateTrigger
	Purpose 	: Get a trigger from the array. If we don't have any left then alloc a new block
	Parameters 	: 
	Returns 	: 
*/
TRIGGER *AllocateTrigger( )
{
	TRIGGER *t;

	// If we need more triggers then allocate a block
	if( triggerList.count >= triggerList.alloced )
		AllocTriggerBlock( );

	// Get the next one off the free list
	t = triggerList.avail.next;
	triggerList.avail.next = t->next;
	triggerList.avail.next->prev = &triggerList.avail;

	// Detach from all lists - will get added to aactiv list if or when an event is attached to it.
	// If no event then it is a logic trigger (AND,OR,etc) and will get freed separately.
	t->next = t->prev = NULL;

	triggerList.count++;

#ifdef DEBUG_EVENT
	utilPrintf( "ALLOC: Trigger count is %i of %i\n", triggerList.count, triggerList.alloced );
#endif

	t->flags &= ~TRIGGER_FREE;

	return t;
}


/*	--------------------------------------------------------------------------------
	Function 	: DeallocateTrigger
	Purpose 	: Remove trigger from list and add to available listt
	Parameters 	: trigger to remove
	Returns 	: 
*/
void DeallocateTrigger( TRIGGER *t )
{
	if( !t || (t->flags & TRIGGER_FREE) ) return;

	// Remove from active list
	if( t->prev ) t->prev->next = t->next;
	if( t->next ) t->next->prev = t->prev;

	// Add to free list
	t->prev = &triggerList.avail;
	t->next = triggerList.avail.next;
	t->next->prev = t;
	triggerList.avail.next = t;

	t->flags |= TRIGGER_FREE;
	triggerList.count--;

#ifdef DEBUG_EVENT
	utilPrintf( "FREE: Trigger count is %i of %i\n", triggerList.count, triggerList.alloced );
#endif

	// Do we need to kill extra logic bits?
	if (t->flags & TRIGGER_LOGIC)
		DeallocateTrigger((TRIGGER*)t->data[0]);
		
	if (t->flags & TRIGGER_LOGIC2)
		DeallocateTrigger((TRIGGER*)t->data[1]);
}


/*	--------------------------------------------------------------------------------
	Function 	: AllocTriggerBlock
	Purpose 	: Make a new block of triggers and add to the list
	Parameters 	: 
	Returns 	: 
*/
void AllocTriggerBlock( )
{
	int i;
	TRIGGERBLOCK *tb = (TRIGGERBLOCK *)MALLOC0( sizeof(TRIGGERBLOCK) );

	// Add the newly malloced block to the extra blocks list
	tb->next = triggerList.extra;
	tb->prev = NULL;
	if( triggerList.extra ) triggerList.extra->prev = tb;

	triggerList.blocks++;
	triggerList.alloced += TRIGGER_BLOCKSIZE;

	// Add all the new elements to the free list
	for( i=0; i<TRIGGER_BLOCKSIZE; i++ )
	{
		tb->triggers[i].prev = &triggerList.avail;
		tb->triggers[i].next = triggerList.avail.next;
		triggerList.avail.next->prev = &tb->triggers[i];
		triggerList.avail.next = &tb->triggers[i];

		tb->triggers[i].flags |= TRIGGER_FREE;
	}
}


/*	--------------------------------------------------------------------------------
	Function 	: DeallocTriggerBlock
	Purpose 	: When a block is unused and is garbage checked, remove all its items from the free list and dealloc.
	Parameters 	: 
	Returns 	: 
*/
void DeallocTriggerBlock( TRIGGERBLOCK *tb )
{
	int i;

	// Remove from extras list
	if (tb->prev) tb->prev->next = tb->next;
	if (tb->next) tb->next->prev = tb->prev;

	// Remove all triggers from free list
	for( i=0; i<TRIGGER_BLOCKSIZE; i++ )
	{
		tb->triggers[i].prev->next = tb->triggers[i].next;
		tb->triggers[i].next->prev = tb->triggers[i].prev;
	}

	triggerList.blocks--;
	triggerList.alloced -= TRIGGER_BLOCKSIZE;

	FREE( tb );
}


/*	--------------------------------------------------------------------------------
	Function 	: TriggerGarbageCollect
	Purpose 	: Check current array for being totally unused. Delete if no active triggers.
	Parameters 	: 
	Returns 	: 
*/
void TriggerGarbageCollect( )
{
	// TODO
}


/*	--------------------------------------------------------------------------------
	Function 	: UpdateEvents
	Purpose 	: Manage events for this turn
	Parameters 	: 
	Returns 	: 
	Info 		: 
*/
void UpdateEvents( )
{
	TRIGGER *trigger, *next;
	int count = 0;

	triggersReset = 0;

	for( trigger = triggerList.head.next; trigger != &triggerList.head; trigger = next, count++)
	{
		next = trigger->next;

		/* Check if the trigger condition(s) are true and the trigger is allowed to fire */
		if( !((trigger->flags & TRIGGER_DELAY) && trigger->count) && trigger->func(trigger) )
		{
			/* rising-edge triggers fire when the trigger is not previously fired */
			if (!(trigger->flags & TRIGGER_RISING && trigger->flags & TRIGGER_FIRED))
			{
				if( trigger->event.func )
					trigger->event.func(&trigger->event);

				// we need to skip trying to do anything else this gameloop if the triggers were reset!
				if (triggersReset)
					return;
			}

			trigger->flags |= TRIGGER_FIRED; // Flag for other triggers to check if needed

			if( trigger->flags & TRIGGER_ONCE ) // if "trigger once", kill this trigger
				DeallocateTrigger(trigger);
			else if( trigger->flags & TRIGGER_DELAY ) 
				trigger->count = trigger->delay;
		}
		else
		{
			if (trigger->count) trigger->count--;

			trigger->flags &= ~TRIGGER_FIRED;
		}
	}
}
