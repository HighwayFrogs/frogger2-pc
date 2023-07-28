/*

	This file is part of Frogger2, (c) 1999 Interactive Studios Ltd.


	File		: particle.c
	Programmer	: Jim Hubbard
	Date		: 12/03/00

----------------------------------------------------------------------------------------------- */

#define F3DEX_GBI_2

//#include <ultra64.h>

#include <islmem.h>

#include "types.h"
#include "Main.h"
#include "particle.h"
#include "layout.h"


// Used for runtime scaling of particle effects


#ifdef PC_VERSION
#define MAX_PARTICLES 512
#else
#define MAX_PARTICLES 168
#endif


void UpdatePFXJet( SPECFX *fx );
void UpdatePFXTeleIdle( SPECFX *fx );
void UpdatePFXTeleActive( SPECFX *fx );


/*	--------------------------------------------------------------------------------
	Function		: CreateParticleEffect
	Purpose			: Subclass of special effect where particle type effects can morph between types in realtime.
	Parameters		: allocated effect to create
	Returns			: 
	Info			: 
*/
SPECFX *CreateParticleEffect( SPECFX *fx )
{
	SPRITE *s;
	PARTICLE *p;
	FVECTOR up;
	long i;//, life = max( ((fx->lifetime-actFrameCount)/60), 1 );

	// Set number of particles to the number calculated by current performance
 	fx->numP = PFX_NUMPARTICLES;
 	i = fx->numP;

	// Allocate particles - sprites may not be used, so they are done per effect
	if( !(fx->particles = AllocateParticles(i)) )
	{
		DeallocateFX(fx,1);
		return NULL;
	}
	p = fx->particles;

	SetVectorFF( &up, &fx->normal );
	ScaleVector( &up, 200 );
	AddToVectorSF( &fx->origin, &up );

	// Different states for different types - may change when morphing to new state
	switch( fx->type )
	{
	case PTYPE_TELEPORTIDLE:
		// Swirl about a bit

		break;

	case PTYPE_TELEPORTACTIVE:
		// Move in to form a frog shape, maybe

		break;

	case PTYPE_WATERJET:
	case PTYPE_JET:
		if( !(fx->sprites = AllocateSprites( fx->numP )) )
		{
			DeallocateFX( fx,1 );
			return NULL;
		}
		if( (fx->rebound = AllocateP2()) )
		{
 			SetVectorSS( &fx->rebound->point, &fx->origin );
 			SetVectorFF( &fx->rebound->normal, &fx->normal );
		}
 
		fx->tex = txtrSolidRing;
// 		fx->fade = max(fx->a/life, 1);
 
 		s = fx->sprites;
		while(i--)
 		{
 			SetVectorSS( &s->pos, &fx->origin );
 
 			s->r = fx->r;
 			s->g = fx->g;
 			s->b = fx->b;
 			s->a = 255;
 
 			s->offsetX = -16;
 			s->offsetY = -16;
 			s->flags = SPRITE_TRANSLUCENT | SPRITE_ADDITIVE;
 
 			// Velocity is normal scaled by speed, plus a random offset scaled by speed
 			SetVectorFF( &p->vel, &fx->normal );
 			ScaleVectorFF( &p->vel, fx->speed );
 			p->vel.vx += (Random(30)-10)*(fx->speed/100);
 			p->vel.vy += (Random(30)-10)*(fx->speed/100);
 			p->vel.vz += (Random(30)-10)*(fx->speed/100);

			s->scaleX = (fx->scale.vx>>12)/SCALE;
 			s->scaleY = (fx->scale.vy>>12)/SCALE;
			s->texture = fx->tex;
 
 			s = s->next;
			p = p->next;
 		}
			
 		fx->Update = UpdatePFXJet;
 		fx->Draw = NULL;
		break;
	}

	return fx;
}


