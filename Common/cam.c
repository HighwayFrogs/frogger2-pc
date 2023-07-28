/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.
	
	  
	File		: cam.c
	Programmer	: Andrew Eder
	Date		: 11/10/98
	
----------------------------------------------------------------------------------------------- */

//mm #include "ultra64.h"
//mm #include "directx.h"
#include "types.h"

#include "islpsi.h"
#include <islmem.h>
#include <islutil.h>
#include <islpad.h>

#include "frogmove.h"
#include "cam.h"
#include "maths.h"
#include "frogger.h"
#include "main.h"
#include "game.h"
#include "newpsx.h"
#include "frontend.h"
#include "layout.h"
#include "multi.h"


int camControlMode = 0;

FVECTOR cameraUpVect		= { 0,4096,0 };
FVECTOR	camVect;

FVECTOR currCamSource	= { 0, 		0, 		 -	100<<12 	};
FVECTOR currCamTarget	= { 0, 		0, 		 	0 			};
FVECTOR currCamDist		= { 0, 		0, 		 	10<<12 		};
FVECTOR camSource		= { 0, 		569<<12,	-200<<12 	};
FVECTOR camTarget;

/*
VECTOR	actualCamSource[2];
VECTOR	actualCamTarget[2];
*/

// in floaty camera mode, the offset of the camera from the player
FVECTOR currCamOffset, camOffset = {0, 100<<12, 100<<12};

fixed	xFOVNew				= 5461;
fixed	yFOVNew				= 184320;
fixed	xFOV				= 5461;
fixed	yFOV				= 184320;
fixed	precScaleFactor		= 4096;

fixed camLookOfs			= 50<<12;
fixed camLookOfsNew			= 50<<12;

SVECTOR camDist				= {0,680*SCALE,192*SCALE};


fixed camSpeed				= 15<<12; //30; //9;
fixed camSpeed2				= 20<<12;; //25; //9;
fixed camSpeed3				= 20<<12;	// source & target interp.
//fixed camSpeed4				= 15<<12;; //25; //8;
fixed fovSpd				= 2<<12;;
fixed transCamSpeedMult		= 4096;

int	camFacing[4]			= {0,0,0,0};
short startCamFacing = 0, startFrogFacing = 0;
fixed scaleV				= 4506;

char controlCamera			= 0;
char fixedDir = 0;
char fixedPos = 0;
char firstPerson = 0;
char fixedUp = 0;

//extern long idleCamera;
VECTOR idleCamDist	= {0,100,102};

fixed sideSwaySpeed = 20, sideSwayAmt=50<<12, swayModifier = 4096;

fixed camSideOfs = 0;
GAMETILE *lastTile[4] = {0,0,0,0};
long prevCamFacing[4] = {0,0,0,0};

CAM_BOX_LIST cameraBoxes;
CAM_BOX *currCamBox;
fixed cam_edge_spacing = 100<<12;

fixed cam_shakiness = 0;
fixed cam_shake_falloff = 410;

char playerFocus = 0;

TRANSCAMERALIST tcList;

// functions

fixed FindMaxInterFrogDistance( );
void CameraSetSource(void);
int CameraBoundPosition(VECTOR *v, CAM_BOX *box, fixed edge);
void CalcSPCamera( FVECTOR *target );
void SlurpCamPosition( );

int GetCamLimitVector(FVECTOR *out, FVECTOR *v, CAM_BOX *box, fixed edge)
{
	int p;
	fixed dist;
	CAM_PLANE *plane;
	FVECTOR a, res;
	int count;

	if (!box)
		return 0;

	count = 0;
	plane = box->planes;
	ZeroVector(&res);

	for (p = box->numPlanes; p; p--, plane++)
	{
		if (!plane->status) continue;

		dist = DotProductFF(v, &plane->normal) - plane->k;

		if (dist < edge)
		{
			dist -= edge;

			SetVectorFF(&a, &plane->normal);
			ScaleVectorFF(&a, -dist);
			AddToVectorFF(&res, &a);
			
			count++;
		}
	}

	SetVectorFF(out, &res);

	return count;
}


