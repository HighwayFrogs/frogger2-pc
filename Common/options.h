/*

	This file is part of Frogger2, (c) 1997 Interactive Studios Ltd.


	File		: options.h
	Programmer	: Matt Cloy
	Date		: 27/05/99

----------------------------------------------------------------------------------------------- */

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	EXTRA_DIFFICULTY,
	EXTRA_VIEW_FMV,
	EXTRA_CREDITS,
	EXTRA_RESET_BEST_TIMES,
	EXTRA_RESET_STORY_MODE,
	EXTRA_MINI,
	EXTRA_VIEW_ART,
	EXTRA_FEEDING_FRENZY,

	NUM_EXTRAS,
};

#define NUM_BASIC_EXTRAS 5

enum
{
	OP_SOUND,				// Sound menu
	OP_EXTRA,				// Cheats/Extras
	OP_CONTROL,				// Controller Setup
	OP_GLOBALMENU,			// Main menu (Choose one of above 3 options)
	OP_MULTIPLAYERNUMBER,	// Number of players selection
	OP_LEVELSEL,			// Level select
	OP_CHARSEL,				// character select
	OP_BOOK,				// the book
	OP_ARCADE,				// arcade mode
	OP_SP_CHARSEL,			// single player char select
	OP_FMV,					// fmv menu
	OP_TOTAL,
};

#define DEFAULT_VOLUME 8
#define NUM_OPTIONS 2
#define SHADOW_DROP 50


extern long pageToStoryLevel[BOOK_NUM_PAGES];
extern long picOffset;
extern int staticFlash;


typedef void (*CONTROLFUNC)(void);

enum
{
	C_UP,
	C_DOWN,
	C_LEFT,
	C_RIGHT,
	C_SELECT,
	C_BACK,
	C_TOTAL,
};

#define MAX_FMV 12

typedef struct
{
	long init;				// Need to initialise
	long mode;				// Current options mode
	long selection;			// Current selection

	TEXTOVERLAY *title;		// Title text
	TEXTOVERLAY *subTitle;	// Title text
	char titleStr[64];		// and string
	char subTitleStr[64];	// and string

	long soundVol;			// Global volume levels (1..10)
	long musicVol;
#ifdef DREAMCAST_VERSION
	char stereo;			// Stereo or mono sound#
#endif

	SPRITEOVERLAY *soundIcons[2];
	ACTOR2 *page;
	ACTOR *book;
	long pageDir;
	long pageDrawCount;
	long pageNum;
	long maxPageAllowed;

	long levelNum;
	
	TEXTOVERLAY *extras[NUM_EXTRAS];	// Extras menu text
	long extrasState[NUM_EXTRAS];
	long extrasToggle[NUM_EXTRAS];
	long extrasAvailable[NUM_EXTRAS];
	
	TEXTOVERLAY *numPText;	// Number of players
	TEXTOVERLAY *numPText2;	// Number of players
	char numPStr[32];		// number of players string
	char numPStr2[32];
	long numPlayers;

	long soundSelection;
	long extraSelection;
	long multiSelection;
	long levelSelection;
	long currentPlayer;

	SPRITEOVERLAY *multiFace[8];

	CONTROLFUNC controls[C_TOTAL];
	TEXTOVERLAY *playerNum[4];

	SPRITEOVERLAY *worldBak,*titleBak,*statusBak;
	TEXTOVERLAY *levelText[MAX_LEVELSTRING];
	TEXTOVERLAY *levelParText[MAX_LEVELSTRING];
	TEXTOVERLAY *levelSetByText[MAX_LEVELSTRING];
	TEXTOVERLAY *levelCoinText[MAX_LEVELSTRING];
	long spChar;
	TEXTOVERLAY *worldText,*arcadeText,*selectText,*parText[3];
	long fmvNum;
	TEXTOVERLAY *fmvText[MAX_FMV];
	SPRITEOVERLAY *soundBar[2];
	TEXTOVERLAY *chapterText[2];
	TEXTOVERLAY *chapterNameText[2][2];
	SPRITEOVERLAY *chapterPic[2];
	TEXTOVERLAY *mpText;
	char mpStr[64];
	TEXTOVERLAY *yesNoText[2];
	ACTOR2 *chestLid;
	ACTOR2 *door;
	SPRITEOVERLAY *levelCoinMedal[MAX_LEVELSTRING];
	SPRITEOVERLAY *soundBak[2];
	TEXTOVERLAY *sfxText[2];
	TEXTOVERLAY *soundInstrText[2];
#ifdef DREAMCAST_VERSION
	TEXTOVERLAY *stereoSelectText[2];		// Stereo or mono
#endif
	TEXTOVERLAY *warnText[4];
	SPRITEOVERLAY *beatenIcon[MAX_LEVELSTRING];
	char playerChar[4];
} OPTIONSOBJECTS;

typedef struct
{
	unsigned char spacing,r,g,b;
}CREDIT_DATA;

extern OPTIONSOBJECTS options;

void MenuRight(void);
void MenuLeft(void);
void MenuBack(void);
void MenuSelect(void);
void NumPRight(void);
void NumPLeft(void);
void NumPSelect(void);
void OptionBack(void);
void ExtraUp(void);
void ExtraDown(void);
void ExtraSelect(void);
void MPLevLeft(void);
void MPLevRight(void);
void MPLevSelect(void);
void MPLevBack(void);
void MPCharLeft(void);
void MPCharRight(void);
void MPCharSelect(void);
void MPCharBack(void);
void SoundUp(void);
void SoundDown(void);
void SoundLeft(void);
void SoundRight(void);
void BookLeft(void);
void BookRight(void);
void BookSelect(void);
void BookDown(void);
void ArcadeUp(void);
void ArcadeDown(void);
void ArcadeLeft(void);
void ArcadeRight(void);
void ArcadeSelect(void);
void ArcadeBack(void);
void SPCharSelect(void);
void SPCharUp(void);
void SPCharDown(void);
void SPCharBack(void);
void FMVUp(void);
void FMVDown(void);
void FMVSelect(void);
void FMVBack(void);

void InitOptionsMenu(void);
void DoneOptionsMenu(void);
void RunOptionsMenu(void);

void GoMulti(void);
void UpdateSoundOptions(void);

int CheckCamStill();

#ifdef PC_VERSION
void SetActorGouraudValues(ACTOR2 *actor, int r,int g,int b);
#endif

#ifdef PSX_VERSION
void SetActorGouraudValues(FMA_MESH_HEADER *mesh, int r,int g,int b);
void SetActorGouraudValuesMinus(FMA_MESH_HEADER *mesh, int r,int g,int b);
void SetActorGouraudValuesPlus(FMA_MESH_HEADER *mesh, int r,int g,int b, FMA_GT4 *gouraudList);
#endif

void DoArcadeMenu();
void StartCredits();
void EndCredits();
void DoCredits();
void BookStart();
void ArcadeStart();
void MPStart();
void SetMusicVolume();
void RunArtViewer();
void RunTeaserScreens( );

#ifdef DREAMCAST_VERSION
void ToggleStereo( );
#endif

extern TIMER artTimer;
extern u16 optionsLastButton;
extern char chapterStr[2][32];


#ifdef __cplusplus
}
#endif

#endif
