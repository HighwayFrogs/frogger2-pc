/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: general.c
	Programmer	: Matthew Cloy
	Date		: 12/01/99

----------------------------------------------------------------------------------------------- */

//#include "ultra64.h"
#include <islfile.h>
#include <islmem.h>
#include "maths.h"
#include "map.h"
#include "main.h"
#include "frogger.h"
#include "enemies.h"
#include "memload.h"

#ifdef PC_VERSION
#include <pcaudio.h>
#endif
#ifdef PSX_VERSION
#include "audio.h"
#endif

#include "general.h"

SAMPLE **sfx_anim_map = NULL;

char statusMessage[255];
char message[256];

unsigned long gstrlen (const char *a)
{
	char *b = (char*)a;
	unsigned long i;
	for (i=0; b[0]==0; b++,i++);
	return i;
}

unsigned long gstrcmp(char *a,char *b)
{
	char *x = a,*y = b;

	while ((x[0] && y[0]) && (x[0]==y[0]))
	{
		x++;
		y++;
	}
	
	if (x[0]!=0)
		return 1;

	if (y[0]!=0)
		return 1;
	
	return 0;
}

char *gstrlwr( char *str )
{
	char *x=str;

	while( *x )
	{
		if( *x >= 'A' && *x <= 'Z' )
			*x += 32;
		x++;
	}

	return str;
}

char *gstrupr( char *str )
{
	char *x=str;

	while( *x )
	{
		if( *x >= 'a' && *x <= 'z' )
			*x -= 32;
		x++;
	}

	return str;
}

void stringChange ( char *name )
{
	char *c, *d, *e;
	const char *obe = ".psi";
	const char *ndo = ".obe";

	for(c = name; *c; c++)
	{
		d = (char*)ndo, e = c;
		while(*e == *d)	// strstr!
		{
			e++, d++;
			if (!*d) {	// end of ".obe" - found substring
				d = (char*)obe;
				while(*d) *(c++) = *(d++);	// strcpy!
				return;
			}
		}
	}
}


void stringChangePSIToOBE ( char *name )
{
	char *c, *d, *e;
	const char *obe = ".obe";
	const char *ndo = ".psi";

	for(c = name; *c; c++)
	{
		d = (char*)ndo, e = c;
		while(*e == *d)	// strstr!
		{
			e++, d++;
			if (!*d) {	// end of ".obe" - found substring
				d = (char*)obe;
				while(*d) *(c++) = *(d++);	// strcpy!
				return;
			}
		}
	}
}


inline void cmemcpy( char *a, char *b, unsigned long size )
{
	while( size-- )
		a[size] = b[size];
}

inline void smemcpy( unsigned short *a, unsigned short *b, unsigned long size )
{
	while( size-- )
		a[size] = b[size];
}

inline void lmemcpy( long *a, long *b, long size )
{
	while( size-- )
		a[size] = b[size];
}


/*	--------------------------------------------------------------------------------
    Function		: FindNearestTile
	Parameters		: VECTOR
	Returns			: GAMETILE*
*/

GAMETILE* FindNearestTileV(VECTOR v)
{
	SVECTOR sv;
	SetVectorSV(&sv,&v);
	return FindNearestTileS(sv);
}

GAMETILE* FindNearestTileF(FVECTOR v)
{
	SVECTOR sv;
	SetVectorSF(&sv,&v);
	return FindNearestTileS(sv);
}

GAMETILE* FindNearestTileS(SVECTOR v)
{
	GAMETILE *t, *closest;
	int dist, closestDist, i;

	closest = NULL; closestDist = 500000;

	t = firstTile;
	for ( i = 0; i < tileCount; i++, t++ )
	{
		dist = DistanceBetweenPointsSS(&v, &t->centre)>>12;

		if (dist < closestDist)
		{
			closest = t;
			closestDist = dist;
		}
	}

	return closest;
}