/*	--------------------------------------------------------------------------------
	Function		: IsPointInCameraBox
	Purpose			: checks if a 3D point is within a camera space
	Parameters		: 
	Returns			: 
*/

int IsPointInCameraBox(FVECTOR v, CAM_BOX *box)
{
	int p;
	CAM_PLANE *plane = box->planes;
	fixed dist;

	for (p = box->numPlanes; p; p--, plane++)
	{
		// todo: v.slow! optimise!
		dist = DotProductFF(&v, &plane->normal);

		if (dist < plane->k)
			return 0;
	}
	return 1;
}

/*	--------------------------------------------------------------------------------
	Function		: CheckCameraBoxes
	Purpose			: checks the current camera box, handles transitions between boxes
	Parameters		: 
	Returns			: 
*/

void CheckCameraBoxes(void)
{
	if (!currCamBox || !IsPointInCameraBox(camTarget, currCamBox))
	{
		CAM_BOX *box = cameraBoxes.boxes;
		int b, c;

		//if (currCamBox)
		//	utilPrintf("Frogger leaves cam box (%08x)\n", box);

		currCamBox = NULL;
	
		for (b = cameraBoxes.numBoxes, c = 0; b; b--, c++, box++)
		{
			if (IsPointInCameraBox(camTarget, box))
			{
				currCamBox = box;
				//utilPrintf("Frogger enters cam box #%d (%08x)\n", c, box);
				break;
			}
		}
	}
}



/*	--------------------------------------------------------------------------------
	Function		: CreateAndAddTransCamera
	Purpose			: adds a dynamic camera angle change for a specified game tile
	Parameters		: GAMETILE *,unsigned long,fixed,fixed,fixed
	Returns			: TRANSCAMERA *
	Info			: 
*/
TRANSCAMERA *CreateAndAddTransCamera(GAMETILE *tile,unsigned char dirCamMustFace, SVECTOR *v, unsigned char flags)
{
	TRANSCAMERA *newItem;// = (TRANSCAMERA *)MALLOC0(sizeof(TRANSCAMERA));

	// Try to get a free transcamera block from the array contained in the transcamera list.
	// Happy now James?
	if( tcList.count >= tcList.blocks || !(newItem = &tcList.array[tcList.count++]) )
		return NULL;

	newItem->tile			= tile;
	newItem->dirCamMustFace	= dirCamMustFace;
	SetVectorSS(&newItem->camOffset, v);
	newItem->flags			= flags;
//mm	newItem->FOV			= ToFixed(45.0);
//mm	newItem->speed			= ToFixed(1.0);
	newItem->FOV			= 184320;
	newItem->speed			= 4096;


//	newItem->next			= transCameraList;
//	transCameraList			= newItem;

	return newItem;
}

/*	--------------------------------------------------------------------------------
	Function		: AllocNmeBlock
	Purpose			: Initialise a fixed number of enemies
	Parameters		: 
	Returns			: void
	Info			: 
*/
void AllocCamBlock( int num )
{
	if( !tcList.array )
		tcList.array = (TRANSCAMERA *)MALLOC0(sizeof(TRANSCAMERA)*num);

	tcList.count = 0;
	tcList.blocks = num;
}


