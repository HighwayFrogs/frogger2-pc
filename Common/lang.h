/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: lang.h
	Programmer	: David Swift
	Date		: 25 Apr 00

----------------------------------------------------------------------------------------------- */

#ifndef LANG_H_INCLUDED
#define LANG_H_INCLUDED

#include "isllocal.h"

enum {
	LANG_UK,		// (United Kingdom)
	LANG_F,			// (France)
	LANG_D,			// (Germany)
	LANG_IT,		// (Italy)
	LANG_US,		// (United States)

	LANG_NUMLANGS
};

#if PALMODE==1
	#define LANG_DEFAULT LANG_UK
#else
	#define LANG_DEFAULT LANG_US
#endif

enum GAMESTRINGS
{
	STR_LANGUAGE,
	STR_START,
	STR_UP,
	STR_LEFT,
	STR_DOWN,
	STR_RIGHT,
	STR_SUPERHOP,
	STR_CROAK,
	STR_TONGUE,
	STR_WORLD_GARDEN,
	STR_WORLD_ANCIENT,
	STR_WORLD_SPACE,
	STR_WORLD_CITY,
	STR_WORLD_SUB,
	STR_WORLD_LAB,
	STR_WORLD_HALL,
	STR_WORLD_SWAMPY,
	STR_WORLD_RETRO,
	STR_LEVEL_UNNAMED,
	STR_LEVEL_GARDEN1,
	STR_LEVEL_GARDENMULTI,
	STR_LEVEL_ANCIENT1,
	STR_LEVEL_ANCIENT2,
	STR_LEVEL_ANCIENT3,
	STR_LEVEL_ANCIENTMULTI,
	STR_LEVEL_SPACE1,
	STR_LEVEL_SPACE2,
	STR_LEVEL_SPACE3,
	STR_LEVEL_SPACEMULTI,
	STR_LEVEL_CITY1,
	STR_LEVEL_CITY2,
	STR_LEVEL_CITY3,
	STR_LEVEL_CITYMULTI,
	STR_LEVEL_SUB1,
	STR_LEVEL_SUB2,
	STR_LEVEL_SUB3,
	STR_LEVEL_SUBMULTI,
	STR_LEVEL_LAB1,
	STR_LEVEL_LAB2,
	STR_LEVEL_LAB3,
	STR_LEVEL_LABMULTI1,
	STR_LEVEL_LABMULTI2,
	STR_LEVEL_LABMULTI3,
	STR_LEVEL_HALL1,
	STR_LEVEL_HALL2,
	STR_LEVEL_HALLMULTI,
	STR_LEVEL_SWAMPY1,
	STR_LEVEL_SWAMPY2,
	STR_LEVEL_SWAMPY3,
	STR_LEVEL_SWAMPY4,
	STR_LEVEL_RETRO1,
	STR_LEVEL_RETRO2,
	STR_LEVEL_RETRO3,
	STR_LEVEL_RETRO4,
	STR_LEVEL_RETRO5,
	STR_LEVEL_RETRO6,
	STR_LEVEL_RETRO7,
	STR_LEVEL_RETRO8,
	STR_LEVEL_RETRO9,
	STR_LEVEL_RETRO10,
	STR_LEVEL_RETROMULTI1,
	STR_LEVEL_RETROMULTI2,
	STR_LEVEL_RETROMULTI3,
	STR_FRONTEND_STATUS,
	STR_OPTIONS_MUSICVOLUME,
	STR_OPTIONS_SFXVOLUME,
	STR_OPTIONS_PLAYTRACK,
	STR_OPTIONS_PLAYSFX,
	STR_OPTIONS_DESIGNVIEW,
	STR_GAMEOVER,
	STR_RESTARTLEVEL,
	STR_QUIT,
	STR_QUIT_TO_WINDOWS,
	STR_RATING_BAD,
	STR_RATING_OKAY,
	STR_RATING_GOOD,
	STR_RATING_FAB,
	STR_PRESSSTART,
	STR_PRESSENTER,
	STR_ARCADEMODE,
	STR_STORYMODE,
	STR_MULTI_PLAYERWINS,
	STR_MULTI_WINMATCH,
	STR_ENTERNAME,
	STR_MULTI_RACEDESC,
	STR_MULTI_SNAKEDESC,
	STR_MULTI_COLLECTDESC,
	STR_PLAYER,
	STR_PLAYERS,
	STR_LEVELCOMPLETE,
	STR_YOUTOOKTIMEMIN,
	STR_YOUTOOKTIMESEC,
	STR_TRAINING1,
	STR_TRAINING2,
	STR_TRAINING3,
	STR_TRAINING4,
	STR_TRAINING5,
	STR_TRAINING6,
	STR_TRAINING7,
	STR_TRAINING8,
	STR_CONTINUE,
	STR_PRESS_X_TO_CONTINUE,
	STR_LEVEL_TRAINING,
	STR_OPTIONS_1,
	STR_OPTIONS_2,
	STR_OPTIONS_3,
	STR_EXTRAS_1,
	STR_EXTRAS_2,
	STR_EXTRAS_3,
	STR_EXTRAS_4,
	STR_EXTRAS_5,
	STR_EXTRAS_6,
	STR_EXTRAS_7,
	STR_EXTRAS_8,
	STR_MULTIPLAYER,
	STR_SELECT_NUM_PLAYERS,
	STR_SELECT_LEVEL,
	STR_SELECT_CHAR,
	STR_X_SELECT_CHAR,
	STR_CHAPTER_1a,
	STR_CHAPTER_1b,
	STR_CHAPTER_2a,
	STR_CHAPTER_2b,
	STR_CHAPTER_3a,
	STR_CHAPTER_3b,
	STR_CHAPTER_4a,
	STR_CHAPTER_4b,
	STR_CHAPTER_5a,
	STR_CHAPTER_5b,
	STR_CHAPTER_6a,
	STR_CHAPTER_6b,
	STR_CHAPTER_7a,
	STR_CHAPTER_7b,
	STR_CHAPTER_8a,
	STR_CHAPTER_8b,
	STR_CHAPTER_9a,
	STR_CHAPTER_9b,
	STR_CHAPTER_10a,
	STR_CHAPTER_10b,
	STR_PAR,
	STR_SET_BY,
	STR_COINS,
	STR_CHAPTER,
	STR_GOT_ALL_COINS,
	STR_MISSED_COINS,
	STR_NO_BONUS,
	STR_COLLECTBABIES,
	STR_GO,
	STR_ROUND_NUM,
	STR_MULTI_DRAW,
	STR_OUTOFTIME,
	STR_ENTER_NAME,
	STR_NEW_BEST_TIME,
	STR_FMV_1,
	STR_FMV_2,
	STR_OPENED_EXTRA_1,
	STR_OPENED_EXTRA_10,
	STR_OPENED_EXTRA_11,
	STR_OPENED_EXTRA_12,
	STR_OPENED_EXTRA_13,
	STR_OPENED_EXTRA_14,
	STR_OPENED_EXTRA_15,
	STR_OPENED_EXTRA_16,
	STR_EXTRA_LIFE,
	STR_QUICK_HOP,
	STR_SLOW_HOP,
	STR_AUTO_HOP,
	STR_INVULNERABILITY,
	STR_ON,
	STR_OFF,
	STR_CHEAT_1,
	STR_CHEAT_2,
	STR_CHEAT_3,
	STR_CHEAT_4,
	STR_CHEAT_5,
	STR_CHEAT_6,
	STR_CHEAT_7,
	STR_TRIANGLE_BACK,
	STR_RETURN,
	STR_RIGHT_CONTROL,
	STR_RIGHT_SHIFT,
	STR_ESCAPE,
	STR_ARE_YOU_SURE,
	STR_YES,
	STR_NO,
	STR_MCARD_CANCELSAVE,
	STR_MCARD_SELECTSAVE,
	STR_MCARD_SAVEYN,
	STR_MCARD_SAVEGAME,
	STR_MCARD_NOSAVE,
	STR_MCARD_FULL,
	STR_MCARD_RETRYSAVE,
	STR_MCARD_NOCARD,
	STR_MCARD_CHECK,
	STR_MCARD_OVERWRITE,
	STR_MCARD_PREVIOUS,
	STR_MCARD_SAVING,
	STR_MCARD_AUTOSAVING,
	STR_MCARD_COMPLETE,
	STR_MCARD_CONTINUE,
	STR_MCARD_SAVEERROR,
	STR_MCARD_UNFORMAT,
	STR_MCARD_FORMATCARD,
	STR_MCARD_FORMATTING,
	STR_MCARD_FORMATERROR,
	STR_MCARD_PROCEED_WITHOUT_SAVE,
	STR_MCARD_RECHECK,
	STR_MCARD_DONTFORMAT,
	STR_MCARD_NEEDFORMAT,
	STR_MCARD_LOADING,
	STR_MCARD_LOADCOMPLETE,
	STR_MCARD_LOADERROR,
	STR_MCARD_NEEDS_ONE_BLOCK,
	STR_MCARD_CREATE_SAVE,
	STR_MCARD_CHANGED,
	STR_MCARD_CHECK_THIS_CARD,
	STR_MCARD_FORMATCOMPLETE,
	STR_SELECTOPTION,
	STR_X_SELECT_OPTION,
	STR_PAUSE_MODE,
	STR_TRAINING_COMPLETE,
	STR_NEED_PADS1,
	STR_NEED_PADS2,
	STR_RECORD,
	STR_MULTI_DESC_1,
	STR_MULTI_DESC_2,
	STR_MULTI_DESC_3,
	STR_MUS_VOL,
	STR_SFX_VOL,
	STR_SOUNDSTEREO,
	STR_SOUNDMONO,
	STR_LOADING,
	STR_SOUND_INSTR_1,
	STR_SOUND_INSTR_2,
	STR_WARNING_1,
	STR_WARNING_2,
	STR_CHAR_NAME_1,
	STR_CHAR_NAME_2,
	STR_CHAR_NAME_3,
	STR_CHAR_NAME_4,
	STR_CHAR_NAME_5,
	STR_CHAR_NAME_6,
	STR_CHAR_NAME_7,
	STR_CHAR_NAME_8,
	STR_DIFFICULTY_1,
	STR_DIFFICULTY_2,
	STR_DIFFICULTY_3,
	STR_DEMO_MODE,
	STR_OPTIONS,
	STR_START_GAME,
	STR_THE_END,
	STR_CREDITS_1,
	STR_CREDITS_2,
	STR_CREDITS_3,
	STR_CREDITS_4,
	STR_CREDITS_5,
	STR_CREDITS_6,
	STR_CREDITS_7,
	STR_CREDITS_8,
	STR_CREDITS_9,
	STR_CREDITS_10,
	STR_CREDITS_11,
	STR_CREDITS_12,
	STR_CREDITS_13,
	STR_CREDITS_14,
	STR_CREDITS_15,
	STR_CREDITS_16,
	STR_CREDITS_17,
	STR_CREDITS_18,
	STR_CREDITS_19,
	STR_CREDITS_20,
	STR_CREDITS_21,
	STR_CREDITS_22,
	STR_CREDITS_23,
	STR_CREDITS_24,
	STR_CREDITS_25,
	STR_CREDITS_26,
	STR_CREDITS_27,
	STR_CREDITS_28,
	STR_CREDITS_29,
	STR_CREDITS_30,
	STR_CREDITS_31,
	STR_CREDITS_32,
	STR_CREDITS_33,
	STR_CREDITS_34,
	STR_CREDITS_35,
	STR_CREDITS_36,
	STR_CREDITS_37,
	STR_CREDITS_38,
	STR_CREDITS_39,
	STR_CREDITS_40,
	STR_CREDITS_41,
	STR_CREDITS_42,
	STR_CREDITS_43,
	STR_CREDITS_44,
	STR_CREDITS_45,
	STR_CREDITS_46,
	STR_CREDITS_47,
	STR_CREDITS_48,
	STR_CREDITS_49,
	STR_CREDITS_50,
	STR_CREDITS_51,
	STR_CREDITS_52,
	STR_CREDITS_53,
	STR_CREDITS_54,
	STR_CREDITS_55,
	STR_CREDITS_56,
	STR_CREDITS_57,
	STR_CREDITS_58,
	STR_CREDITS_59,
	STR_CREDITS_60,
	STR_CREDITS_61,
	STR_CREDITS_62,
	STR_CREDITS_63,
	STR_CREDITS_64,
	STR_CREDITS_65,
	STR_CREDITS_66,
	STR_CREDITS_67,
	STR_CREDITS_68,
	STR_CREDITS_69,
	STR_CREDITS_70,
	STR_CREDITS_71,
	STR_CREDITS_72,
	STR_CREDITS_73,
	STR_CREDITS_74,
	STR_CREDITS_75,
	STR_CREDITS_76,
	STR_CREDITS_77,
	STR_CREDITS_78,
	STR_CREDITS_79,
	STR_CREDITS_80,
	STR_CREDITS_81,
	STR_CREDITS_82,
	STR_CREDITS_83,
	STR_CREDITS_84,
	STR_CREDITS_85,
	STR_CREDITS_86,
	STR_CREDITS_87,
	STR_CREDITS_88,
	STR_CREDITS_89,
	STR_CREDITS_90,
	STR_CREDITS_91,
	STR_CREDITS_92,
	STR_CREDITS_93,
	STR_CREDITS_94,
	STR_CREDITS_95,
	STR_CREDITS_96,
	STR_CREDITS_97,
	STR_CREDITS_98,
	STR_CREDITS_99,
	STR_CREDITS_100,
	STR_CREDITS_101,
	STR_CREDITS_102,
	STR_CREDITS_103,
	STR_CREDITS_104,
	STR_CREDITS_105,
	STR_CREDITS_106,
	STR_CREDITS_107,
	STR_CREDITS_108,
	STR_CREDITS_109,
	STR_CREDITS_110,
	STR_CREDITS_111,
	STR_CREDITS_112,
	STR_CREDITS_113,
	STR_CREDITS_114,
	STR_CREDITS_115,
	STR_CREDITS_116,
	STR_CREDITS_117,
	STR_CREDITS_118,
	STR_CREDITS_119,
	STR_CREDITS_120,
	STR_CREDITS_121,
	STR_CREDITS_122,
	STR_CREDITS_123,
	STR_CREDITS_124,
	STR_CREDITS_125,
	STR_CREDITS_126,
	STR_CREDITS_127,
	STR_CREDITS_128,
	STR_CREDITS_129,
	STR_CREDITS_130,
	STR_CREDITS_131,
	STR_CREDITS_132,
	STR_CREDITS_133,
	STR_CREDITS_134,
	STR_CREDITS_135,
	STR_CREDITS_136,
	STR_CREDITS_137,
	STR_CREDITS_138,
	STR_CREDITS_139,
	STR_CREDITS_140,
	STR_CREDITS_141,
	STR_CREDITS_142,
	STR_CREDITS_143,
	STR_CREDITS_144,
	STR_CREDITS_145,
	STR_CREDITS_146,
	STR_CREDITS_147,
	STR_CREDITS_148,
	STR_CREDITS_149,
	STR_CREDITS_150,
	STR_CREDITS_151,
	STR_CREDITS_152,
	STR_CREDITS_153,
	STR_CREDITS_154,
	STR_CREDITS_155,
	STR_CREDITS_156,
	STR_CREDITS_157,
	STR_CREDITS_158,
	STR_CREDITS_159,
	STR_CREDITS_160,
	STR_CREDITS_161,
	STR_CREDITS_162,
	STR_CREDITS_163,
	STR_CREDITS_164,
	STR_CREDITS_165,
	STR_CREDITS_166,
	STR_CREDITS_167,
	STR_CREDITS_168,
	STR_CREDITS_169,
	STR_CREDITS_170,
	STR_CREDITS_171,
	STR_CREDITS_172,
	STR_CREDITS_173,
	STR_CREDITS_174,
	STR_CREDITS_175,
	STR_CREDITS_176,
	STR_CREDITS_177,
	STR_CREDITS_178,
	STR_CREDITS_179,
	STR_CREDITS_180,
	STR_CREDITS_181,
	STR_CREDITS_182,
	STR_CREDITS_183,
	STR_CREDITS_184,
	STR_CREDITS_185,
	STR_CREDITS_186,
	STR_CREDITS_187,
	STR_CREDITS_188,
	STR_CREDITS_189,
	STR_CREDITS_190,
	STR_CREDITS_191,
	STR_CREDITS_192,
	STR_CREDITS_193,
	STR_CREDITS_194,
	STR_CREDITS_195,
	STR_CREDITS_196,
	STR_CREDITS_197,
	STR_CREDITS_198,
	STR_CREDITS_199,
	STR_CREDITS_200,
	STR_CREDITS_201,
	STR_CREDITS_202,
	STR_CREDITS_203,
	STR_CREDITS_204,
	STR_CREDITS_205,
	STR_CREDITS_206,
	STR_CREDITS_207,
	STR_CREDITS_208,
	STR_CREDITS_209,
	STR_CREDITS_210,
	STR_CREDITS_211,
	STR_CREDITS_212,
	STR_CREDITS_213,
	STR_CREDITS_214,
	STR_CREDITS_215,
	STR_CREDITS_216,
	STR_CREDITS_217,
	STR_CREDITS_218,
	STR_CREDITS_219,
	STR_CREDITS_220,
	STR_CREDITS_221,
	STR_CREDITS_222,
	STR_CREDITS_223,
	STR_CREDITS_224,
	STR_CREDITS_225,
	STR_CREDITS_226,
	STR_CREDITS_227,
	STR_CREDITS_228,
	STR_CREDITS_229,
	STR_CREDITS_230,
	STR_CREDITS_231,
	STR_CREDITS_232,
	STR_CREDITS_233,
	STR_CREDITS_234,
	STR_CREDITS_235,
	STR_CREDITS_236,
	STR_CREDITS_237,
	STR_CREDITS_238,
	STR_CREDITS_239,
	STR_CREDITS_240,
	STR_LEGAL_1,
	STR_LEGAL_2,
	STR_LEGAL_3,
	STR_LEGAL_4,
	STR_LEGAL_5,
	STR_LEGAL_6,
	STR_LEGAL_7,
	STR_LEGAL_8,
	STR_LEGAL_9,
	STR_PCSETUP_VIDEO,
	STR_PCSETUP_RESOLUTION,
	STR_PCSETUP_OK,
	STR_PCSETUP_CANCEL,
	STR_PCSETUP_CONTROLS,
	STR_PCSETUP_WINDOWED,
	STR_PCSETUP_SETUP,
	STR_PCSETUP_KEYBOARD,
	STR_PCSETUP_INSERTCD,
	STR_PCSETUP_BADVIDEO,
	STR_PCSETUP_PRESSKEY,
	STR_PCSETUP_DEFAULTKEYS,
	STR_LEVEL_PCMULTI1,
	STR_LEVEL_PCMULTI2,
	STR_LEVEL_PCMULTI3,
	STR_LEVEL_PCMULTI4,

#ifdef PC_VERSION
	STR_NET_NETWORKGAME,
	STR_NET_CREATEAGAME,
	STR_NET_FINDAGAME,
	STR_NET_STARTAGAME,
	STR_NET_PLAYERNAME,
	STR_NET_CONNECTION,
	STR_NET_CLICKJOIN,
	STR_NET_SEARCHING,
	STR_NET_FIND,
	STR_NET_JOIN,
	STR_NET_CREATE,
	STR_NET_PLAYERS,
	STR_NET_SEND,
	STR_NET_READY,
	STR_NET_GAMENAME,
	STR_NET_JOINED,
	STR_NET_LEFT,
	STR_NET_HOST,
	STR_NET_WAITINGPLAYERS,
	STR_NET_WAITINGHOST,
	STR_NET_SYNCH,
	STR_NET_FINISHED,
	STR_CONFIG_KEYUSED,
	STR_PCSETUP_DEFAULTS,
#endif

	LANG_NUM_STRINGS
};

// in case we ever need to debug or do anything sneaky, use a define here
#define GAMESTRING(n) ((((n)>0) && ((n)<LANG_NUM_STRINGS))?TEXTSTR(n):"out of range string")

#endif