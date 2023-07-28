/************************************************************************************
	ACTION MAN PS	(c) 1998-9 ISL

	timer.h:		Debug timing functions

************************************************************************************/

#ifndef _TIMER_H_
#define _TIMER_H_

#define TIMER_SET 0

//*** TIMER SET 0 - GAME LOOP ***/
#if TIMER_SET==0
#define TIMER_SET_0
#define TIMER_START0(x) 	 TIMER_START(x)
#define TIMER_STOP0(x)  	 TIMER_STOP(x)
#define TIMER_STOP_ADD0(x) TIMER_STOP_ADD(x)
#else
#define TIMER_START0(x)
#define TIMER_STOP0(x)
#define TIMER_STOP_ADD0(x)
#endif

//*** TIMER SET 0 - DrawActorList ***//
#if TIMER_SET==1
#define TIMER_SET_1
#define TIMER_START1(x)    TIMER_START(x)
#define TIMER_STOP1(x) 	   TIMER_STOP(x)
#define TIMER_STOP_ADD1(x) TIMER_STOP_ADD(x)
#else
#define TIMER_START1(x)
#define TIMER_STOP1(x)
#define TIMER_STOP_ADD1(x)
#endif

//*** TIMER SET 2 - actorDraw ***/
#if TIMER_SET==2
#define TIMER_SET_2
#define TIMER_START2(x) 	 TIMER_START(x)
#define TIMER_STOP2(x)  	 TIMER_STOP(x)
#define TIMER_STOP_ADD2(x)   TIMER_STOP_ADD(x)
#else
#define TIMER_START2(x)
#define TIMER_STOP2(x)
#define TIMER_STOP_ADD2(x)
#endif

//*** TIMER SET 3 - RunGameLoop ***/
#if TIMER_SET==3
#define TIMER_SET_3
#define TIMER_START3(x) 	 TIMER_START(x)
#define TIMER_STOP3(x)  	 TIMER_STOP(x)
#define TIMER_STOP_ADD3(x)   TIMER_STOP_ADD(x)
#else
#define TIMER_START3(x)
#define TIMER_STOP3(x)
#define TIMER_STOP_ADD3(x)
#endif

//*** TIMER SET 4 - PsiActor2ClipCheck ***/
#if TIMER_SET==4
#define TIMER_SET_4
#define TIMER_START4(x) 	 TIMER_START(x)
#define TIMER_STOP4(x)  	 TIMER_STOP(x)
#define TIMER_STOP_ADD4(x)   TIMER_STOP_ADD(x)
#else
#define TIMER_START4(x)
#define TIMER_STOP4(x)
#define TIMER_STOP_ADD4(x)
#endif



//*** TIMER SET 0 - MAIN LOOP ***//
#ifdef TIMER_SET_0
enum {
	TIMER_TOTAL,
	TIMER_GAMELOOP,
	TIMER_DRAW_WORLD,
	TIMER_DRAW_SPECFX,
	TIMER_PRINT_SPRITES,
	TIMER_DRAW_SCENICS,
	TIMER_ACTOR_DRAW,
	TIMER_PRINT_OVERS,
	TIMER_PROCTEX,
	TIMER_DRAWSYNC,
	TIMER_TIMERS,
	TIMER_UPDATETEXANIM,

	TIMER_NUMTIMERS
};

#define TIMER_NAMES	   	"TOTAL", \
						"GAMELOOP", \
						"DRAW_WORLD", \
						"DRAW_SPECFX", \
						"PRINT_SPRITES", \
						"DRAW_SCENICS", \
						"ACTOR_DRAW", \
						"PRINT_OVERS", \
						"PROCTEX", \
						"DRAWSYNC", \
						"TIMERS", \
						"UPDATE_TEX_ANIM"
#endif //TIMER_SET_0


//*** TIMER SET 1 - DrawActorList ***//
#ifdef TIMER_SET_1
enum {
	TIMER_TOTAL,
	TIMER_PSICLIP,
	TIMER_HEADCHK,
	TIMER_PSIBACK,
	TIMER_PSIDRAW,
	TIMER_FMACLIP,
	TIMER_SPHERE,
	TIMER_FMADRAW,
	TIMER_TIMERS,

	TIMER_NUMTIMERS
};

#define TIMER_NAMES	   	"TOTAL", \
						"PSICLIP", \
						"HEADCHK", \
						"PSIBACK", \
						"PSIDRAW", \
						"FMACLIP", \
						"SPHERE", \
						"FMADRAW", \
						"TIMERS"
#endif //TIMER_SET_1


