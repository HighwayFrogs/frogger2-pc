#include <string.h>
#include <stdio.h>

#include "shell.h"
//#include "global.h"
#include "sonylibs.h"
#include <islutil.h>
//#include <islpsi.h>

#include "islpsi.h"

#include "Layout.h"
#include "bff_load.h"
#include "Objects.h"

#include "world.h"
#include "bff_load.h"

extern int numObjectBanks;

extern BFF_Header *objectBanks [ MAX_OBJECT_BANKS ];

extern int		PSIFileListPos;
extern char	PSIFileList [ 10 ][12];


void LoadObjectBank ( int objectBank )
{
	char BFFfileName[256];
	char PILfileName[256];

	switch ( objectBank )
	{
		case FROGGER_OBJ_BANK:
					sprintf ( PILfileName, "FROGGER.PIL" );					
				break;

		case LILLI_OBJ_BANK:
					sprintf ( PILfileName, "LILLI.PIL" );					
				break;

		case TAD_OBJ_BANK:
					sprintf ( PILfileName, "TAD.PIL" );					
				break;

		case SWAMPY_OBJ_BANK:
					sprintf ( PILfileName, "SWAMPY.PIL" );					
				break;

		case TWEE_OBJ_BANK:
					sprintf ( PILfileName, "TWEE.PIL" );					
				break;

		case WART_OBJ_BANK:
					sprintf ( PILfileName, "TOAD.PIL" );					
				break;

		case ROOBIE_OBJ_BANK:
					sprintf ( PILfileName, "ROOBIE.PIL" );					
				break;

		case TANK_OBJ_BANK:
					sprintf ( PILfileName, "TANK.PIL" );					
				break;

		case MFROGGER_OBJ_BANK:
					sprintf ( PILfileName, "MFROGGER.PIL" );					
				break;
		case MLILLI_OBJ_BANK:
					sprintf ( PILfileName, "MLILLI.PIL" );					
				break;
		case MTAD_OBJ_BANK:
					sprintf ( PILfileName, "MTAD.PIL" );					
				break;
		case MSWAMPY_OBJ_BANK:
					sprintf ( PILfileName, "MSWAMPY.PIL" );					
				break;
		case MTWEE_OBJ_BANK:
					sprintf ( PILfileName, "MTWEE.PIL" );					
				break;
		case MWART_OBJ_BANK:
					sprintf ( PILfileName, "MTOAD.PIL" );					
				break;
		case MROOBIE_OBJ_BANK:
					sprintf ( PILfileName, "MROOBIE.PIL" );					
				break;
		case MTANK_OBJ_BANK:
					sprintf ( PILfileName, "MTANK.PIL" );					
				break;

/*	// Multiplayer object banks
	MFROGGER_OBJ_BANK,
	MLILLI_OBJ_BANK,
	MTAD_OBJ_BANK,
	MSWAMPY_OBJ_BANK,
	MTWEE_OBJ_BANK,
	MWART_OBJ_BANK,
	MROOBIE_OBJ_BANK,
	MTANK_OBJ_BANK,*/

		case INGAMEGENERIC_OBJ_BANK:
					sprintf ( BFFfileName, "OBJECTS\\GENERIC.BFF" );					
					sprintf ( PILfileName, "GENERIC.PIL" );					
				break;

		case GARDENLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "GARDEN1.BFF" );
				sprintf ( PILfileName, "GARDEN1.PIL" );
			break;

		case GARDENMULTI_OBJ_BANK:
				sprintf ( BFFfileName, "GARDENMULTI.BFF" );
				sprintf ( PILfileName, "GARDEN1.PIL" );
			break;

		case ANCIENTLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "ANCIENTS1.BFF" );
				sprintf ( PILfileName, "ANCIENTS1.PIL" );
			break;

		case ANCIENTLEV3_OBJ_BANK:
				sprintf ( BFFfileName, "ANCIENTS2.BFF" );
				sprintf ( PILfileName, "ANCIENTS2.PIL" );
			break;

		case ANCIENTBOSS_OBJ_BANK:
				sprintf ( BFFfileName, "ANCIENTSBOSS.BFF" );
				sprintf ( PILfileName, "ANCIENTSBOSS.PIL" );
			break;

		case ANCIENTMULTI_OBJ_BANK:
				sprintf ( BFFfileName, "ANCIENTSMULTI.BFF" );
				sprintf ( PILfileName, "ANCIENTSMULTI.PIL" );
			break;

		case SPACELEV1_OBJ_BANK:
				sprintf ( BFFfileName, "SPACE1.BFF" );
				sprintf ( PILfileName, "SPACE1.PIL" );
			break;

		case SPACELEV3_OBJ_BANK:
				sprintf ( BFFfileName, "SPACE2.BFF" );
				sprintf ( PILfileName, "SPACE2.PIL" );
			break;

		case SPACEBOSS_OBJ_BANK:
				sprintf ( BFFfileName, "SPACE3.BFF" );
				sprintf ( PILfileName, "SPACE3.PIL" );
			break;

		case CITYLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "CITY1.BFF" );
				sprintf ( PILfileName, "CITY1.PIL" );
			break;

		case CITYLEV2_OBJ_BANK:
				sprintf ( BFFfileName, "CITY2.BFF" );
				sprintf ( PILfileName, "CITY2.PIL" );
			break;

		case CITYLEV3_OBJ_BANK:
				sprintf ( BFFfileName, "CITY3.BFF" );
				sprintf ( PILfileName, "CITY3.PIL" );
			break;

		case CITYMULTI_OBJ_BANK:
				sprintf ( BFFfileName, "CITYMULTI.BFF" );
				sprintf ( PILfileName, "CITY3.PIL" );
			break;

		case SUBTERRANEANLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "SUB1.BFF" );
				sprintf ( PILfileName, "SUB1.PIL" );
			break;
		
		case SUBTERRANEANLEV2_OBJ_BANK:
				sprintf ( BFFfileName, "SUB2.BFF" );
				sprintf ( PILfileName, "SUB2.PIL" );
			break;

		case SUBTERRANEANBOSS_OBJ_BANK:
				sprintf ( BFFfileName, "SUB4.BFF" );
				sprintf ( PILfileName, "SUB4.PIL" );
			break;

		case LABORATORYLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "LAB1.BFF" );
				sprintf ( PILfileName, "LAB1.PIL" );
			break;

		case LABORATORYLEV3_OBJ_BANK:
				sprintf ( BFFfileName, "LAB2.BFF" );
				sprintf ( PILfileName, "LAB2.PIL" );
			break;

		case LABORATORYBOSSA_OBJ_BANK:
				sprintf ( BFFfileName, "LABBOSS.BFF" );
				sprintf ( PILfileName, "LABBOSS.PIL" );
			break;

		case LABORATORYMULTI1_OBJ_BANK:
				sprintf ( BFFfileName, "LABMULTI.BFF" );
				sprintf ( PILfileName, "LABMULTI" );
			break;

		case LABORATORYMULTI2_OBJ_BANK:
				sprintf ( BFFfileName, "LABMULTI2.BFF" );
				sprintf ( PILfileName, "LABMULTI2.PIL" );
			break;

		case LABORATORYMULTI3_OBJ_BANK:
				sprintf ( BFFfileName, "LABMULTI3.BFF" );
				sprintf ( PILfileName, "LABMULTI3.PIL" );
			break;

		case HALLOWEENLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "HALLOWEEN1.BFF" );
				sprintf ( PILfileName, "HALLOWEEN1.PIL" );
			break;

		case HALLOWEENLEV2_OBJ_BANK:
				sprintf ( BFFfileName, "HALLOWEEN2.BFF" );
				sprintf ( PILfileName, "HALLOWEEN2.PIL" );
			break;

		case HALLOWEENLEV3_OBJ_BANK:
				sprintf ( BFFfileName, "HALLOWEEN3.BFF" );
				sprintf ( PILfileName, "HALLOWEEN3.PIL" );
			break;

		case HALLOWEENBOSS_OBJ_BANK:
				sprintf ( BFFfileName, "HALLOBOSS.BFF");
				sprintf ( PILfileName, "HALLOBOSS.PIL" );
			break;


		case HALLOWEENBOSSB_OBJ_BANK:
				sprintf ( BFFfileName, "HALLOBOSSB.BFF" );
				sprintf ( PILfileName, "HALLOBOSSB.PIL" );
			break;

		case RETROLEV1_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO1.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV2_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO2.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV3_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO3.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV4_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO4.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV5_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO5.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV6_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO6.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV7_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO7.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV8_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO8.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV9_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO9.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROLEV10_OBJ_BANK:
				sprintf ( BFFfileName, "RETRO10.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI1_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI2_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI2.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI3_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI3.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI4_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI4.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI5_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI5.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI6_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI6.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI7_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI7.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI8_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI8.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI9_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI9.BFF" );
				//sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case RETROMULTI10_OBJ_BANK:
				sprintf ( BFFfileName, "RETROMULTI10.BFF" );