/*	--------------------------------------------------------------------------------
	Function		: CheckForDynamicCameraChange
	Purpose			: checks current tile to see if a camera change is needed
	Parameters		: GAMETILE *
	Returns			: void
	Info			: 
*/
void CheckForDynamicCameraChange(GAMETILE *tile, int pl)
{	
	char specialCaseTile = 0;
	int i;
	TRANSCAMERA *cur;
//	static VECTOR stdVector = {0,480,192};
		
	if(camControlMode)
		return;

//	cur = transCameraList;
//	while(cur)
	for( i=tcList.count; i; i-- )
	{
		cur = &tcList.array[i-1];

		if(cur->tile == tile)
		{
			switch (cur->flags)
			{
			case LOOK_AT_FROG:
				SetVectorSS(&camDist,&cur->camOffset);
			
				if (fixedPos)
					SubVectorFFF(&currCamOffset, &currCamSource, &currCamTarget);

				specialCaseTile = 1;
				fixedDir = 0;
				fixedPos = 0;
				fixedUp	 = 0;
				break;

			case LOOK_AT_FROG_NOTILT:
				SetVectorSS(&camDist,&cur->camOffset);
				if (fixedPos)
					SubVectorFFF(&currCamOffset, &currCamSource, &currCamTarget);
				specialCaseTile = 1;
				fixedDir = 0;
				fixedPos = 0;
				fixedUp	 = 1;
				break;

			case STATIC_CAMERA:
				SetVectorFS(&camSource, &cur->camOffset);
				fixedDir = 1;
				fixedPos = 1;
				fixedUp = 0;
				break;

			case FIXED_SOURCE:
				SetVectorFS(&camSource, &cur->camOffset);
				fixedDir = 0;
				fixedPos = 1;
				fixedUp = 0;
			break;

			case LOOK_AT_TILE:
				SetVectorFS(&camSource, &cur->camOffset);
				SetVectorFS(&camTarget, &tile->centre );
				fixedDir = 1;
				fixedPos = 1;
				fixedUp = 0;
				break;

			case LOOK_IN_DIR:								   
				SetVectorFS(&camSource, &cur->camOffset );
															   
				SetVectorFF(&camTarget,&cur->camLookAt);
				AddToVectorFF(&camTarget, &camSource);
				
				fixedDir = 1;
				fixedPos = 1;
				fixedUp = 1;
				break;
			}

			if (cur->dirCamMustFace)
			{
				camFacing[pl] = cur->dirCamMustFace - 1;
				player[pl].extendedHopDir = (frogFacing[pl] - camFacing[pl])&3;
			}

			if(cur->speed!=0)
				transCamSpeedMult = cur->speed;
			else
//mm				transCamSpeedMult = ToFixed(1.0F);
				transCamSpeedMult = 4096;

			yFOVNew = cur->FOV;

			break;
		}
//		cur = cur->next;
	}

//	if(!specialCaseTile)
//		SetVector(&camDist,&stdVector);
}


/*	--------------------------------------------------------------------------------
	Function		: FreeTransCameraList
	Purpose			: frees the camera list
	Parameters		: 
	Returns			: void
	Info			: 
*/
void FreeTransCameraList()
{
//	TRANSCAMERA *cur,*next;

//	cur = transCameraList;
//	while(cur)
//	{
//		next = cur->next;
//		FREE(cur);
//		cur = next;
//	}
//	transCameraList = NULL;

	if( tcList.array )
		FREE( tcList.array );

	tcList.array = NULL;
	tcList.blocks = tcList.count = 0;

	lastTile[0] = NULL;
	lastTile[1] = NULL;
	lastTile[2] = NULL;
	lastTile[3] = NULL;

	fixedDir = 0;
	fixedPos = 0;
	firstPerson = 0;

	startCamFacing = startFrogFacing = 0;
}


/* --------------------------------------------------------------------------------
	Function	: CameraLookAtFrog
	Purpose		: Sets camTarget to the frog, or an average position
	Parameters	: (void)
	Returns		: void 
*/
void CameraLookAtFrog(void)
{
	if(camControlMode)
		return;

	if(frog[0] && !fixedDir && !controlCamera)
	{
		// Average frog position	
		FVECTOR target;

		if( gameState.multi != MULTILOCAL )
		{
			CalcSPCamera( &target );
			SetVectorFF(&camTarget, &target);
		}
 		else
 			CalcMPCamera( &camTarget );

/*		if (!idleCamera)
		{
			VECTOR v;
			SetVector(&v, currTile[0]->dirVector[frogFacing[0]]);
			ScaleVector(&v, camLookOfs);
			AddToVector(&target, v);
		}
*/
	}
	
}

