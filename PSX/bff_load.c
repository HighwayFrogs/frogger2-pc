
// comment out to link into the frogger testbed
// leave in to link into the ActionMan2 testbed
//#define USE_NEW_LIBS


// the "depend" batch doesn't like non-existant headers
//	#include "global.h"
#include <islfile.h>
#include <islutil.h>
#include "bff_load.h"
#include "flatpack.h"
/*
Quickie guide to BFF files:-

 A BFF is, simply, a collection of data segments, prefixed by a TYPE, a SIZE, and a name (or rather the CRC of a name)
Data of many different types can just be shoved together into one file.
After loading, the program scans through the list, checks the type of each segment, and calls a "link" routine
for that type, which basically fills in any required pointers.

 Then there's a little routine saying "find a bff segment of "such&such type, with this name-CRC"

*/


// A Macro and a variable that were used in Glover, but aren't in Frogger2
// (But here they are anyway)

//#define BFF_CorrectPointer(header,pointer) {if(pointer) (pointer) = ADD2POINTER((header),((int)(pointer)));}
//BFF_Header *generic_bff;


// We need access to a some ISLPSI internal variable
// (Latest version libs have these in)

int	biggestVertexModel;
//extern long *transformedVertices;
//extern VERT *transformedNormals;
//extern long *transformedDepths;



// Adjust 0...128 texture coordinates into real ones

ULONG FMA_TmapCorrect(ULONG x,ULONG w,ULONG per)
{
	ULONG calc;
	ULONG result;

	calc=(w*per)>>7; 
 	result=calc+x;

	if(result==256)	// Work around the silly playstation's lack of ability to access the top pixel of a texture
		result = 255;

 	if (result>255)
	{
//		printf("tmap correction OV: x=%d,w=%d,per=%d\n",x,w,per);
		result=8;
	}
 	return result;
}

// ===================================================================================
// Turn the offsets & spritex numbers in a FMA MESH BFF-segment data into pointers

void BFF_Link_FMA_Mesh(FMA_MESH_HEADER *mesh)
{
//	unsigned long *tmap_crcs;
	TextureType **tmaps;
	TextureType *tmap;

	int i;


	tmaps = ADD2POINTER(mesh, (int)mesh->tmap_crcs);

	mesh->gt3s = ADD2POINTER(mesh,(int)mesh->gt3s);
	mesh->gt4s = ADD2POINTER(mesh,(int)mesh->gt4s);
	mesh->sprs = ADD2POINTER(mesh,(int)mesh->sprs);
	mesh->verts = ADD2POINTER(mesh,(int)mesh->verts);

	if(mesh->n_verts > biggestVertexModel)
	{

// When using the new libs, you update "biggestVertexModel" during loading, and
// call a PSI.C function that allocates the memory after you've loaded all the shapes

		biggestVertexModel = mesh->n_verts;
	}


// Having done that, let's get down to the real work of converting the loaded memory block
// into a usable one with pointers and real-world texture info.


// Number one - We've got a list of texture CRC's to convert into pointers.
//	printf("Mesh has %d textures\n",mesh->n_tmaps);
	for(i = 0; i < mesh->n_tmaps; i++)
	{
		tmap = textureFindCRCInAllBanks((unsigned long)tmaps[i]);
		if(!tmap)
		{
			utilPrintf("World Texture (crc %lu) not found\n",(unsigned long)tmaps[i]);

			tmaps[i] = textureFindCRCInAllBanks(utilStr2CRC ("FROGWATCH01"));
		}
		else
		{
			tmaps[i] = tmap;
		}

	}

// Number two. Every polygon contains a texture number, which needs converting
// into a clut & a tpage. They also need their tu and tv adjusting.
	for(i = 0; i < mesh->n_gt3s; i++)
	{
		tmap = tmaps[mesh->gt3s[i].tpage];
		if(tmap)
		{
			mesh->gt3s[i].clut = tmap->clut;
			mesh->gt3s[i].tpage = tmap->tpage;

			mesh->gt3s[i].u0 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt3s[i].u0);
			mesh->gt3s[i].v0 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt3s[i].v0);
			mesh->gt3s[i].u1 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt3s[i].u1);
			mesh->gt3s[i].v1 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt3s[i].v1);
			mesh->gt3s[i].u2 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt3s[i].u2);
			mesh->gt3s[i].v2 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt3s[i].v2);