/*	--------------------------------------------------------------------------------
	Function		: UpdatePFXJet
	Purpose			: Fountain or hose or something
	Parameters		: fx
	Returns			: 
	Info			: 
*/
void UpdatePFXJet( SPECFX *fx )
{
	fixed dist;//, vS;
//	int i = fx->numP, j, fo, ele, alive=0;
	int i = fx->numP, alive=0;
	FVECTOR up;
	SPRITE *s;
	PARTICLE *p;

 	if( fx->follow )
 		SetVectorSS( &fx->origin, &fx->follow->position );

	s = fx->sprites;
	p = fx->particles;
	while(i--)
	{
		if( p->bounce ) continue;
		alive++;

		if( fx->gravity )
		{
			SetVectorFF( &up, &fx->normal );
			ScaleVectorFF( &up, FMul(fx->gravity,gameSpeed) );
			SubFromVectorFF( &p->vel, &up );
		}

		if( fx->rebound )
		{
			FVECTOR fRP;
			FVECTOR fSP;
			SetVectorFS(&fRP,&fx->rebound->point);
			fx->rebound->J = -DotProductFF( &fRP, &fx->rebound->normal );
			SetVectorFS(&fSP,&s->pos);
			dist = -(DotProductFF(&fSP, &fx->rebound->normal) + fx->rebound->J);

			// check if particle has hit (or passed through) the plane
			if(dist > 0)
			{
				p->bounce = 1;
				s->a = 0;
				s->draw = 0;
				// check if this particle type triggers some other effect or event
				if( fx->type == PTYPE_WATERJET )
					if( dist < 204800 )
						CreateSpecialEffect( FXTYPE_WAKE, &s->pos, &fx->rebound->normal, 20480, 819, 410, 1229 );
			}
		}

		s->pos.vx += (FMul(p->vel.vx, gameSpeed)>>12);
		s->pos.vy += (FMul(p->vel.vy, gameSpeed)>>12);
		s->pos.vz += (FMul(p->vel.vz, gameSpeed)>>12);

//		fo = (Random(4) + fx->fade) * (gameSpeed>>12);
//		if( s->a > fo ) s->a -= fo;
//		else s->a = 0;

//		if( !s->a )
//		{
//			s->draw = 0;
//			p->bounce = 1;
//		}

		// particle has died, respawn if the entire effect is alive
		if( p->bounce && ((fx->lifetime==-1) || (actFrameCount < fx->lifetime)) )
		{
			p->bounce = 0;

	 		SetVectorSS( &s->pos, &fx->origin );

 			s->r = fx->r;
 			s->g = fx->g;
 			s->b = fx->b;
 			s->a = 255;
			s->draw = 1;

 			// Velocity is normal scaled by speed, plus a random offset scaled by speed
 			SetVectorFF( &p->vel, &fx->normal );
 			ScaleVectorFF( &p->vel, fx->speed );
 			p->vel.vx += (Random(30)-10)*(fx->speed/100);
			p->vel.vy += (Random(30)-10)*(fx->speed/100);
			p->vel.vz += (Random(30)-10)*(fx->speed/100);
		}

		s = s->next;
		p = p->next;
	}

	if( !alive && ((fx->lifetime!=-1) && (actFrameCount > fx->lifetime)) )
		DeallocateFX(fx, 1);
}


/*	--------------------------------------------------------------------------------
	Function		: AllocateParticles
	Purpose			: Find a number of particles and return a sublist
	Parameters		: number
	Returns			: pointer to first one0 
	Info			: 
*/
PARTICLE *AllocateParticles( int number )
{
	PARTICLE *p;
	// Return if allocation is impossible for any reason
	if( (number <= 0) || (partList.stackPtr-number < 0) || (number >= MAX_PARTICLES-partList.count) )
//	{
//		utilPrintf ( "Could not find enough space to allocate particles. Number : %d\n", number );
		return NULL;
//	}


	// Now we can go and allocate particles with gay abandon
	while( number-- )
	{
		p = partList.stack[partList.stackPtr--];

		p->prev	= &partList.head;
		p->next	= partList.head.next;
		partList.head.next->prev = p;
		partList.head.next = p;

		partList.count++;
	}

	return partList.stack[partList.stackPtr+1];
}


/*	--------------------------------------------------------------------------------
	Function		: DeallocateParticles
	Purpose			: Remove particles from list and flag as unused
	Parameters		: number of particles
	Returns			: 
	Info			: 
*/
void DeallocateParticles( PARTICLE *head, int number )
{
	PARTICLE *p=head, *t;

	if( !p || !p->next || (number<=0) || (partList.stackPtr+number >= MAX_PARTICLES) ) return;

	while( number-- )
	{
		t = p->next;

		p->prev->next	= p->next;
		p->next->prev	= p->prev;
		p->next = NULL;

		// Deallocate any random data that may exist and set everything to zero
//		if( p->rMtrx ) FREE( p->rMtrx );
		memset( p, 0, sizeof(PARTICLE) );

		partList.count--;
		partList.stack[++partList.stackPtr] = p;

		p = t;
	}
}


/*	--------------------------------------------------------------------------------
	Function		: FreeParticleList
	Purpose			: Deallocate all particles and set list to empty
	Parameters		: 
	Returns			: 
	Info			: 
*/
void FreeParticleList( )
{
	int i;

	if( partList.array )
	{
		// Remove all particles in array from particle list so they don't get removed after deallocation
		for( i=0; i<MAX_PARTICLES; i++ )
		{
			if( !partList.array[i].next ) continue;

			partList.array[i].prev->next	= partList.array[i].next;
			partList.array[i].next->prev	= partList.array[i].prev;
			partList.array[i].next = NULL;
		}

		FREE( partList.array );
		partList.array = NULL;
	}

	if( partList.stack )
	{
		FREE( partList.stack );
		partList.stack = NULL;
	}

	partList.count = 0;
	partList.stackPtr = 0;
}


/*	--------------------------------------------------------------------------------
	Function		: InitParticleList
	Purpose			: Set up static array and some bits of data.
	Parameters		: 
	Returns			: 
	Info			: 
*/
void InitParticleList( )
{
	int i;

	partList.head.next = partList.head.prev = &partList.head;

	// Allocate a big bunch of particles
	partList.array = (PARTICLE *)MALLOC0( sizeof(PARTICLE)*MAX_PARTICLES );
	partList.stack = (PARTICLE **)MALLOC0( sizeof(PARTICLE*)*MAX_PARTICLES );

	// Initially, all particles are available
	for( i=0; i<MAX_PARTICLES; i++ )
		partList.stack[i] = &partList.array[i];

	partList.count = 0;
	partList.stackPtr = i-1;
}

