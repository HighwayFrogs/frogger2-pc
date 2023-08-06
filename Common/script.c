/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: script.c
	Programmer	: David Swift

----------------------------------------------------------------------------------------------- */

#include <islfile.h>
#include <islmem.h>
#include <stdio.h>
#include <islutil.h>

#include "codes.h"

#include "script.h"
#include "memload.h"

#include "evenfunc.h"
#include "actor2.h"
#include "enemies.h"
#include "platform.h"
#include "babyfrog.h"
#include "map.h"
#include "frogger.h"
#include "frogmove.h"
#include "sprite.h"
#include "anim.h"
#include "collect.h"
#include "game.h"
#include "newpsx.h"
#include "Main.h"
#include "maths.h"
#include "layout.h"
#include "cam.h"
#include "ptexture.h"
#include "training.h"
#include "lang.h"
#include "frontend.h"
#include "flatpack.h"

#ifdef PC_VERSION
#include <pcaudio.h>
#else
#include "audio.h"
#include "timer.h"
#include "temp_psx.h"
#endif

#define SCRIPT_FEV_VERSION	3
#define SCRIPT_HEADERSIZE (1+4)		// block starts after header + outer block size

/* --------------------------------------------------------------------------------- */

#define FS_SET_MOVE			1
#define FS_SET_STOP			2
#define FS_SET_TOGGLEMOVE	3
#define FS_SET_INVIS		4
#define FS_SET_VIS			5
#define FS_SET_TOGGLEVIS	6
#define FS_SET_DRAW			7
#define FS_SET_NODRAW		8
#define FS_SET_TOGGLEDRAW	9

#define SCRIPT_MAX_FLAGS	64
#define SCRIPT_FLAGS_BYTES	(SCRIPT_MAX_FLAGS/8)
#define SCRIPT_MAX_COUNTERS	16