//				sprintf ( PILfileName, "OBJECTS\\RETRO\\SUPER.PIL" );
			break;

		case FRONTEND_OBJ_BANK:
				sprintf ( PILfileName, "HUB.PIL" );
			break;

		case FRONTEND1_OBJ_BANK:
 				sprintf ( BFFfileName, "HUB1.BFF" );
			break;

		case FRONTEND2_OBJ_BANK:
				sprintf ( BFFfileName, "LEVEL.BFF" );
			break;

		case FRONTEND4_OBJ_BANK:
				sprintf ( BFFfileName, "TRAINING.BFF" );
				sprintf ( PILfileName, "TRAIN.PIL" );
			break;

		default:
				//utilPrintf ( "Not a valid Object Bank type...............\n" );
				return;
			break;
	}
	// ENDSWITCH - textureBank

	if ( numObjectBanks < MAX_OBJECT_BANKS )
	{
		char *c = strstr ( BFFfileName, ".BFF" );

		if ( c )
		{
			int i;
			FMA_MESH_HEADER **mesh;


			//utilPrintf("Loading BFF World %s \n", BFFfileName);
					//GetWorldFrom_BFF(fileName);
			objectBanks [ numObjectBanks++ ] = BFF_LoadFile ( BFFfileName );
			fma_world = (void *)BFF_FindObject(BFF_FMA_WORLD_ID,utilStr2CRC("WORLD.PSI"));	// Should only be one world in the BFF

			if ( !fma_world )
			{
				utilPrintf("World Object Not Found\n");
			}
			else
			{

				mesh = ADD2POINTER(fma_world,sizeof(FMA_WORLD));
				
				for ( i = fma_world->n_meshes; i != 0; i--, mesh++ )
				{
					(*mesh)->flags |= DRAW_SEGMENT;
				}
			}
			// ENDIF
		}
		else
		{
			//utilPrintf("ERROR: Unrecognised BFF file type:  %s \n", BFFfileName);
		}
		// ENDELSEIF

		if ( ( c = strstr ( PILfileName, ".PIL" ) ) )
		{
			//utilPrintf("Loading PIL Library %s \n", PILfileName);
			psiLoadPIL  ( PILfileName );
		}
		else
		{
			//utilPrintf("ERROR: Unrecognised PIL file type:  %s \n", PILfileName);
		}
		// ENDELSEIF
	}
	else
	{
		//utilPrintf("Error Loading: %s : %s, Max Object Banks Reached\n", PILfileName, BFFfileName );
	}
	// ENDELSEIF - 	if ( numObjectBanks < MAX_OBJECT_BANKS )