void CalcSPCamera( FVECTOR *target )
{
	FVECTOR v;
	fixed t = player[0].jumpTime;
	
//	if (t > 0 && t < 1)	// jumping; calculate linear position
	if (t > 0 && t < 4096)	// jumping; calculate linear position
	{
		SetVectorFS(target, &player[0].jumpOrigin);

		// horizontal
		SetVectorFF(&v, &player[0].jumpFwdVector);
		ScaleVectorFF(&v, t);
		AddToVectorFF(target, &v);

		// vertical
		//SetVector(&v, &player[i].jumpUpVector);
		//ScaleVector(&v, player[i].heightJumped);
		//AddToVector(&target, &v);
	}										
	else
		SetVectorFS(target, &frog[0]->actor->position);
}


void CameraSetOffset(void)
{
	long nC;
	int i, l=0;

	if( gameState.multi != MULTILOCAL )
	{
		FVECTOR v;
		nC = (camFacing[0]+1)&3;

		SetVectorFF(&v, &camVect);
		ScaleVectorFF(&v, currCamDist.vy);
		SetVectorFF(&camOffset, &v);

		SetVectorFF(&v, &currTile[0]->dirVector[camFacing[0]]);
		ScaleVectorFF(&v, -currCamDist.vz);
		AddToVectorFF(&camOffset, &v);

		SetVectorFF(&v, &currTile[0]->dirVector[nC]);
		ScaleVectorFF(&v, camSideOfs);
		AddToVectorFF(&camOffset, &v);
	}
	else
	{
		fixed afx=0,afy=0,afz=0;
	
		for( i=0; i<NUM_FROGS; i++ )
		{
			nC = (camFacing[i]+1)&3;
			if( !(player[i].frogState & FROGSTATUS_ISDEAD) )
			{
				afx += /*FMul(currTile[i]->normal.vx,currCamDist.vy)*/0 - FMul(currTile[i]->dirVector[camFacing[i]].vx,currCamDist.vz); 
				afy += /*FMul(currTile[i]->normal.vy,*/currCamDist.vy/*)*/ - FMul(currTile[i]->dirVector[camFacing[i]].vy,currCamDist.vz);
				afz += /*FMul(currTile[i]->normal.vz,currCamDist.vy)*/0 - FMul(currTile[i]->dirVector[camFacing[i]].vz,currCamDist.vz);

				afx += FMul(currTile[i]->dirVector[nC].vx,camSideOfs);
				afy += FMul(currTile[i]->dirVector[nC].vy,camSideOfs);
				afz += FMul(currTile[i]->dirVector[nC].vz,camSideOfs);

				l++;
			}
		}

		if( l )
		{
			camOffset.vx = afx;
			camOffset.vy = afy;
			camOffset.vz = afz;

			ScaleVectorFF( &camOffset, 4096/l );
		}
	}
}