// Multiply the vertex numbers by 4.
// A nice little optimisation when looking the coordinates up
// (this code now moved to the PC compiler end)
//			mesh->gt3s[i].vert0 *= 4;
//			mesh->gt3s[i].vert1 *= 4;
//			mesh->gt3s[i].vert2 *= 4;
		}
	}

	for(i = 0; i < mesh->n_gt4s; i++)
	{
		tmap = tmaps[mesh->gt4s[i].tpage];
		if(tmap)
		{
			mesh->gt4s[i].clut = tmap->clut;
			mesh->gt4s[i].tpage = tmap->tpage;

			mesh->gt4s[i].u0 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt4s[i].u0);
			mesh->gt4s[i].v0 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt4s[i].v0);
			mesh->gt4s[i].u1 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt4s[i].u1);
			mesh->gt4s[i].v1 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt4s[i].v1);
			mesh->gt4s[i].u2 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt4s[i].u2);
			mesh->gt4s[i].v2 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt4s[i].v2);
			mesh->gt4s[i].u3 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->gt4s[i].u3);
			mesh->gt4s[i].v3 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->gt4s[i].v3);
//			mesh->gt4s[i].vert0 *= 4;
//			mesh->gt4s[i].vert1 *= 4;
//			mesh->gt4s[i].vert2 *= 4;
//			mesh->gt4s[i].vert3 *= 4;
		}
	}

	for(i = 0; i < mesh->n_sprs; i++)
	{
		tmap = tmaps [ mesh->sprs[i].tpage ];
		if ( tmap )
		{
			mesh->sprs[i].clut = tmap->clut;
			mesh->sprs[i].tpage = tmap->tpage;

			mesh->sprs[i].u0 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->sprs[i].u0);
			mesh->sprs[i].v0 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->sprs[i].v0);
			mesh->sprs[i].u1 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->sprs[i].u1);
			mesh->sprs[i].v1 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->sprs[i].v1);
			mesh->sprs[i].u2 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->sprs[i].u2);
			mesh->sprs[i].v2 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->sprs[i].v2);
			mesh->sprs[i].u3 = FMA_TmapCorrect(tmap->x,tmap->w,mesh->sprs[i].u3);
			mesh->sprs[i].v3 = FMA_TmapCorrect(tmap->y,tmap->h,mesh->sprs[i].v3);
//			mesh->gt4s[i].vert0 *= 4;
//			mesh->gt4s[i].vert1 *= 4;
//			mesh->gt4s[i].vert2 *= 4;
//			mesh->gt4s[i].vert3 *= 4;
		}
	}

// Tra-la. The BFF memory-image is now a usable, drawable, thing.

}

// Turn the FMA CRCs in a FMA MAP BFF-segment data into pointers
// This is an easy one. The world just has a list of mesh-name CRC's, which
// need converting into pointers. This is done by BFF_Finding them, even
// though they're actually part of the same file as the FMA_WORLD!
void BFF_Link_FMA_World(FMA_WORLD *world)
{
	int i;
	unsigned long *crcs;
	FMA_MESH_HEADER *mesh;

#define meshes ((FMA_MESH_HEADER **)crcs)

	meshes = ADD2POINTER(world, sizeof(FMA_WORLD));
	for(i = 0; i < world->n_meshes;i++)
	{
		mesh = (void *)BFF_FindObject(BFF_FMA_MESH_ID,crcs[i]);
		if(!mesh)
		{
			//utilPrintf("Mesh %d of the world not found (crc = &%8x)\n",i,crcs[i]);
		}
		meshes[i] = mesh;
	}
#undef meshes
}










// ===================================================================================
//                GENERAL   BFF   HANDLING   STUFF
// ===================================================================================

#define MAX_BFF_FILES 4
BFF_Header *BFF_Pointers[MAX_BFF_FILES] = {NULL,NULL,NULL,NULL};
int BFF_Lens[MAX_BFF_FILES];

BFF_Header *BFF_FindObject(int id, unsigned long crc)
{
	int i;
	BFF_Header *work;
	for(i = 0; i < MAX_BFF_FILES; i++)
	{
		if(BFF_Pointers[i])
		{
//			printf(" scanning bff %d",i);
			work = BFF_Pointers[i];
			for(;;)
			{
				if(work->id == id && (crc == 0 || work->crc == crc))
				{
//					printf("\n");
					return work;
				}
				if(work->len & 0x80000000)
					break;
				work = ADD2POINTER(work,work->len);
			}
//			printf("\n");
		}
	}
	return NULL;	// yes there are occasions when we need to try to find things that aren't there
}


