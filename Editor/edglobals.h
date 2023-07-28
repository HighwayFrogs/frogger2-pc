#ifndef _EDGLOBALS_H_INCLUDED
#define _EDGLOBALS_H_INCLUDED

#include "edittypes.h"

/*	-------------------------------------------------------------------------------------------
	Globals
*/

typedef void (*EDITVARFUNC)(const char *) ;
typedef void (*CREATEFUNCINT)(CREATEENTITY*, const int);
typedef void (*CREATEFUNCSTR)(CREATEENTITY*, const char *);
typedef void (*CREATEFUNCFLOAT)(CREATEENTITY*, const float);
typedef void (*SELECTFUNC)(int, int, int, int);

typedef struct TAGEDITTEXT {
	char string[128];
	const char *var;
	EDITVAR type;
	void *variable;
	BYTE charLimit;
	BOOL active;
} EDITTEXT;

typedef enum {
	EDIT_PLACE_GARIB,
	EDIT_PLACE_ENEMY,
	EDIT_PLACE_PLATFORM,
	EDIT_PLACE_PLACEHOLDER,
	EDIT_MAKE_PATH,
	EDIT_CLEAR_LISTS,
	EDIT_RECT_SELECT,
	EDIT_PLACE_FROG,
	EDIT_EFFECTS,
	EDIT_NONE
} EDITMODE;

#define NUMTILESTATES 12

#ifdef __cplusplus
extern "C" {
#endif

extern const char* TILESTATESTRINGS[NUMTILESTATES];
extern const char* GARIBTYPESTRINGS[];
extern const char* HELPSTRINGS[];
extern const char* CAMTYPESTRINGS[];

extern EDITPATH *editPath;
extern EDITPATH *pathList;
extern EDITGROUP *createList;
extern BOOL		drawLinks;
extern BOOL		drawNormals;
extern BOOL		drawVectors;
extern BOOL		drawDots;
extern BOOL		invertMouse;
extern BOOL		cameraMode;
extern float	HorizontalWhizziness;
extern float	InOutWhizziness;
extern BOOL		clicking;
extern BOOL		showX;
extern EDVECTOR	clickPos;
extern POINT	oldMousePos;
extern short	selectedIcon;
extern short	garibType;
extern int		createFlags;
extern EDITMODE editMode;
extern GAMETILE *editTile;
extern EDITPATHNODE *editFlag;
extern EDVECTOR	editCameraRot;
extern TOOLBAR_SUBMENU submenu;
extern char	savePath[80], scriptPath[80];
extern SELECTFUNC selectFunc;
extern EDITGROUP *selectionList;
extern EDITTEXT editText;
extern char createTypes[][MAXTYPELENGTH];
extern CREATEENTITY *copyEntity;
extern short copyFlag;
extern short helpScreen;
extern short addTiles;

#define EDITOR_SAVE_BASE	"c:\\work\\froggerii\\leveldump\\"
#define ENTITY_DUMP_FILE	"entity-%d-%d.dat"
#define EDITOR_STATE_FILE	"c:\\frogedit.dat"

#ifdef __cplusplus
}
#endif
#endif