/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: LevelData.c
	Programmer	: James Healey
	Date		: 14/06/99

----------------------------------------------------------------------------------------------- */


//#include "ultra64.h"
#include "defdata.h"

SCORENAME scoreName [ MAX_HISCORE_SLOTS ] = 
{
	{"jam" },
	{"jam" },
	{"jam" },
	{"jam" },
	{"jam" }
};

HISCORE	hiScoreData [ MAX_HISCORE_SLOTS ] = 
{
	{"", 10000 },
	{"", 9000  },
	{"", 8000  },
	{"", 7000  },
	{"", 6000  }
};

SAVE_SLOT saveSlots [ MAX_SAVE_SLOTS ] = 
{
	{"", 0 },
	{"", 0 },
	{"", 0 },
	{"", 0 },
	{"", 0 }
};