BFF_Header *BFF_LoadFile(char *filename)
{
	BFF_Header *addr;
	BFF_Header *work;
	BFF_Header *ending;
	int i;
	int lastfilelength;

// Oh lovely, Frogger's makefile didn't warn me about duff numbers of parameters being sent to functions...
// Someone's not got the warning level set high enough!


	addr = (BFF_Header*)FindStakFileInAllBanks ( filename, &lastfilelength );

	//addr = (BFF_Header*)getFileFromStack ( stakFile, filename, &lastfilelength );

//	addr = (void *)fileLoad(filename, &lastfilelength);


	if(!addr)
	{
		//utilPrintf("Could Not Load Bff File.\n");
		return NULL;
	}

	ending = ADD2POINTER(addr, lastfilelength);

	work = addr;

// Using the file length, scan through the file and mark the last entry in the BFF file,
// because the "findobject" routines won't know the length.
	for(;;)
	{

/*				utilPrintf("Scanning BFF class %d (%c%c%c%d), len &%8x\n",
					work->id,
					((char *)(&work->id))[0],
					((char *)(&work->id))[1],
					((char *)(&work->id))[2],
					(int)(((char *)(&work->id))[3]),
					work->len
					);*/



		if((void *)ADD2POINTER(work,work->len) >= (void *)ending)
		{
			work->len |= 0x80000000;	// mark as the last one
			break;
		}
		work = ADD2POINTER(work,work->len);
	}

// Mark the BFF as being available for searching for objects.
// This needs to be done BEFORE linking, because some objects want to look up
// the addresses of other objects in the same file.
	for(i = 0; i < MAX_BFF_FILES; i++)
	{
		if(!BFF_Pointers[i])
		{
			BFF_Pointers[i] = addr;
			BFF_Lens[i] = lastfilelength;
			break;
		}
	}

// Essentially, convert any "offsets" in the loaded objects into "pointers", and
// any other work that needs doing to convert the loaded data into usable data.
// This means checking the type of the data, and calling a routine to "link"
// that type.
	work = addr;
	for(;;)
	{

/*				utilPrintf("Linking BFF class %d (%c%c%c%d)\n",
					work->id,
					((char *)(&work->id))[0],
					((char *)(&work->id))[1],
					((char *)(&work->id))[2],
					(int)(((char *)(&work->id))[3])
					);
*/

		switch(work->id)
		{
			case BFF_FMA_MESH_ID:
				BFF_Link_FMA_Mesh((FMA_MESH_HEADER *)work);
				break;
			case BFF_FMA_WORLD_ID:
				BFF_Link_FMA_World((FMA_WORLD *)work);
				break;

			default:
/*				utilPrintf("Unknown or out-of-date BFF class %d (%c%c%c%d)\n",
					work->id,
					((char *)(&work->id))[0],
					((char *)(&work->id))[1],
					((char *)(&work->id))[2],
					(int)(((char *)(&work->id))[3])
					);*/
				CRASH;
		}

		if(work->len & 0x80000000)
			break;
		work = ADD2POINTER(work,work->len);
//		DB("ok\n");
	}
//	DB("linking all done\n");

//	outputdebugmessage
	return addr;
}

// Releasing a file's dead trivial. Just free the memory, and mark it as
// not to be scanned in future "findobject"s
// As if by magic, all the contents are freed.
// (Unless the "link" routines do any mallocs - in which case there'll need to be
// a full BFF-scan here)

void BFF_ReleaseFile(BFF_Header *header)
{
	int i;
	if(!header)
		return;
	for(i = 0; i < MAX_BFF_FILES; i++)
	{
		if(BFF_Pointers[i] == header)
		{
			BFF_Pointers[i] = NULL;
			break;
		}
	}
	FREE(header);
}

// A little diagnostic check that you can use to ensure that you've not
// kept a pointer to a BFF object in a file that has since been discarded.
// In practice, this should never be used, natch.

int BFF_IsInBFF(void *addr)
{
	void *base;
	int i;

	for(i = 0; i < MAX_BFF_FILES; i++)
	{
		base = BFF_Pointers[i];
		if(base)
		{
			if(base <= addr && ADD2POINTER(base,BFF_Lens[i]) > addr)
				return 1;
			break;
		}
	}
	return 0;
}


BFF_Header *BFF_FindNamedObject(int id, char *name)
{
	int i;
	unsigned long crc;

	BFF_Header *work;

	crc = utilStr2CRC(name);
	for(i = 0; i < MAX_BFF_FILES; i++)
	{
		if(BFF_Pointers[i])
		{
//			printf(" scanning bff %d",i);
			work = BFF_Pointers[i];
			for(;;)
			{

//				printf(".");
				if(work->id == id && (crc == 0 || work->crc == crc))
				{
//					printf("\n");
					return work;
				}
				if(work->len & 0x80000000)
					break;
				work = ADD2POINTER(work,work->len);
			}
//			printf("\n");
		}
	}
//	printf("null\n");
	return 0;	// yes there are occasions when we need to try to find things that aren't there
}