/* --------------------------------------------------------------------------------
	Function	: SlurpCamPosition
	Purpose		:
	Parameters	: (void)
	Returns		: void 
*/
void SlurpCamPosition( )
{
	fixed s1,s2,s3;//,s4;
	FVECTOR v;

	if(camControlMode)
		return;

//	if (idleCamera)
//	{
//		s1 = min(4096, (gameSpeed<<12)/(FMul(camSpeed ,transCamSpeedMult)*6));
//		s2 = min(4096, (gameSpeed<<12)/(FMul(camSpeed2,transCamSpeedMult)*6));
//		s3 = min(4096, (gameSpeed<<12)/(FMul(camSpeed3,transCamSpeedMult)*6));
//		s4 = min(4096, (gameSpeed<<12)/(FMul(camSpeed4,transCamSpeedMult)*6));
//	}
//	else
//	{
		s1 = min(4096, (gameSpeed<<12)/FMul(camSpeed ,transCamSpeedMult) );
		s2 = min(4096, (gameSpeed<<12)/FMul(camSpeed2,transCamSpeedMult) );
		s3 = min(4096, (gameSpeed<<12)/FMul(camSpeed3,transCamSpeedMult) );
		//s4 = min(4096, (gameSpeed<<12)/FMul(camSpeed4,transCamSpeedMult) );
//	}

	SubVectorFFF(&v, &camTarget, &currCamTarget);
	ScaleVectorFF(&v, s3);
	AddToVectorFF(&currCamTarget, &v);

	if( gameState.multi != MULTILOCAL  && !fixedDir)
	{
  		if(GetCamLimitVector(&v, &currCamTarget, currCamBox, cam_edge_spacing))
		{
	  		ScaleVectorFF(&v, s3);
  			AddToVectorFF(&currCamTarget, &v);
		}
	}

	if (!fixedPos)
	{
		// use offset from target..
		CameraSetOffset();

		SubVectorFFF(&v, &camOffset, &currCamOffset);
		ScaleVectorFF(&v, s3);
		AddToVectorFF(&currCamOffset, &v);

		AddVectorFFF(&currCamSource, &currCamTarget, &currCamOffset);
	}
	else
	{
		// .. otherwise interpolate source independantly
		SubVectorFFF(&v, &camSource, &currCamSource);
		ScaleVectorFF(&v, s3);
		AddToVectorFF(&currCamSource, &v);
	}

	// camDist

	currCamDist.vx = scaleV * camDist.vx;
	currCamDist.vy = scaleV * camDist.vy;
	currCamDist.vz = scaleV * camDist.vz;
	
	// camVect (up vector)

	if ( gameState.multi != SINGLEPLAYER )
	{
		FVECTOR t = { 0,0,0 };
		int i;
	
		// In multiplayer, average out camera 'up' vectors

		for (i=0; i<NUM_FROGS; i++)
		{
			t.vx+=currTile[i]->normal.vx;
			t.vy+=currTile[i]->normal.vy;
			t.vz+=currTile[i]->normal.vz;
		}

		MakeUnit (&t);
		
		if (!fixedUp)
		{
			camVect.vx -= FMul(s2 , (camVect.vx-t.vx) );
			camVect.vy -= FMul(s2 , (camVect.vy-t.vy) );
			camVect.vz -= FMul(s2 , (camVect.vz-t.vz) );
		}
	}
	else
	{
		if (!fixedUp)
		{
			SubVectorFFF(&v, &currTile[0]->normal, &camVect);
			ScaleVectorFF(&v, s2);
			AddToVectorFF(&camVect, &v);
			
			MakeUnit(&camVect);	// fake a vaguely spherical interpolation
		}
	}

	camLookOfs -= FMul(s1 , (camLookOfs-camLookOfsNew) );
}


