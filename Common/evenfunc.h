/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.
	
	  
	File		: event.h
	Programmer	: Jim Hubbard
	Date		: 30/06/99
	
----------------------------------------------------------------------------------------------- */

#ifndef EVENTFUNCS_H_INCLUDED
#define EVENTFUNCS_H_INCLUDED

#include "maths.h"
#include "define.h"
#include "event.h"

typedef struct TAG_SPRINGINFO
{
	SVECTOR S, V, H;
	GAMETILE *dest;
	int start, end, frog;
} SPRINGINFO;

#ifdef __cplusplus
extern "C" {
#endif

/*----- [ TRIGGER PROTOTYPES ] -----------------------------------------------------------------*/
extern int EnemyOnTile( TRIGGER *trigger );
extern int FrogOnTile( TRIGGER *trigger );
extern int FrogOnPlatform( TRIGGER *trigger );
extern int ActorWithinRadius( TRIGGER *trigger );
extern int OnTimeout( TRIGGER *trigger );
extern int OnRandomTimeout( TRIGGER *trigger );
extern int OnTrigger( TRIGGER *trigger );
extern int LogicalAND( TRIGGER *trigger );
extern int LogicalOR( TRIGGER *trigger );
extern int FrogOnFrog( TRIGGER *trigger );
extern int EnemyAtFlag( TRIGGER *trigger );
extern int PlatformAtFlag( TRIGGER *trigger );
extern int ActorNearActor( TRIGGER *trigger );
extern int PathAtFlag( TRIGGER *trigger );
extern int BitCheck( TRIGGER *trigger );
extern int FrogIsDead( TRIGGER *trigger );
extern int LevelIsOpen( TRIGGER *trigger );
extern int EnemyAtFlag(TRIGGER *trigger);
extern int PlatformAtFlag(TRIGGER *trigger);

/*----- [ EVENT PROTOTYPES ] -------------------------------------------------------------------*/
extern void ChangeActorScale( EVENT *event );
extern void AssignFloatToFloat( EVENT *event );
extern void AssignIntToInt( EVENT *event );
extern void TogglePlatformMove( EVENT *event );
extern void ToggleEnemyMove( EVENT *event );
extern void ToggleTileLink( EVENT *event );
extern void PlaySFX( EVENT *event );
extern void EvAnimateActor( EVENT *event );
extern void TeleportFrog( EVENT *event );
extern void DeathAnim( EVENT *event );

#ifdef __cplusplus
}
#endif
#endif