GAMETILE* FindNearestJoinedTile( GAMETILE *tile, SVECTOR *pos )
{
	GAMETILE *t, *closest = tile;
	int i, j;
	fixed dist,
		closestDist = DistanceBetweenPointsSS(pos,&tile->centre); // Closest is current tile initially

	for( i=0; i<4; i++ )
	{
		t = tile->tilePtrs[i];
		if( t )
		if( (t->state != TILESTATE_BARRED) && (t->state != TILESTATE_JOIN) )
		{
			dist = DistanceBetweenPointsSS(pos, &t->centre);
			if (dist < closestDist)
			{
				closest = t;
				closestDist = dist;
			}

			for( j=0; j<4; j++ )
			{
				if( t->tilePtrs[j] )
				if( (t->tilePtrs[j]->state != TILESTATE_BARRED) && (t->tilePtrs[j]->state != TILESTATE_JOIN) )
				{
					dist = DistanceBetweenPointsSS(pos, &t->tilePtrs[j]->centre);
					if (dist < closestDist)
					{
						closest = t->tilePtrs[j];
						closestDist = dist;
					}
				}
			}
		}
	}

	return closest;
}

GAMETILE *FindJoinedTileByDirection( GAMETILE *st, FVECTOR *d )
{
	fixed distance = ToFixed(100000), t;
	int i;
	GAMETILE *res = NULL;

	for (i=0; i<4; i++)
	{
		t = DotProductFF( d, &st->dirVector[i] );
		if( (t < distance) && st->tilePtrs[i] )
		if( (st->tilePtrs[i]->state != TILESTATE_DEADLY) && (st->tilePtrs[i]->state != TILESTATE_SINK) && (st->tilePtrs[i]->state != TILESTATE_BARRED) && (st->tilePtrs[i]->state != TILESTATE_JOIN) )
		{
			distance = t;
			res = st->tilePtrs[i];
		}
	}

	return res;
}

// As above, but constrained to an angle
GAMETILE *FindJoinedTileByDirectionConstrained(GAMETILE *st, FVECTOR *d, fixed angle)
{
	fixed distance = ToFixed(100000), t;
	int i;
	GAMETILE *res = NULL;

	for (i=0; i<4; i++)
	{
		t = DotProductFF( d, &st->dirVector[i] );
		if( (t < distance) && (t < rcos(angle)) && st->tilePtrs[i] )
		if( (st->tilePtrs[i]->state != TILESTATE_FALL) && (st->tilePtrs[i]->state != TILESTATE_DEADLYFALL) && (st->tilePtrs[i]->state != TILESTATE_DEADLY) && (st->tilePtrs[i]->state != TILESTATE_SINK) && (st->tilePtrs[i]->state != TILESTATE_BARRED) && (st->tilePtrs[i]->state != TILESTATE_JOIN) )
		{
			distance = t;
			res = st->tilePtrs[i];
		}
	}

	return res;
}

GAMETILE *FindJoinedTileByDirectionAndType( GAMETILE *st, FVECTOR *d, int type )
{
	fixed distance = ToFixed(100000), t;
	int i;
	GAMETILE *res = NULL;

	for (i=0; i<4; i++)
	{
		t = DotProductFF( d, &st->dirVector[i] );
		if( (t < distance) && st->tilePtrs[i] )
		if( st->tilePtrs[i]->state == type )
		{
			distance = t;
			res = st->tilePtrs[i];
		}
	}

	return res;
}

// Which direction vector most closely matches the vector
SVECTOR *FindClosestTileVector( GAMETILE *tile, FVECTOR *d )
{
	fixed distance = -100000, t;
	SVECTOR *res;
	int i;

	for (i=0; i<4; i++)
	{
		t = DotProductFF( d, &tile->dirVector[i] );
		if( t > distance )
		{
			distance = t;
			res = &tile->dirVector[i];
		}
	}

	return res;
}