//*** TIMER SET 2 - actorDraw ***//
#ifdef TIMER_SET_2
enum {
	TIMER_TOTAL,
	TIMER_ACTDR_QUAT,
	TIMER_ACTDR_MTX,
	TIMER_ACTDR_SEG,
	TIMER_TIMERS,

	TIMER_NUMTIMERS
};

#define TIMER_NAMES	   	"TOTAL", \
						"QUAT", \
						"MTX", \
						"SEG", \
						"TIMERS"
#endif //TIMER_SET_2



//*** TIMER SET 3 - Game Loop ***//
#ifdef TIMER_SET_3
enum {
	TIMER_TOTAL,
	TIMER_CAM_POS,
	TIMER_SET_CAM,
	TIMER_FROG_POS,
	TIMER_UP_PLAT,
	TIMER_UP_ENEM,
	TIMER_UP_SPEC,
	TIMER_UP_EVEN,
	TIMER_UP_AMBI,
	TIMER_COLLECT,
	TIMER_ONSCREEN,
	TIMER_TILENUM,
	TIMER_TIMERS,

	TIMER_NUMTIMERS
};

#define TIMER_NAMES	   	"TOTAL", \
						"CAM_POS", \
						"SET_CAM", \
						"FROG_POS", \
						"UP_PLAT", \
						"UP_ENEM", \
						"UP_SPEC", \
						"UP_EVEN", \
						"UP_AMBI", \
						"COLLECT", \
						"ONSCREEN", \
						"TILENUM", \
						"TIMERS"
#endif //TIMER_SET_3




//*** TIMER SET 4 - PsiActo2ClipCheck ***//
#ifdef TIMER_SET_4
enum {
	TIMER_TOTAL,
	TIMER_UPANI,
	TIMER_SETANI,
	TIMER_TIMERS,

	TIMER_NUMTIMERS
};

#define TIMER_NAMES	   	"TOTAL", \
						"UPANI", \
						"SETANI", \
						"TIMERS"
#endif //TIMER_SET_4









/*
//for RunGameLoop
enum {
	TIMER_TOTAL,
	TIMER_PROC_CONT,
	TIMER_CAM_POS,
	TIMER_SET_CAM,
	TIMER_FROG_POS,
	TIMER_UP_PLAT,
	TIMER_UP_ENEM,
	TIMER_UP_SPEC,
	TIMER_UP_EVEN,
	TIMER_UP_AMBI,
	TIMER_COLLECT,
	TIMER_ONSCREEN,
	TIMER_TILENUM,
	TIMER_TIMERS,

	TIMER_NUMTIMERS
};


#define TIMER_NAMES	   	"TOTAL", \
						"PROC_CONT", \
						"CAM_POS", \
						"SET_CAM", \
						"fROG_POS", \
						"UP_PLAT", \
						"UP_ENEM", \
						"UP_SPEC", \
						"UP_EVEN", \
						"UP_AMBI", \
						"COLLECT", \
						"ONSCREEN", \
						"TILENUM", \
						"TIMERS"
*/						







extern char	timerActive;
extern char	*timerName[TIMER_NUMTIMERS];
extern int	globalTimer[TIMER_NUMTIMERS];
extern int	startTimer[TIMER_NUMTIMERS];
extern int	prevTimer[TIMER_NUMTIMERS];


/**************************************************************************
	FUNCTION:	TIMER_START()
	PURPOSE:	Start timing
	PARAMETERS:	Timer number
	RETURNS:	
**************************************************************************/

//#define TIMER_START(n)		globalTimer[n] = VSync(1)
#define TIMER_START(n)		startTimer[n] = VSync(1)


/**************************************************************************
	FUNCTION:	TIMER_STOP()
	PURPOSE:	Stop timing
	PARAMETERS:	Timer number
	RETURNS:	
**************************************************************************/

//#define TIMER_STOP(n)		globalTimer[n] = VSync(1)-globalTimer[n]
#define TIMER_STOP(n)		globalTimer[n] = VSync(1)-startTimer[n]

#define TIMER_STOP_ADD(n)	globalTimer[n] += VSync(1)-startTimer[n]


/**************************************************************************
	FUNCTION:	TIMER_ENDFRAME()
	PURPOSE:	Start timing
	PARAMETERS:	Timer number
	RETURNS:	
**************************************************************************/

#define TIMER_ENDFRAME		memcpy(prevTimer, globalTimer, sizeof(globalTimer))

//need to zero, cos globalTimer may not be reset if only using TIMER_STOP_ADD
#define TIMER_ZERO			memset(globalTimer, 0, sizeof(globalTimer))


/**************************************************************************
	FUNCTION:	timerDisplay()
	PURPOSE:	Display timer readings if enabled
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void timerDisplay();


#endif
