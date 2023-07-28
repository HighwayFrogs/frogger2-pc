/******************************************************************************************
	AM2 PS   (c) 2000-1 ISL		-=Modified for Chicken Run 27/3/2000 Barry Paterson=-

	language.h:		Language selection
******************************************************************************************/

#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

//#include "gametext.h"
#include "lang.h"
#include <isltex.h>

#define BONUS_FLAGS	13

extern int DoneLangSel;

typedef struct _LanguageDataType
{
	long			frame;
	TextureBankType	*flagTextures;
	TextureType		*flagTexture[LANG_NUMLANGS+BONUS_FLAGS];
	long lastFade;

} LanguageDataType;

void languageInitialise();
void languageFrame();
void languageDestroy();
void calculateTileTexture(int x, int y);

#endif