/*	--------------------------------------------------------------------------------
	Function 	: LoadSfxMapping
	Purpose 	: Load a mapping of animations to sound effects for a list of actors
	Parameters 	: world and level ID's
	Returns 	: Pointer to mapping array
	Info 		: 
*/
void LoadSfxMapping( int world, int level )
{
	unsigned char *in;
	unsigned char *buffer;
//	HANDLE h;
	char filename[128], wnum[6];
	long num, /*read, */index, type, i, count, uid;
	int size;


// JIM: UUuuuuurrrrrrrghhhhhhh - Random!
/*
#ifdef PSX_VERSION
#ifdef GOLDCD
	if ( ( ( world == 8 ) && ( level == 0 ) ) ||
			 ( ( world == 2 ) && ( level == 1 ) ) ||
			 ( ( world == 5 ) && ( level == 1 ) ) ||
			 ( ( world == 5 ) && ( level == 2 ) ) ||
			 ( ( world == 6 ) && ( level == 2 ) ) ||

			 ( ( world == 1 ) && ( level == 5 ) ) ||
			 ( ( world == 1 ) && ( level == 3 ) ) ||
			 ( ( world == 5 ) && ( level == 3 ) ) ||

			 ( ( world == 5 ) && ( level == 4 ) ) ||
			 ( ( world == 5 ) && ( level == 5 ) ) ||

			 ( ( world == 7 ) && ( level == 10 ) ) ||
			 ( ( world == 7 ) && ( level == 11 ) ) ||
			 ( ( world == 7 ) && ( level == 12 ) ) ||
			 ( ( world == 7 ) && ( level == 13 ) ) ||

			 ( ( world == 7 ) && ( level == 0 ) ) ||
			 ( ( world == 7 ) && ( level == 1 ) ) ||
			 ( ( world == 7 ) && ( level == 2 ) ) ||
			 ( ( world == 7 ) && ( level == 3 ) ) ||
			 ( ( world == 7 ) && ( level == 4 ) ) ||
			 ( ( world == 7 ) && ( level == 5 ) ) ||
			 ( ( world == 7 ) && ( level == 6 ) ) ||
			 ( ( world == 7 ) && ( level == 7 ) ) ||
			 ( ( world == 7 ) && ( level == 8 ) ) ||
			 ( ( world == 7 ) && ( level == 9 ) ) ||

			 ( ( world == 8 ) && ( level == 3 ) ) )
	{
		return;
	}
#endif
#endif
*/

	// Directory and start of sfxmap filename
//	strcpy( filename, baseDirectory );
#ifdef PC_VERSION
	strcpy( filename, "MAPS\\SFXANIM" );
#endif
#ifdef PSX_VERSION
	strcpy( filename, "SFXANIM" );
#endif
	// World and level ids are part of the filename

	// Represent level number in hex so we can have up to 16 levels instead of 10 in a world :)
	sprintf( wnum, "%d%x", world, level/*(level<=9)?('0'+level):('a'+(level-10))*/ );
	strcat(filename,wnum);

/*
	_itoa( world, wnum, 10 );
	strcat( filename, wnum );
	_itoa( level, lnum, 10 );
	strcat( filename, lnum );
*/

	// Extension
	strcat( filename, ".SAM" );

/*
	// Open input file
	h = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if (h == INVALID_HANDLE_VALUE)
	{
		utilPrintf("Couldn't load sfx map file %s\n", filename);
		FREE( filename );
		return;
	}

	// Get file info and data
	size = GetFileSize(h, NULL);
	buffer = MALLOC0( size );
	ReadFile( h, buffer, size, &read, NULL );
	CloseHandle(h);
*/

#ifdef PC_VERSION
	buffer = fileLoad(filename,&size);
#endif

#ifdef PSX_VERSION
	buffer = FindStakFileInAllBanks ( filename, &size );
//	buffer = fileLoad(filename,&size);
#endif

	if(buffer == NULL)
		return;

	in = (unsigned char *)buffer;
	sfx_anim_map = (SAMPLE **)MALLOC0( size+8 );

	index=0;
	count=0;

	// Read data from buffer into mapping array
	while( count < size-4 )
	{
		// Actor uid
		uid = MEMGETINT(&in);
		count+=4;

		// Actor type
		type = MEMGETINT(&in);
		count+=4;

		// If a tile based sample, skip the normal mapping load and just make an ambient
		if( type == 4 && uid < tileCount )
		{
			AddAmbientSound( FindSample(MEMGETINT(&in)), &firstTile[uid].centre, 12000, AMBIENT_VOLUME, -1, 0, 0, NULL );
			count+=4;
			continue;
		}

		// If not tile based, store the previously loaded stuff in the sample map
		sfx_anim_map[index++] = (SAMPLE *)uid;
		sfx_anim_map[index++] = (SAMPLE *)type;

		switch( type )
		{
		case 0: num = NUM_FROG_ANIMS; break;
		case 1: num = MULTI_NUM_ANIMS; break;
		case 2: num = NUM_NME_ANIMS; break;
		case 3: num = 1; break; // NUM_SCENIC_ANIMS
		case 4: num = 1; break; // NUM_SCENIC_ANIMS
		default: num = 0; break;
		}

		for( i=0; i<num; i++ )
		{
			// Locate samples by their uids
			sfx_anim_map[index++] = FindSample( MEMGETINT(&in) );
			count+=4;
		}
	}

	// Signal end of list
	sfx_anim_map[index] = NULL;

//	FREE( filename );

#ifdef PC_VERSION
	FREE( buffer );
#endif
}