//	ExtractPsiNames();

	sprintf ( PILfileName, "" );					
	sprintf ( BFFfileName, "" );					


}


void FreeObjectBank ( BFF_Header *header )
{
	BFF_ReleaseFile ( header );
}

void FreeAllObjectBanks ( void )
{
	short c;

	//utilPrintf("Freeing All Object Banks : OBJECT\n");

	for ( c = 0; c < MAX_OBJECT_BANKS; c++ )
	{
		if ( objectBanks[c] )
			FreeObjectBank ( objectBanks [c] );
		// ENDIF - if ( textureBanks[c] )
	}
	// ENDIF - 	for ( c = o; c < MAX_TEXTURE_BANKS; c++ )
//MM	FREE( ( FMA_WORLD* ) fma_world );
	FREE( fma_world );
	numObjectBanks = 0;
	fma_world = NULL;
}


void ExtractPsiNames ( void )
{
	char *addr;
	char *PSIname=0;

	int psiM;
	int *table;//,*crcs;
	int i;


	addr = (char*)pilLibraryList[1];// = (long*)addr;

	i = (int)*addr;
	(char*)table = addr+4;

	while ( i )
	{
		i--;
		psiM = *(table++);		psiM += (int)addr;

		PSIname = (char*)(psiM);
		//utilPrintf("Extracting PSI File Name : %s\n",PSIname);
		sprintf ( PSIFileList [ PSIFileListPos++ ], "%s",  PSIname );

		psiM += 36;
	}
	// ENDWHILE
}
