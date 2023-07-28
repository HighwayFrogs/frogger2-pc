/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: Cam.h
	Programmer	: Andy Eder
	Date		: 15/04/99 15:01:33

----------------------------------------------------------------------------------------------- */

#ifndef CAM_H_INCLUDED
#define CAM_H_INCLUDED


#include "define.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	STATIC_CAMERA,
	FIXED_SOURCE,
	LOOK_AT_FROG,
	LOOK_AT_FROG_NOTILT,
	LOOK_AT_TILE,
	LOOK_IN_DIR,
};


//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

typedef struct TAGTRANSCAMERA
{
	GAMETILE		*tile;
	unsigned char		dirCamMustFace;
	unsigned char		flags;
	SVECTOR			camOffset; 
	FVECTOR			camLookAt;
	fixed FOV;
	fixed speed;

//	struct TAGTRANSCAMERA	*next;

} TRANSCAMERA;

typedef struct
{
	TRANSCAMERA *array;
	short count, blocks;

} TRANSCAMERALIST;

// ------------- Camera boxes ----------------

typedef struct
{
	FVECTOR	normal;
	fixed	k;
	unsigned long status;
} CAM_PLANE;

typedef struct
{
	int numPlanes;
	CAM_PLANE *planes;
} CAM_BOX;

typedef struct
{
	int numBoxes;
	int numPlanes;
	CAM_BOX *boxes;
	CAM_PLANE *planes;
} CAM_BOX_LIST;

//----- [ GLOBALS ] ----------------------------------------------------------------------------//

extern FVECTOR	cameraUpVect;
extern FVECTOR	camVect;

extern FVECTOR	currCamSource;
extern FVECTOR	currCamTarget;
extern FVECTOR	currCamDist;
extern FVECTOR	camSource;
extern FVECTOR	camTarget;

extern SVECTOR	camDist;

extern fixed	camSpeed;
extern fixed	camSpeed3;
extern fixed	camLookOfs;
extern int		camFacing[];
extern long		prevCamFacing[];
extern short	startCamFacing, startFrogFacing;


/*
//extern VECTOR	actualCamSource[2];
//extern VECTOR	actualCamTarget[2];

*/
extern fixed	xFOV;				// actually the SCREEN RATIO, i.e. 4/3
extern fixed	yFOV;				// THIS is the field of view
extern fixed	precScaleFactor;

extern fixed	scaleV;
extern fixed camSideOfs;

extern int camControlMode;
extern char		controlCamera;
extern char		fixedDir;
extern char		fixedPos;
extern char		fixedUp;
extern char		firstPerson;

extern fixed cam_shakiness;
extern fixed cam_shake_falloff;

extern char playerFocus;
extern FVECTOR currCamOffset, camOffset;
extern GAMETILE *lastTile[];

extern CAM_BOX_LIST cameraBoxes;

//----- [ FUNCTION PROTOTYPES ] ----------------------------------------------------------------//

TRANSCAMERA *CreateAndAddTransCamera(GAMETILE *tile,unsigned char dirCamMustFace, SVECTOR *v, unsigned char flags);

void AllocCamBlock( int num );
void CheckForDynamicCameraChange(GAMETILE *tile, int pl);
void FreeTransCameraList();

void CameraLookAtFrog();
void UpdateCameraPosition( );
void InitCamera(void);
void ControlCamera();
void ResetCamera( );
fixed FindMaxInterFrogDistance( );
void SlurpCamPosition();
void CameraSetOffset(void);


#ifdef __cplusplus
}
#endif

#endif