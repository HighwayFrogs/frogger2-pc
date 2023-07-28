/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.

	File		: anim.h
	Programmer	: Andy Eder
	Date		: 29/04/99 10:32:50

----------------------------------------------------------------------------------------------- */

#ifndef ANIM_H_INCLUDED
#define ANIM_H_INCLUDED

#include "actor2.h"

#define ANIM_RANDOM_START	(1 << 0)

#ifdef __cplusplus
extern "C" {
#endif

// Frogger death functions
void DeathNormal( int pl );
void DeathRunOver( int pl );
void DeathDrowning( int pl );
void DeathSquashed( int pl );
void DeathFire( int pl );
void DeathElectric( int pl );
void DeathFalling( int pl );
void DeathWhacking( int pl );
void DeathInflation( int pl );
void DeathPoison( int pl );
void DeathSlicing( int pl );
void DeathExplosion( int pl );
void DeathGibbing( int pl );
void DeathIncinerate( int pl );
void DeathBursting( int pl );
void DeathSpiked( int pl );
void DeathMulti( int pl );
void DeathLightning( int pl );


#ifdef __cplusplus
}
#endif
#endif
