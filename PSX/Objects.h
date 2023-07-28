#ifndef _OBJECTS_H_INCLUDE
#define _OBJECTS_H_INCLUDE

#define MAX_OBJECT_BANKS	7


void LoadObjectBank			( int objectbank );
void FreeObjectBank			( BFF_Header *header );
void FreeAllObjectBanks ( void );



#endif