/* --------------------------------------------------------------------------------
	Programmer	: Matthew Cloy
	Function	: UpdateCameraPosition

	Purpose		:
	Parameters	: (void)
	Returns		: void 
*/
void UpdateCameraPosition( )
{
	int i;

	if(camControlMode)
		return;

	if(!frog[playerFocus] || !currTile[playerFocus] || controlCamera)
		return;

	for( i=0; i<NUM_FROGS; i++ )
	{
		if (currTile[i] != lastTile[i])	//This causes lots of problems with camera transitions..
		{
			if (currTile[i] && lastTile[i])
			{
				if (camFacing[i] == prevCamFacing[i])
					camFacing[i] = GetTilesMatchingDirection(lastTile[i],camFacing[i],currTile[i]);
			}
			prevCamFacing[i] = camFacing[i];
			lastTile[i] = currTile[i];
			CheckForDynamicCameraChange(currTile[i], i);
		}
	}

	CameraLookAtFrog();

 	CheckCameraBoxes();

	//if ( gameState.mode != CAMEO_MODE && !fixedPos )
	//	CameraSetSource();

	/*if (fixedUp)
	{
		camVect.vx = 0;
		camVect.vx = ToFixed(0);
		camVect.vy = ToFixed(1);
	}*/

	SlurpCamPosition();

	// Handle shaky camera nonsense
	
	if (cam_shakiness > 0)
	{
		currCamTarget.vx += Random(cam_shakiness) - (cam_shakiness>>1);
		currCamSource.vx += Random(cam_shakiness) - (cam_shakiness>>1);
		currCamTarget.vy += Random(cam_shakiness) - (cam_shakiness>>1);
		currCamSource.vy += Random(cam_shakiness) - (cam_shakiness>>1);
		currCamTarget.vz += Random(cam_shakiness) - (cam_shakiness>>1);
		currCamSource.vz += Random(cam_shakiness) - (cam_shakiness>>1);

		cam_shakiness -= FMul(cam_shake_falloff, gameSpeed);
	}

	// edge spacing should probably related to tan(FOV/2)
	cam_edge_spacing = FMul( DistanceBetweenPointsFF(&currCamSource,&currCamTarget) , 3072);

//	if( swingCam )
//	{
//		// if the player is on their last life - give the swaying camera more 'urgency' - subtle
//		// bunch of arse, more like
//		if(player[0].lives < 3)
//		{
//			swayModifier	= 3<<12;
//			sideSwayAmt		= 25<<12;
//		}
//		else
//		{
//			swayModifier	= 1<<12;
//			sideSwayAmt		= 50<<12;
//		}

//		camSideOfs = ((sinf(actFrameCount*sideSwaySpeed*swayModifier)*sideSwayAmt) * camDist.vz) / 350.0;
//		camSideOfs = FDiv( FMul(rsin(actFrameCount*FMul(sideSwaySpeed,swayModifier)),sideSwayAmt)*camDist.vz, 350<<12);
//		camSideOfs = (FMul(rsin(actFrameCount*FMul(sideSwaySpeed,swayModifier)),sideSwayAmt)*camDist.vz) / 350;
//	}
}

void ResetCamera( )
{
	FVECTOR initialCamOffset = { 0,100<<12,100<<12 };
	SVECTOR initialCamDist = { 0,680*SCALE,192*SCALE };

//bb - this change seemed to fix the camera start problem,
//     for 1 or 2 runs, but now it doesn't seem to do anything.
//	FVECTOR initialCamSource	= { 0,569<<12,-200<<12 };
//	FVECTOR initialCurrCamSource = { 0,100<<12,100<<12 };
	FVECTOR initialCamSource;
	FVECTOR initialCurrCamSource;
	if(frog[0]->actor)
	{
		initialCamSource.vx	=		 frog[0]->actor->position.vx <<12;
		initialCamSource.vy	=		(frog[0]->actor->position.vy + 569) <<12;
		initialCamSource.vz	=		(frog[0]->actor->position.vz - 200) <<12;
		
		initialCurrCamSource.vx =	 frog[0]->actor->position.vx <<12;
		initialCurrCamSource.vy =	(frog[0]->actor->position.vy + 100) <<12;
		initialCurrCamSource.vz =	(frog[0]->actor->position.vz + 100) <<12;
	}
	else
	{
		FVECTOR initialCamSource	= { 0,569<<12,-200<<12 };
		FVECTOR initialCurrCamSource = { 0,100<<12,100<<12 };
	}

	SetVectorFF( &camSource, &initialCamSource );
	SetVectorFF( &currCamSource, &initialCurrCamSource );
	SetVectorFF( &camOffset, &initialCamOffset );
	SetVectorFF( &currCamOffset, &initialCamOffset );
	SetVectorSS( &camDist, &initialCamDist );
	ZeroVector( &currCamTarget );
	SetVectorFF( &cameraUpVect, &upVec );
	scaleV = 4506;
}

/*	--------------------------------------------------------------------------------
	Function		: InitCamera
	Purpose			: 
	Parameters		: 
	Returns			: 
*/