unsigned char scriptFlags[SCRIPT_FLAGS_BYTES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
short scriptCounters[SCRIPT_MAX_COUNTERS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int lineNumber = 0;

#ifdef DEBUG_SCRIPTING

#define SCRIPT_VALIDATE_FLAG(x)		if ((x) < 0 || (x) > SCRIPT_MAX_FLAGS) { PrintScriptDebugMessage("Invalid toggle number"); return 0; }
#define SCRIPT_VALIDATE_COUNTER(x)	if ((x) < 0 || (x) > SCRIPT_MAX_COUNTERS) { PrintScriptDebugMessage("Invalid counter number"); return 0; }

#else

#define SCRIPT_VALIDATE_FLAG(x)
#define SCRIPT_VALIDATE_COUNTER(x)

#endif

typedef struct tagRUNANIMINFO
{
	int anim, loop, queue;
	int speed;
} RUNANIMINFO;

/* --------------------------------------------------------------------------------- */

TRIGGER *LoadTrigger(UBYTE**);
BOOL ExecuteCommand(UBYTE**);
void LoadTestScript(const char* filename);

void InterpretEvent( EVENT *e );
int ANDtrigger(TRIGGER *t);
int ORtrigger(TRIGGER *t);
int NOTtrigger(TRIGGER *t);
int AlwaysTrigger(TRIGGER *t);
int OnFlagSet(TRIGGER *t);
int OnCounterEquals(TRIGGER *t);

int EnumPlatforms(long id, int (*func)(PLATFORM*, int), int param);
int EnumEnemies(long id, int (*func)(ENEMY*, int), int param);

int SetEnemyFlag(ENEMY *e, int flag);
int ResetEnemyFlag(ENEMY *e, int flag);
int SetPlatformFlag(PLATFORM *p, int flag);
int ResetPlatformFlag(PLATFORM *p, int flag);
int CollapsePlatform(PLATFORM *p, int time);
int AnimateEnemy(ENEMY *e, int params);
int AnimatePlatform(PLATFORM *p, int params);

UBYTE* scriptBuffer;

void	*scriptFile;

typedef struct TAGSCRIPT_EFFECT_PARAMS
{
	int type;
	fixed size, speed, accn, lifetime;
	char clip;
} SCRIPT_EFFECT_PARAMS;

/* --------------------------------------------------------------------------------- */

//bb
//fixed volTest = ToFixed(0.5);
fixed volTest = 2048;

fixed ChangeVolume(int a, fixed delta)
{
	volTest += FMul(delta,gameSpeed);
	return volTest;
}


/*	--------------------------------------------------------------------------------
    Function		: PrintScriptDebugMessage
	Parameters		: const char* str
	Returns			: 
*/

#ifdef DEBUG_SCRIPTING

void PrintScriptDebugMessage(const char* str)
{
	if (lineNumber)
	{
#ifdef PC_VERSION
		sprintf(statusMessage, "(script) %s (%d)", str, lineNumber);
#endif
		utilPrintf("[Interpreter Debug] %s (line %d)\n", str, lineNumber);
	}
	else
	{
#ifdef PC_VERSION
		sprintf(statusMessage, "(script) %s", str);
#endif
		utilPrintf("[Interpreter Debug] %s\n", str);
	}
}

#else
#define PrintScriptDebugMessage(foo)
#endif
/*	--------------------------------------------------------------------------------
    Function		: GetTileFromNumber
	Parameters		: int
	Returns			: GAMETILE*
*/


#ifdef DEBUG_SCRIPTING
GAMETILE* GetTileFromNumber(int number)
{
/*	GAMETILE *tile;

	tile = firstTile;*/

	if ( number > tileCount )
	{
		char buf[40];
		sprintf(buf, "Invalid tile number: %d", number);
		PrintScriptDebugMessage(buf);
	}

	return &firstTile[number];

/*	for (tile = firstTile; number && tile; number--, tile = tile->next);
	if (!tile)
	{
		char buf[40];
		sprintf(buf, "Invalid tile number: %d", number);
		PrintScriptDebugMessage(buf);
	}
	return tile;*/
}

#else
#define GetTileFromNumber(i) (&firstTile[i])
#endif
/*	--------------------------------------------------------------------------------
    Function		: EnumPlaceholderTiles
	Parameters		: int, function
	Returns			: 
*/

int EnumPlaceholderTiles(long id, int (*func)(PATHNODE*, int), int param)
{
	ENEMY *cur;
	PATHNODE *p;
	int n, count = 0;

	for(cur = enemyList.head.next; cur != &enemyList.head; cur = cur->next, count++)
	{
		if (!id || cur->uid == id)
		{
			for (p = cur->path->nodes, n = cur->path->numNodes; n; p++, n--)
				if (!func(p, param)) break;
		}
	}

	return count;
}


int FrogOnPath(TRIGGER *t)
{
	PATHNODE *node;
	int n;
	int pl;
	PATH *p;
	
	pl = (int)t->data[0];
	
	// if true, we're either going to land on the tile shortly,
	// or we're double-jumping over it - skip the test
	if (player[pl].isSuperHopping) return 0;	

	p = (PATH*)t->data[1];

	for (node = p->nodes, n = p->numNodes; n; n--, node++)
		if (node->worldTile == currTile[pl])
			return 1;

	return 0;
}


/*	--------------------------------------------------------------------------------
    Function		: InterpretEvent
	Parameters		: EVENT*
	Returns			: 

	Passed to the Event/Trigger code as a function pointer in an EVENT. Parameter 0
	contains a pointer to a block of code which is sent to interpret()
*/

void InterpretEvent( EVENT *e )
{
	UBYTE *buffer = (UBYTE *)e->data[0];
	if (!Interpret(buffer))
		utilPrintf("Error running script within trigger!\n");
}

int ANDtrigger(TRIGGER *t)
{
	TRIGGER *a = (TRIGGER*)t->data[0], *b = (TRIGGER*)t->data[1];
	return a->func(a) && b->func(b);
}

int ORtrigger(TRIGGER *t)
{
	TRIGGER *a = (TRIGGER*)t->data[0], *b = (TRIGGER*)t->data[1];
	return a->func(a) || b->func(b);
}

int NOTtrigger(TRIGGER *t)
{
	TRIGGER *a = (TRIGGER*)t->data[0];
	return !a->func(a);
}

int OnFlagSet(TRIGGER *t)
{
	int f = (int)t->data[0];
	return (scriptFlags[f >> 3] & (1 << (f & 7))) != 0;
}

int OnCounterEquals(TRIGGER *t)
{
	int c = (int)t->data[0], v = (int)t->data[1];
	return (scriptCounters[c] == v);
}

/*	--------------------------------------------------------------------------------
    Function		: SetEnemy
	Parameters		: ENEMY*, int
	Returns			: 
*/
int SetEnemy(ENEMY *nme, int v)
{
	switch (v)
	{
	case FS_SET_MOVE:
		SetEnemyMoving(nme, 1);
		break;

	case FS_SET_STOP:
		SetEnemyMoving(nme, 0);
		break;
	
	case FS_SET_TOGGLEMOVE:
		SetEnemyMoving(nme, !nme->isWaiting);
		break;

	case FS_SET_INVIS:
		SetEnemyVisible(nme, 0);
		break;

	case FS_SET_VIS:
		SetEnemyVisible(nme, 1);
		break;

	case FS_SET_TOGGLEVIS:
		SetEnemyVisible(nme, !nme->active);
		break;

	case FS_SET_DRAW:
		if (nme->nmeActor)
			nme->nmeActor->draw = 1;
		break;

	case FS_SET_NODRAW:
		if (nme->nmeActor)
			nme->nmeActor->draw = 0;
		break;

	case FS_SET_TOGGLEDRAW:
		if (nme->nmeActor)
			nme->nmeActor->draw = !nme->nmeActor->draw;
		break;
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: SetPlatform
	Parameters		: PLATFORM*, int
	Returns			: 
*/
int SetPlatform(PLATFORM *plt, int v)
{
	switch (v)
	{
	case FS_SET_MOVE:
		SetPlatformMoving(plt, 1);
		break;

	case FS_SET_STOP:
		SetPlatformMoving(plt, 0);
		break;
	
	case FS_SET_TOGGLEMOVE:
		SetPlatformMoving(plt, (plt->isWaiting == -1));
		break;

	case FS_SET_INVIS:
		SetPlatformVisible(plt, 0);
		break;

	case FS_SET_VIS:
		SetPlatformVisible(plt, 1);
		break;

	case FS_SET_TOGGLEVIS:
		SetPlatformVisible(plt, !plt->active);
		break;

	case FS_SET_DRAW:
		plt->pltActor->draw = 1;
		MovePlatformToStartOfList(plt);
		break;

	case FS_SET_NODRAW:
		plt->pltActor->draw = 0;
		MovePlatformToEndOfList(plt);
		break;

	case FS_SET_TOGGLEDRAW:
		plt->pltActor->draw = !plt->pltActor->draw;
		break;
	}

	return 1;
}

int PathEffect(ENEMY *nme, int params)
{
	SCRIPT_EFFECT_PARAMS *p;
	PATHNODE *node = nme->path->nodes;
	SVECTOR v;
	FVECTOR up;
	SPECFX *fx;
	int c = nme->path->numNodes;
	
	(int)p = params;

	while (c--)
	{
		GetPositionForPathNode(&v, node);
		SetVectorFF( &up, &node->worldTile->normal );
		if( p->clip )
			fx = CreateSpecialEffect(p->type, &v, &up, p->size, p->speed, 0, p->lifetime);
		else
			fx = CreateSpecialEffectDirect(p->type, &v, &up, p->size, p->speed, 0, p->lifetime);

		if (fx) fx->gravity = p->accn;
		node++;
	}
	return 1;
}

int EnemyEffect(ENEMY *nme, int params)
{
	SCRIPT_EFFECT_PARAMS *p;
	SPECFX *fx;
	FVECTOR up;

	SetVectorFF( &up, &nme->inTile->normal );
	(int)p = params;
	if( p->clip )
		fx = CreateSpecialEffect(p->type, &nme->nmeActor->actor->position, &up, p->size, p->speed, 0, p->lifetime);
	else
		fx = CreateSpecialEffectDirect(p->type, &nme->nmeActor->actor->position, &up, p->size, p->speed, 0, p->lifetime);

	if (fx) fx->gravity = p->accn;
	return 1;
}

int SetTileState(PATHNODE *node, int state)
{
	node->worldTile->state = state;
	return 1;
}

/*	-------------------------------------------------------------------------------- */


int SetEnemyFlag(ENEMY *e, int flag)
{
	ENEMY_UPDATEFUNC *update;
	e->flags |= flag;
	for (update=e->Update; *update; update++)
		(*update)(e);
	return 1;
}

int ResetEnemyFlag(ENEMY *e, int flag)
{
	ENEMY_UPDATEFUNC *update;
	e->flags &= ~flag;
	for (update=e->Update; *update; update++)
		(*update)(e);
	return 1;
}

int SetPlatformFlag(PLATFORM *p, int flag)		{ p->flags |= flag;	p->Update(p); return 1; }
int ResetPlatformFlag(PLATFORM *p, int flag)	{ p->flags &= ~flag; p->Update(p); return 1; }

int CollapsePlatform(PLATFORM *p, int time)
{
	p->countdown = time + Random(time / 4); return 1;
}

int AnimateEnemy(ENEMY *e, int params)
{
	RUNANIMINFO *i = (RUNANIMINFO*)params;
	actorAnimate(e->nmeActor->actor, i->anim, i->loop, i->queue, i->speed, 0);
	return 1;
}

int AnimatePlatform(PLATFORM *p, int params)
{
	RUNANIMINFO *i = (RUNANIMINFO*)params;
	actorAnimate(p->pltActor->actor, i->anim, i->loop, i->queue, i->speed, 0);	
	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: LoadTrigger
	Parameters		: UBYTE*
	Returns			: TRIGGER*
*/
TRIGGER *LoadTrigger(UBYTE **p)
{
	TRIGGER *trigger = NULL;
	UBYTE token = MEMGETBYTE(p);
	UBYTE tmpByte;
	short tmpWord;

	switch (token)
	{
	case TR_ENEMYONTILE:
		{
			ENEMY *enemy;
			if (!(enemy = GetEnemyFromUID(MEMGETWORD(p)))) return 0;

			trigger = MakeTrigger( EnemyOnTile, (void *)enemy, (void*)GetTileFromNumber(MEMGETWORD(p)), NULL, NULL );
		}
		break;

	case TR_FROGONTILE:
		tmpByte = MEMGETBYTE(p);
		trigger = MakeTrigger( FrogOnTile, (void *)tmpByte, (void*)GetTileFromNumber(MEMGETWORD(p)), NULL, NULL );
		break;

	case TR_FROGONPLATFORM:
		tmpByte = MEMGETBYTE(p);
		trigger = MakeTrigger( FrogOnPlatform, (void *)tmpByte, (void*)MEMGETWORD(p), NULL, NULL );
		break;

	case TR_ENEMYATFLAG:
		tmpWord = MEMGETWORD(p);
		trigger = MakeTrigger( EnemyAtFlag, (void *)tmpWord, (void*)MEMGETWORD(p), NULL, NULL );
		break;

	case TR_PLATATFLAG:
		tmpWord = MEMGETWORD(p);
		trigger = MakeTrigger( PlatformAtFlag, (void *)tmpWord, (void*)MEMGETWORD(p), NULL, NULL );
		break;

	case TR_OR:
		{
			TRIGGER *a, *b;

			a = LoadTrigger(p);
			if (!a) return 0;

			b = LoadTrigger(p);
			if (!b) return 0;

			trigger = MakeTrigger( ORtrigger, (void*)a, (void*)b, NULL, NULL );
			trigger->flags |= TRIGGER_LOGIC|TRIGGER_LOGIC2;
		}
		break;

	case TR_AND:
		{
			TRIGGER *a, *b;

			a = LoadTrigger(p);
			if (!a) return 0;

			b = LoadTrigger(p);
			if (!b) return 0;

			trigger = MakeTrigger( ANDtrigger, (void*)a, (void*)b, NULL, NULL );
			trigger->flags |= TRIGGER_LOGIC|TRIGGER_LOGIC2;
		}
		break;

	case TR_NOT:
		{
			TRIGGER *a;
			a = LoadTrigger(p);
			if (!a) return 0;
			trigger = MakeTrigger( NOTtrigger, (void*)a, NULL, NULL, NULL );
			trigger->flags |= TRIGGER_LOGIC;
		}
		break;

	case TR_WAIT:
		{
			fixed time = MEMGETFIXED(p);
			trigger = MakeTrigger( OnTimeout, (void*)(actFrameCount + ((time * 60)>>12)), (void*)((time * 60 )>>12), NULL, NULL );
		}
		break;

	case TR_RANDOMWAIT:
		{
			unsigned long start, end;
			
			start = (MEMGETFIXED(p)*60)>>12;
			end = (MEMGETFIXED(p)*60)>>12;

			trigger = MakeTrigger( OnRandomTimeout, (void*)(actFrameCount + start + Random(end-start)), (void*)(start), (void *)(end), NULL );
		}
		break;

	case TR_FLAGSET:
		trigger = MakeTrigger( OnFlagSet, (void*)MEMGETBYTE(p), NULL, NULL, NULL );
		break;

	case TR_COUNTEREQUALS:
		tmpByte = MEMGETBYTE(p);
		trigger = MakeTrigger( OnCounterEquals, (void*)tmpByte, (void *)MEMGETWORD(p), NULL, NULL );
		break;
		
	case TR_FROGISDEAD:
		trigger = MakeTrigger( FrogIsDead, (void*)MEMGETBYTE(p), NULL, NULL, NULL );
		break;

	case TR_LEVELISOPEN:
		tmpByte = MEMGETBYTE(p);
		trigger = MakeTrigger( LevelIsOpen, (void*)tmpByte, (void *)MEMGETBYTE(p), NULL, NULL );
		break;

	case TR_ONPATH:
		{
			ENEMY *e;
			void *arg1 = (void*)MEMGETBYTE(p);
			
			if (!(e = GetEnemyFromUID(MEMGETWORD(p))))
				return 0;
			
			trigger = MakeTrigger( FrogOnPath, arg1, (void *)e->path, NULL, NULL );
			break;
		}

	default:
#ifdef DEBUG_SCRIPTING
		utilPrintf("Unrecognised trigger type %02x, skipping\n", token);
#endif
		return NULL;
	}

	return trigger;
}

/*	--------------------------------------------------------------------------------
    Function		: ExecuteCommand
	Parameters		: UBYTE*
	Returns			: TRUE if successful, FALSE otherwise

	NOTE! (*p) MUST now be incremented to the start of the next command
*/
BOOL ExecuteCommand(UBYTE **p)
{
	UBYTE command = MEMGETBYTE(p);

	switch (command)
	{
#ifdef DEBUG_SCRIPTING
	case EV_DEBUG:
		{
			char string[80];
			int len;

			len = MEMGETBYTE(p);
			memcpy(string, (*p), len);
			string[len] = 0;
			(*p) += len;

			PrintScriptDebugMessage(string);
			break;
		}
#endif

	case EV_SETENEMY:
		{
			long id = (long)MEMGETWORD(p);
			int foo = (int)MEMGETBYTE(p);
			if (EnumEnemies(id, SetEnemy, foo) == 0) return 0;
			break;
		}

	case EV_SETPLATFORM:
		{
			long id = (long)MEMGETWORD(p);
			int foo = (int)MEMGETBYTE(p);
			if (EnumPlatforms(id, SetPlatform, foo) == 0) return 0;
			break;
		}

	case EV_SETENEMYFLAG:
		{
			long id = (long)MEMGETWORD(p);
			unsigned flags = (unsigned)MEMGETINT(p);
			if (EnumEnemies(id, SetEnemyFlag, flags) == 0) return 0;
			break;
		}
		
	case EV_RESETENEMYFLAG:
		{
			long id = (long)MEMGETWORD(p);
			unsigned flags = (unsigned)MEMGETINT(p);
			if (EnumEnemies(id, ResetEnemyFlag, flags) == 0) return 0;
			break;
		}

	case EV_SETPLATFLAG:
		{
			long id = (long)MEMGETWORD(p);
			unsigned flags = (unsigned)MEMGETINT(p);
			if (EnumPlatforms(id, SetPlatformFlag, flags) == 0) return 0;
			break;
		}

	case EV_RESETPLATFLAG:
		{
			long id = (long)MEMGETWORD(p);
			unsigned flags = (unsigned)MEMGETINT(p);
			if (EnumPlatforms(id, ResetPlatformFlag, flags) == 0) return 0;
			break;
		}
	case EV_ANIMATEACTOR:
		{
			RUNANIMINFO i;
			int id, flags;

			id = MEMGETWORD(p);

			i.anim = MEMGETBYTE(p);
			flags = MEMGETBYTE(p);
			i.loop = (flags & 1);
			i.queue = (flags & 2);
			i.speed = MEMGETINT(p) >> 8;

			EnumEnemies(id, AnimateEnemy, (int)&i);
			EnumPlatforms(id, AnimatePlatform, (int)&i);
			break;
		}

	case EV_ON:
		{
			TRIGGER *t;
			int flags, size;

			if ((t = LoadTrigger(p)))
			{
				flags = MEMGETBYTE(p);
				size = MEMGETINT(p); 

				AttachEvent(t, (unsigned short)flags, 0, InterpretEvent, *p, NULL, NULL, NULL );

				(*p) += size;	// skip block of code!
			}
			else
				return 0;
			break;
		}

	case EV_IF:
		{
			TRIGGER *t;

			if((t = LoadTrigger(p)))
			{
				int size = MEMGETINT(p);

				if (t->func(t))
					Interpret(*p);

				DeallocateTrigger(t);
				
				(*p) += size;
			}
			else return 0;
			break;
		}

	case EV_MOVEENEMY:
		{
			long id = MEMGETWORD(p);
			int flag = MEMGETWORD(p);
			EnumEnemies(id, MoveEnemyToNode, flag);
			break;
		}

	case EV_MOVEPLAT:
		{
			long id = MEMGETWORD(p);
			int flag = MEMGETWORD(p);
			EnumPlatforms(id, MovePlatformToNode, flag);
			break;
		}

	case EV_CHANGEVOLUME:
		{
			fixed delta, amount;
			int n, platnum;
			PLATFORM *plt;
			SVECTOR u, v;

			n = MEMGETBYTE(p);
			delta = MEMGETFIXED(p);
			platnum = MEMGETWORD(p);
			amount = ChangeVolume(n, delta);

			plt = GetPlatformFromUID(platnum);
			if (!plt || (plt->path->numNodes != 2)) return 0;

			GetPositionForPathNode(&u, &plt->path->nodes[1]);
			GetPositionForPathNode(&v, &plt->path->nodes[0]);

			ScaleVector(&u, amount>>12);
			ScaleVector(&v, (4096-amount)>>12);
			AddVectorSSS(&plt->pltActor->actor->position, &u, &v);
			plt->isWaiting = -1;
			break;
		}

	case EV_CHANGELEVEL:
		{
			int world = MEMGETBYTE(p);
			int level = MEMGETBYTE(p);

			player[0].worldNum = world;
			player[0].levelNum = level;

			gameState.mode = LEVELCOMPLETE_MODE;
			gameState.multi = SINGLEPLAYER;

			GTInit( &modeTimer, 1 );
			showEndLevelScreen = 0;
			break;
		}

	case EV_FOG:
		{
			fog.r = MEMGETBYTE(p);
			fog.g = MEMGETBYTE(p);
			fog.b = MEMGETBYTE(p);
			fog.min = MEMGETWORD(p)*100;
			fog.max = MEMGETWORD(p)*100;
//			fog.mode = 1;
			break;
		}

	case EV_NO_FOG:	
		fog.mode = 0; 
		fog.max = worldVisualData[player[0].worldNum].levelVisualData[player[0].levelNum].farClip;
		break;

	case EV_CAMEOMODE_ON: cameoMode = 1; break;

	case EV_CAMEOMODE_OFF:
		{
			int pl;
			for (pl=0; pl<4; pl++)
				player[pl].canJump = 1;
			cameoMode = 0; break;
		}

	case EV_HOP:
		{
			int pl = MEMGETBYTE(p);
			GAMETILE *tile = GetTileFromNumber(MEMGETWORD(p));
			HopFrogToTile(tile, pl);
			break;
		}

	case EV_SETFLAG:
		{
			int flag = MEMGETBYTE(p);
			scriptFlags[flag >> 3] |= (1 << (flag & 7));
			break;
		}

	case EV_RESETFLAG:
		{
			int flag = MEMGETBYTE(p);
			scriptFlags[flag >> 3] &= ~(1 << (flag & 7));
			break;
		}

//	case EV_SFX_PLATFORM:
	case EV_SFX_ENEMY:
		{
			int id;
			SCRIPT_EFFECT_PARAMS params;

			id = MEMGETWORD(p);

			params.type = MEMGETBYTE(p);
			params.size = MEMGETFIXED(p);
			params.speed = MEMGETFIXED(p);
			params.accn = MEMGETFIXED(p);
			params.lifetime = MEMGETFIXED(p);
			params.clip = 1;

			EnumEnemies(id, EnemyEffect, (int)&params);
			break;
		}

	case EV_SFX_PLACEHOLDER:
		{
			int id;
			SCRIPT_EFFECT_PARAMS params;

			id = MEMGETWORD(p);

			params.type = MEMGETBYTE(p);
			params.size = MEMGETFIXED(p);
			params.speed = MEMGETFIXED(p);
			params.accn = MEMGETFIXED(p);
			params.lifetime = MEMGETFIXED(p);
			params.clip = 1;

			EnumEnemies(id, PathEffect, (int)&params);
			break;
		}

	case EV_SFX_P_NOCLIP:
		{
			int id;
			SCRIPT_EFFECT_PARAMS params;

			id = MEMGETWORD(p);

			params.type = MEMGETBYTE(p);
			params.size = MEMGETFIXED(p);
			params.speed = MEMGETFIXED(p);
			params.accn = MEMGETFIXED(p);
			params.lifetime = MEMGETFIXED(p);
			params.clip = 0;

			EnumEnemies(id, PathEffect, (int)&params);
			break;
		}

	case EV_SCALENMESPEED:
		{
			ENEMY *nme;
			fixed speed;

			nme = GetEnemyFromUID(MEMGETWORD(p));
			if (!nme) return 0;

			speed = MEMGETFIXED(p);

			ScalePathSpeed(nme->path, speed);
			break;
		}
			
	case EV_SCALEPLATSPEED:
		{
			PLATFORM *plat;
			fixed speed;

			plat = GetPlatformFromUID(MEMGETWORD(p));
			if (!plat) return 0;

			speed = MEMGETFIXED(p);

			ScalePathSpeed(plat->path, speed);
			break;
		}

	case EV_INC_COUNTER:
		{
			unsigned char counter = MEMGETBYTE(p);
			SCRIPT_VALIDATE_COUNTER(counter);
			scriptCounters[counter]++;
			break;
		}

	case EV_DEC_COUNTER:
		{
			unsigned char counter = MEMGETBYTE(p);
			SCRIPT_VALIDATE_COUNTER(counter);
			scriptCounters[counter]--;
			break;
		}

	case EV_SET_COUNTER:
		{
			unsigned char counter = MEMGETBYTE(p);
			SCRIPT_VALIDATE_COUNTER(counter);
			scriptCounters[counter] = MEMGETWORD(p);
			break;
		}

	case EV_COLLAPSE_PLAT:
		{
			int id = MEMGETWORD(p);
			int time = MEMGETFIXED(p) * 60;
			EnumPlatforms(id, CollapsePlatform, time);
			break;
		}

	case EV_CHANGEFROG:
		{
			char model[20];
			int pl, l;

			pl = MEMGETBYTE(p);
			l = MEMGETBYTE(p);

			if (l > 19) {
				PrintScriptDebugMessage("Model names must be < 20 chars");
				return 0;
			}
			memcpy(model, *p, l);
			model[l] = 0;
			(*p) += l;

			ChangeModel( frog[pl]->actor, model );
			actorAnimate( frog[pl]->actor, FROG_ANIM_BREATHE, YES, NO, FROG_BREATHE_SPEED, NO );
			break;
		}

	case EV_SHAKECAMERA:
		{
			cam_shakiness = MEMGETFIXED(p)*10;
			cam_shake_falloff = MEMGETFIXED(p)*10;
			break;
		}

	case EV_SETTILE_P:
		{
			int id = MEMGETWORD(p), state = MEMGETBYTE(p);
			EnumPlaceholderTiles(id, SetTileState, state);
			break;
		}

	case EV_SETSTARTTILE_P:
		{
			ENEMY *e;
			if (!(e = GetEnemyFromUID(MEMGETWORD(p)))) return 0;
			gTStart[0] = e->path->nodes->worldTile;
			startCamFacing = camFacing[0];
			startFrogFacing = frogFacing[0];
			break;
		}

	case EV_TELEPORT_P:
		{
			TRIGGER *t;
			int fNum, time;
			GAMETILE *tile;
			FVECTOR up;

			fNum = MEMGETBYTE(p);
			tile = GetEnemyFromUID(MEMGETWORD(p))->path->nodes->worldTile;
			time = ((MEMGETFIXED(p) * 60)>>12) + actFrameCount;

			if( player[fNum].frogState & FROGSTATUS_ISDEAD ) break;

			player[fNum].frogState &= ~FROGSTATUS_ALLHOPFLAGS;
			player[fNum].frogState |= FROGSTATUS_ISTELEPORTING;	// clear ALL other flags
			player[fNum].canJump = 0;
			FrogLeavePlatform(fNum);	// bah

#ifdef PC_VERSION
			if( dissolveTex && babiesSaved != numBabies )
			{
				((MDX_ACTOR *)frog[fNum]->actor->actualActor)->overrideTex = (MDX_TEXENTRY *)dissolveTex->tex;
				dissolveTex->active = 1;
			}
#endif

			SetVectorFF( &up, &currTile[0]->normal );
			CreateSpecialEffect( FXTYPE_FLASH, &frog[0]->actor->position, &up, 0, 12384, 512, 2048 );//16384, 1024, 2048 );
			PlaySample( genSfx[GEN_TELEPORT], NULL, 0, SAMPLE_VOLUME, -1 );
		
			t = MakeTrigger( OnTimeout, (void *)(time), NULL, NULL, NULL );
			AttachEvent( t, TRIGGER_ONCE, 0, TeleportFrog, (void *)fNum, (void *)tile, NULL, NULL );

			break;
		}

	case EV_SPRING_P:
		{
			GAMETILE *tile;	int pl;	fixed height, time;

			pl = MEMGETBYTE(p);
			tile = GetEnemyFromUID(MEMGETWORD(p))->path->nodes->worldTile;
			height = MEMGETFIXED(p)*10;
			time = MEMGETFIXED(p);

			SpringFrogToTile(tile, height, time, pl);

			break;
		}

	case EV_PLAYSOUND:
		{
			char name[32];
			short length, i=0;
			ENEMY *nme;
			PLATFORM *plat;
			SAMPLE* bbSamp;
			unsigned long uid;

			length = MEMGETBYTE(p);
			if( length > 31 )
			{
				utilPrintf("Sample name too long! Max 32 characters\n");
				return 0;
			}
			memcpy( name, *p, length );
			name[length] = '\0';
			(*p) += length;
			while( name[i] != '\0' && name[i] != '.' ) i++;
			name[i] = '\0';

			uid = MEMGETWORD(p);

// JH:  Did not compile, we may not need this
#ifdef PC_VERSION
			strlwr(name);
#endif
			if( !(bbSamp = FindSample(utilStr2CRC(name))) )
			{
				//PrintScriptDebugMessage("Couldn't find sample");	SHUT UP SHUT UP SHUT UP
				break;
			}

			if (!uid)
				PlaySample(bbSamp, NULL, 0, SAMPLE_VOLUME, -1);
			else if( (nme = GetEnemyFromUID(uid)) )
			{
				if( nme->nmeActor && nme->nmeActor->actor )
					PlaySample(bbSamp, &nme->nmeActor->actor->position, 0, SAMPLE_VOLUME, -1);
				else
					PlaySample(bbSamp, &nme->path->nodes->worldTile->centre, 0, SAMPLE_VOLUME, -1);
			}
			else if( (plat = GetPlatformFromUID(uid)) )
			{
				if( plat->pltActor && plat->pltActor->actor )
					PlaySample(bbSamp, &plat->pltActor->actor->position, 0, SAMPLE_VOLUME, -1);
				else
					PlaySample(bbSamp, &plat->path->nodes->worldTile->centre, 0, SAMPLE_VOLUME, -1);
			}
			else
				PrintScriptDebugMessage("PlaySound(): Couldn't find actor");

			break;
		}

	case EV_ANIMATEFROG:
		{
			int id, flags, anim, speed;

			id = MEMGETBYTE(p);

			anim = MEMGETBYTE(p);
			flags = MEMGETBYTE(p);
			speed = MEMGETINT(p) >> 8;

			actorAnimate(frog[id]->actor, anim, (flags & 1), (flags & 2), speed, 0);
			break;
		}

	case EV_KILLFROG:
		{
			int pl, num;

			pl = MEMGETBYTE(p);
			num = MEMGETBYTE(p);

			if (currPlatform[pl])
			{
				currPlatform[pl]->flags &= ~PLATFORM_NEW_CARRYINGFROG;
				currPlatform[pl]->carrying = 0;
				currPlatform[pl]=NULL;
			}

			player[pl].frogState |= FROGSTATUS_ISDEAD;

			if( num < NUM_DEATHTYPES )
				deathAnims[num] (pl);
			else
				deathAnims[DEATHBY_NORMAL] (pl);
//			player[pl].deathBy = DEATHBY_NORMAL;
//			GTInit(&player[pl].dead, time);
		}
		break;


	case EV_STOPSOUND:
		{
			char name[32];
			short length, i=0;
			SAMPLE* bbSamp;

			length = MEMGETBYTE(p);
			if( length > 31 )
			{
				utilPrintf("Sample name too long! Max 32 characters\n");
				return 0;
			}
			memcpy( name, *p, length );
			name[length] = '\0';
			(*p) += length;
			while( name[i] != '\0' && name[i] != '.' ) i++;
			name[i] = '\0';

// JH:  Did not compile, we may not need this
#ifdef PC_VERSION
			strlwr(name);
#endif
		
			if( (bbSamp = FindSample(utilStr2CRC(name))) )
				StopSample(bbSamp);


			break;
		}

	case EV_ENDLEVEL:
		{
//			UpdateCompletedLevel(player[0].worldNum,player[0].levelNum);
			gameState.mode = LEVELCOMPLETE_MODE;
			gameState.multi = SINGLEPLAYER;

			GTInit( &modeTimer, 8 );
			SetTimeForLevel();
			PrepareSong(AUDIOTRK_LEVELCOMPLETE, NO);
			StartLevelComplete();
			break;
		}

	case EV_RESETSCRIPT:
		{
			KillAllTriggers();
			Interpret(scriptBuffer + SCRIPT_HEADERSIZE);
			PrintScriptDebugMessage("Reset script");
			return 0;	// don't try to run any more commands!
		}

	case EV_POKEFROGS:
		{
			int pl;
			for (pl=0;pl<NUM_FROGS;pl++)
				CheckTileState(currTile[pl], pl); 
			break;
		}

	case EV_HINT:
		{
			int str = MEMGETWORD(p);
//			if (str>0)
			TrainingHint(GAMESTRING(str+STR_TRAINING1));
			break;
		}

#ifdef DEBUG_SCRIPTING
	case C_DEBUG:
		MEMGETWORD(p);	// blank
		lineNumber = MEMGETWORD(p);
		break;
#endif

	default:
#ifdef DEBUG_SCRIPTING
		PrintScriptDebugMessage("Unrecognised command");
#endif
		return 0;
	}

	return 1;
}			

/*	--------------------------------------------------------------------------------
    Function		: InitLevelScript
	Parameters		: void*
	Returns			: 1 for success, 0 for failure
*/
int InitLevelScript(void *buffer)
{
	int err = 0;

	KillAllTriggers();
	FreeLevelScript();

	scriptBuffer = (unsigned char*)buffer;
	
	if (*scriptBuffer != SCRIPT_FEV_VERSION)
	{
#ifdef DEBUG_SCRIPTING
		utilPrintf("Script failed version check\n");
#endif
		err = 1;
	}
	else if (!Interpret(scriptBuffer + SCRIPT_HEADERSIZE))
		err = 1;

	if (err)
	{
		FREE(scriptBuffer);
		scriptBuffer = NULL;
		KillAllTriggers();
		return 0;
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: FreeLevelScript
	Parameters		: void
	Returns			: 1 for success
*/
int FreeLevelScript(void)
{
	if(scriptBuffer)
	{
		FREE(scriptBuffer);
		scriptBuffer = NULL;
	}

	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: Interpret
	Parameters		: UBYTE*
	Returns			: TRUE if success, FALSE otherwise
*/
int Interpret(const UBYTE *buffer)
{
	UBYTE *p;//, *q;
	int events;

	p = (UBYTE*)buffer;

	events = MEMGETWORD(&p);

	while (events--)
	{
		if (!ExecuteCommand(&p))
		{
#ifdef DEBUG_SCRIPTING
			if (lineNumber)
				utilPrintf("Possibly an error on line %d?\n", lineNumber);
#endif

			return 0;
		}
	}

	return 1;
}


/*	--------------------------------------------------------------------------------
    Function		: LoadTestScript
	Parameters		: const char*
	Returns			: 
*/
int LoadLevelScript ( int worldID, int levelID )
{
	int fileLength;
	char fileName[256];

#ifdef PSX_VERSION
	sprintf ( fileName, "SCRIPT-%d-%d.FEV", worldID, levelID );

	scriptFile = FindStakFileInAllBanks ( fileName, &fileLength );
#endif

#ifdef PC_VERSION
	sprintf ( fileName, "MAPS\\SCRIPT-%d-%d.FEV", worldID, levelID );

	scriptFile = ( void * ) fileLoad ( fileName, &fileLength );
#endif

	utilPrintf("Loading script %s...\n", fileName);
	
	if ( !scriptFile )
	{
		utilPrintf("error loading file\n", fileName);
		return 0;
	}

	if ( !InitLevelScript ( scriptFile ) )
	{
		utilPrintf ( "InitLevelScript() failed\n" );
		return 0;
	}

	utilPrintf("ok\n");

	return 1;
}

/*	--------------------------------------------------------------------------------
    Function		: LoadTestScript
	Parameters		: const char*
	Returns			: 
*/
void LoadTestScript(const char* filename)
{
	UBYTE* buffer;
	
	utilPrintf("Testing script %s\n", filename);

	if (!(buffer = (UBYTE*)fileLoad(filename, 0)))
	{
#ifdef PC_VERSION
		sprintf(statusMessage, "Couldn't load script file %s", filename);
#endif
		utilPrintf("%s\n", statusMessage); return;
	}

	if (!InitLevelScript(buffer))
	{
#ifdef PC_VERSION
		sprintf(statusMessage, "Failed initialising script %s", filename);
#endif
		utilPrintf("%s\n", statusMessage); return;
	}

#ifdef PC_VERSION
	sprintf(statusMessage, "Loaded script %s", filename);
#endif
}


/*	--------------------------------------------------------------------------------
    Function		: InitScripting
	Parameters		: void
	Returns			: TRUE for success (currently ALWAYS returns true)
*/

int InitScripting(void)
{
	memset(scriptFlags, 0, SCRIPT_FLAGS_BYTES);
	return TRUE;
}

int AlwaysTrigger(TRIGGER *t) { return TRUE; }
