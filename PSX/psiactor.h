/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	actor.h			Skinned model control routines

************************************************************************************/


#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "fixed.h"

#include "map_draw.h"


#define ANIM_QUEUE_LENGTH	8
#define ANIMSHIFT			8

// Actor animation data structure
typedef struct
{
	short		numAnimations;
	short		currentAnimation;

	USHORT			*animSegments;			// list of start-end frames (shorts)

	u8		reachedEndOfAnimation;
	u8		loopAnimation;
	short 		numberQueued;

	long		animationSpeed;

	short		queueAnimation[ANIM_QUEUE_LENGTH];
	u8		queueLoopAnimation[ANIM_QUEUE_LENGTH];
	long		queueAnimationSpeed[ANIM_QUEUE_LENGTH];
	long		animTime;

	short		frame;

	short blendSpeed;
	short blendFrame;
	short blendStartFrame;
	short blendEndFrame;
	short oldAnimation;

	u8 		exclusive;
	u8 		spare;
	struct _SAMPLE **sfxMapping;	// Cue these sound effects off the animations

} ACTOR_ANIMATION;


// bounding box data structure
typedef struct
{

	long	minX,maxX;
	long	minY,maxY;
	long	minZ,maxZ;

} BOUNDINGBOX;


// Actor data structure
typedef struct _ACTOR
{
	struct _ACTOR	*next,*prev;

	USHORT					animFrames;				// number of animation frames
	USHORT					*animSegments;			// list of start-end frames (shorts)
	ACTOR_ANIMATION	animation;
	VECTOR					oldPosition;	//
	VECTOR					accumulator;	// for animation movement;
	SVECTOR					position;		// real position
  VECTOR					size; //mm only ever set to 1(4096)
	ULONG						radius;
	BOUNDINGBOX			bounding;//mm not used in PC
	PSIDATA					psiData;

	FVECTOR					vel;

 	SVECTOR					rotaim;//mm not used in the PSX at the moment, but may make it easier to determine direction 
 	ACTOR_SHADOW		*shadow; //mm added from PC  //mmshadow

	IQUATERNION			qRot;

	unsigned short	clutOverride;
	unsigned long		flags;

	MATRIX					bffMatrix;  // JH: Used to store the matrix of a scenic object.

} ACTOR;


typedef struct
{
	short animStart;
	short animEnd;
} ANIMATION;

// linked list of actors
typedef struct 
{

	ACTOR	head;
	int	 	numEntries;

}	ACTORLIST;

extern ACTORLIST	actorList;


//extern int MAXMODELS;
extern PSIMODEL **psiModelList;
extern long *psiModelListCRC;
extern int	psiModelListLen;
extern char *PSIname;

//reactive anims - normal frog actor store
//extern PSIDATA oldModel;
extern ACTOR oldActor;


// functions

void actorInitialise();

void actorAdd(ACTOR *actor);

void actorSub(ACTOR *actor);

void actorFree(ACTOR *actor);

//ACTOR *actorCreate(PSIMODEL *psiModel);
//ACTOR *actorCreate(PSIMODEL *psiModel, int checkForModel );
ACTOR *actorCreate(PSIMODEL *psiModel, int checkForModel, int scaleSkinned );

void actorInitAnim(ACTOR *actor);

void actorAdjustPosition(ACTOR *actor);

//PC AnimateActor(ACTOR *actor, int animNum, char loop, char queue, fixed speed, BYTE morphs,BOOL keepProportion)
void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed,   short blendSpeed);

void actorSetAnimationSpeed(ACTOR *actor, int speed);

void actorUpdateAnimations(ACTOR *actor);

void actorDrawAll();

void actorMove(ACTOR *actor);

void actorDraw(ACTOR *actor);

void actorDraw2(ACTOR *actor);

void actorDrawBones(ACTOR *actor);

void actorRotate(short angx, short angy, short angz, long movex, long movey, long movez, VECTOR *result);

//void actorSetAnimation(ACTOR *actor, ULONG frame);
void actorSetAnimation(ACTOR *actor, ULONG frame, int setKeys);

void actorSetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend);

int actorCalcSegments(ACTOR *actor);

void actorSetBoundingRotated(ACTOR *actor,int frame,int rotX,int rotY, int rotZ);

void actorSetBounding(ACTOR *actor,int frame);

u8 actorIsVisible(ACTOR *actor);

#define ACTORGETANIMFRAMESTART(actor,anim)	(actor)->animSegments[ (anim)*2]

#define ACTORGETANIMFRAMEEND(actor,anim)		(actor)->animSegments[((anim)*2)+1]

void ScalePsi(PSIMESH* pPSI);

void *ChangeModelNoUndo( ACTOR *actor, char *model );
void *ChangeModel( ACTOR *actor, char *model );
int UndoChangeModel( ACTOR *actor );

void StartAnimateActor(ACTOR *actor, int animNum, char loop, char queue, int speed, short blendSpeed);

//void CopyKeyFrames ( ACTOR *dest, ACTOR *src );
void CopyKeyFrames ( PSIOBJECT *dest, PSIOBJECT *src );

void FindFrogSubObjects( int p );
void UpdateFrogCroak( int pl  );

void actorSetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend);

#endif //__ACTOR_H__