void InitCamera(void)
{
	int i;
	currCamBox = NULL;
//bbupdate
//	CheckCameraBoxes();

	if( gameState.multi != MULTILOCAL )
	{
		lastTile[0] = NULL;
		CheckForDynamicCameraChange(currTile[0],0);
	}
	else
	{
		for( i=0; i<NUM_FROGS; i++ )
		{
			camFacing[i] = 0;
			lastTile[i] = NULL;
			CheckForDynamicCameraChange(currTile[i],i);
		}
	}

	cam_shakiness = 0;
	controlCamera = 0;

	SubVectorFFF(&currCamOffset, &camTarget, &camSource);
	UpdateCameraPosition();

	SetVectorFF(&currCamSource, &camSource);
	SetVectorFF(&currCamTarget, &camTarget);
//	SetVectorFF(&currCamOffset, &camOffset);
	SetVectorFF(&camVect, &currTile[0]->normal);
	SlurpCamPosition();
	SetVectorFF(&currCamOffset, &camOffset);
	for(i = 0;i < 50;i++)
		UpdateCameraPosition();

	SetCamFF(currCamSource,currCamTarget);
}




fixed FindMaxInterFrogDistance( )
{
	int i, j;
	fixed dist, best=ToFixed(-100000);
	SVECTOR sep;

	for( i=0; i<NUM_FROGS; i++ )
	{
		for( j=i+1; j<NUM_FROGS; j++ )
		{
			SubVectorSSS( &sep, &frog[i]->actor->position, &frog[j]->actor->position );
			dist = MagnitudeS( &sep );
//			dist = (abs(frog[i]->actor->position.vz - frog[j]->actor->position.vz))<<12;

			if( dist > best )
			{
				best = dist;
			}
		}
	}

	return best;
	
//	return ToFixed(-1);
}

/*
int camrotorder = 0;
int camrotval1 = -1;
int camrotval2 = 1;
int camrotval3 = 1;
int camrotval4 = 1;
int camrotval5 = 1;
int camrotval6 = 1;
int camrotval7 = 1;
int camrotval8 = 1;
*/
void SetCamFF(FVECTOR src, FVECTOR tar)
{
#ifdef PSX_VERSION
#ifdef DREAMCAST_VERSION
	FVECTOR view,rotUp;
	fixed angle,camX,camY,len;

	static short tempAng = 0;

	SubVectorFFF(&view,&src,&tar);
	camY = ((atan2(-view.vx,view.vz) / 6.283185308) * 4096.0);
	camY &=  0xfff;
	len = Magnitude2DF(&view);
	camX = ((atan2(view.vy,len) / 6.283185308) * 4096.0);
	camX &=  0xfff;

	RotateVector2D(&rotUp,&camVect,camY);
	RotateVector2DX(&rotUp,&rotUp,camX);

	angle = ((atan2(rotUp.vx,rotUp.vy) / 6.283185308) * 4096.0);
	angle &= 0xfff;
	angle *= 360;

	camera.vpx = -src.vx>>12;
	camera.vpy = -src.vy>>12;
	camera.vpz = src.vz>>12;

	camera.vrx = -tar.vx>>12;
	camera.vry = -tar.vy>>12;
	camera.vrz = tar.vz>>12;

	camera.rz  = angle;
	
	GsSetRefView2(&camera);


	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
#else
	FVECTOR view,rotUp;
	fixed angle,camX,camY,len;

	static short tempAng = 0;

	SubVectorFFF(&view,&src,&tar);
	camY = utilCalcAngle(-view.vx,view.vz) & 0xfff;
	len = Magnitude2DF(&view);
	camX = utilCalcAngle(view.vy,len) & 0xfff;

	RotateVector2D(&rotUp,&camVect,camY);
	RotateVector2DX(&rotUp,&rotUp,camX);
	
	angle = (utilCalcAngle(rotUp.vx,rotUp.vy) & 0xfff)*360;

	camera.vpx = -src.vx>>12;
	camera.vpy = -src.vy>>12;
	camera.vpz = src.vz>>12;

	camera.vrx = -tar.vx>>12;
	camera.vry = -tar.vy>>12;
	camera.vrz = tar.vz>>12;

	camera.rz  = angle;

//	utilPrintf( "%i\n", angle );
	
	GsSetRefView2(&camera);


	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
#endif
#endif
}

#ifndef FINAL_MASTER
fixed controlcamspeed = 4096*70;
fixed camturnspeed = 100;
void ControlCamera()
{
	FVECTOR camDir,camRight,camUp;
	fixed camSpeed = FMul(controlcamspeed,gameSpeed);
	fixed camTurnSpeed;
	fixed dist;

	SubVectorFFF(&camDir,&currCamTarget,&currCamSource);
	dist = MagnitudeF(&camDir);
	MakeUnit(&camDir);
	CrossProductFFF(&camRight,&camDir,&camVect);
	MakeUnit(&camRight);
	CrossProductFFF(&camUp,&camRight,&camDir);
	MakeUnit(&camUp);


	camTurnSpeed = FMul(camturnspeed,dist);
	camTurnSpeed = FMul(camTurnSpeed,gameSpeed);
//	camTurnSpeed = FMul(camTurnSpeed,camturnspeed);

	if(padData.digital[0] & PAD_LEFT)
	{
		AddToOneScaledVector(&currCamSource,&camRight,-camSpeed);
		AddToOneScaledVector(&currCamTarget,&camRight,-camSpeed);
	}
	if(padData.digital[0] & PAD_RIGHT)
	{
		AddToOneScaledVector(&currCamSource,&camRight,camSpeed);
		AddToOneScaledVector(&currCamTarget,&camRight,camSpeed);
	}
	if(padData.digital[0] & PAD_UP)
	{
		AddToOneScaledVector(&currCamSource,&camDir,camSpeed);
		AddToOneScaledVector(&currCamTarget,&camDir,camSpeed);
	}
	if(padData.digital[0] & PAD_DOWN)
	{
		AddToOneScaledVector(&currCamSource,&camDir,-camSpeed);
		AddToOneScaledVector(&currCamTarget,&camDir,-camSpeed);
	}

#ifdef PSX_VERSION
	if(padData.digital[0] & PAD_TRIANGLE)
	{
		AddToOneScaledVector(&currCamTarget,&camUp,camTurnSpeed);
	}
	if(padData.digital[0] & PAD_CROSS)
	{
		AddToOneScaledVector(&currCamTarget,&camUp,-camTurnSpeed);
	}
	if(padData.digital[0] & PAD_SQUARE)
	{
		AddToOneScaledVector(&currCamTarget,&camRight,-camTurnSpeed);
	}
	if(padData.digital[0] & PAD_CIRCLE)
	{
		AddToOneScaledVector(&currCamTarget,&camRight,camTurnSpeed);
	}
	if(padData.digital[0] & PAD_L2)
	{
		AddToOneScaledVector(&currCamSource,&camUp,-camSpeed);
		AddToOneScaledVector(&currCamTarget,&camUp,-camSpeed);
	}
	if(padData.digital[0] & PAD_L1)
	{
		AddToOneScaledVector(&currCamSource,&camUp,camSpeed);
		AddToOneScaledVector(&currCamTarget,&camUp,camSpeed);
	}
#endif

#ifdef PC_VERSION
	if(padData.digital[1] & PAD_LEFT)
	{
		AddToOneScaledVector(&currCamTarget,&camRight,-camTurnSpeed);
	}
	if(padData.digital[1] & PAD_RIGHT)
	{
		AddToOneScaledVector(&currCamTarget,&camRight,camTurnSpeed);
	}
	if(padData.digital[1] & PAD_UP)
	{
		AddToOneScaledVector(&currCamTarget,&camUp,camTurnSpeed);
	}
	if(padData.digital[1] & PAD_DOWN)
	{
		AddToOneScaledVector(&currCamTarget,&camUp,-camTurnSpeed);
	}
	if(padData.digital[3] & PAD_DOWN)
	{
		AddToOneScaledVector(&currCamSource,&camUp,-camSpeed);
		AddToOneScaledVector(&currCamTarget,&camUp,-camSpeed);
	}
	if(padData.digital[3] & PAD_UP)
	{
		AddToOneScaledVector(&currCamSource,&camUp,camSpeed);
		AddToOneScaledVector(&currCamTarget,&camUp,camSpeed);
	}
#endif
}
#endif
