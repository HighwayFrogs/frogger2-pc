#include "map_draw.h"
#include "world_eff.h"
#include <islpad.h>
#include <isltex.h>
#include <islcard.h>
#include "main.h"
#include "layout.h"
#include "frogger.h"
#include "savegame.h"
#include "water.h"
#include "actor2.h"
#include "temp_psx.h"
#include "cam.h"
#include "textures.h"
#include "map_draw.h"
#include "timer.h"
#include "memcard.h"
#include "islvideo.h"
#include "story.h"
#include "audio.h"
#include "menus.h"
#include "options.h"

SCENICOBJ *lightBeam;
FMA_GT4 *lightBeamGouraudValues = NULL;

FVECTOR fmaActorScale;

void GetObjectGouraudValues ( FMA_MESH_HEADER *mesh, FMA_GT4 *gouraudList )
{
	int i;

	FMA_GT4 *gList;

	register char *opcd asm("$18");

#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;

	gList = gouraudList;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		*gouraudList = *op;
		gouraudList++;
	}
#undef op
}

void SetObjectGouraudValues ( FMA_MESH_HEADER *mesh, FMA_GT4 *gouraudList )
{
	int i;

	FMA_GT4 *gList;

	register char *opcd asm("$18");

#define op ((FMA_GT4 *)opcd)

	op = mesh->gt4s;

	gList = gouraudList;

	for ( i = mesh->n_gt4s; i != 0; i--, op++ )
	{
		*op = *gouraudList;
		gouraudList++;
	}
#undef op
}


void CreateLevelObjects(unsigned long worldID,unsigned long levelID)
{
	SCENICOBJ *obj;
	short i;
	char tmp[5];
	int flags;

	ACTOR2 *theActor;
	SCENIC *cur = firstScenic;


//	char *compare;

	/*int actCount = 0;
	*/
	// go through and add items
	while (cur)
	{
		utilPrintf("%s, %d, %d, %d\n", cur->name, cur->pos.vx, cur->pos.vy, cur->pos.vz);
		if ( (gstrcmp ( cur->name,"world.psi") != 0) /*&& (gstrcmp ( cur->name,"backdrop.psi") != 0)*/ )
		{
			for( i=0; i<4; i++ )
				tmp[i] = cur->name[i];
			tmp[4] = '\0';

			if ( ( gstrcmp ( tmp,"cam_\0") != 0 ) )
			{
				//if ( ( gstrcmp ( tmp,"wat_\0") ==0 ) )
				//{
				//	flags = 0;

					//if ( cur->name[4] == 'f' )
						//flags = ACTOR_SLIDYTEX;
					// ENDIF

//					CreateAndAddScenicObject(cur);
				//	utilUpperStr ( cur->name );
				//	CreateAndAddWaterObject ( cur );
			//		CreateAndAddScenicObject(cur);

				//}
				/*else*/ if ( ( strstr ( cur->name, "xx_" ) ) || ( strstr ( cur->name, "lea_" ) ) ||
									( strstr ( cur->name, "xxa_" ) ) || ( strstr ( cur->name, "spl_" ) ) ||
									( strstr ( cur->name, "wh" ) ) || ( strstr ( cur->name, "wat_" ) )||
									( strstr ( cur->name, "slu_" ) ) || ( strstr ( cur->name, "spm_" ) )||
									( strstr ( cur->name, "log" ) )|| ( strstr ( cur->name, "cfence" ) )||
									( strstr ( cur->name, "bar" ) )|| ( strstr ( cur->name, "submush" ) )||
									( strstr ( cur->name, "pylon" ) )|| ( strstr ( cur->name, "spf_" ) )||
									( strstr ( cur->name, "gunk" ) )|| ( strstr ( cur->name, "mini" ) )||
									( strstr ( cur->name, "plat" ) )|| ( strstr ( cur->name, "poper" ) )||
									( strstr ( cur->name, "powerl" ) )|| ( strstr ( cur->name, "lamp" ) )||
									( strstr ( cur->name, "tubtank" ) )|| ( strstr ( cur->name, "cityrub" ) )||
									( strstr ( cur->name, "candle" ) )|| ( strstr ( cur->name, "torch" ) )||
									( strstr ( cur->name, "framea" ) )|| ( strstr ( cur->name, "frameb" ) )||
									( strstr ( cur->name, "framec" ) )|| ( strstr ( cur->name, "framed" ) )||
									( strstr ( cur->name, "levels" ) )|| ( strstr ( cur->name, "players" ) )||
									( strstr ( cur->name, "babyfrog" ) )|| ( strstr ( cur->name, "froggr" ) )||
									( strstr ( cur->name, "hopper" ) )|| ( strstr ( cur->name, "lillie" ) )||
									( strstr ( cur->name, "wart" ) )|| ( strstr ( cur->name, "twee" ) )||
									( strstr ( cur->name, "swampy" ) )|| ( strstr ( cur->name, "robofrog" ) )||
									( strstr ( cur->name, "glass" ) )|| ( strstr ( cur->name, "robofrog" ) )||
									( strstr ( cur->name, "ancnt2" ) )|| ( strstr ( cur->name, "vs" ) )||
									( strstr ( cur->name, "wat_" ) )|| ( strstr ( cur->name, "rails" ) )||
									( strstr ( cur->name, "window" ) )|| ( strstr ( cur->name, "anspike" ) )  ) 
				{
					int thisistheone = 0;

					if ( strstr ( cur->name, "wat_flbm" ) )
						thisistheone = 1;
					else
						thisistheone = 0;
					// ENDIF

					utilUpperStr ( cur->name );

					obj = CreateAndAddScenicObject(cur);

					if ( thisistheone )
					{
						FMA_MESH_HEADER **mesh;

						lightBeam = obj;

						if ( lightBeam )
						{
							mesh = ADD2POINTER(lightBeam->fmaObj,sizeof(FMA_WORLD));

							lightBeamGouraudValues = MALLOC0(sizeof(FMA_GT4) * (*mesh)->n_gt4s );

							GetObjectGouraudValues ( *mesh, lightBeamGouraudValues );
						}
						// ENDIF
					}
					// ENDIF
				}
				else
				{
					utilUpperStr ( cur->name );
					theActor = CreateAndAddActor ( cur->name, cur->pos.vx, cur->pos.vy, cur->pos.vz, INIT_ANIMATION );

					if(theActor)
					{
						QuatToPSXMatrix ( &cur->rot, &theActor->actor->bffMatrix );

						theActor->actor->qRot = cur->rot;

						actorAnimate ( theActor->actor, 0, YES, NO, 150, NO );
					}
				}
			}
		}

		/*fixed tv;

		stringChange(ts->name);

		if(gstrcmp(ts->name,"backdrop.obe") == 0)
		{
			backGnd = CreateAndAddActor(ts->name, ToFixed(ts->pos.vx), ToFixed(ts->pos.vz), ToFixed(ts->pos.vy), INIT_ANIMATION, ToFixed(0), 0);
			backGnd->actor->scale.vx = ToFixed(5);
			backGnd->actor->scale.vy = ToFixed(5);
			backGnd->actor->scale.vz = ToFixed(5);
			actList = actList->next;
			if (actList)
				actList->prev = NULL;
		}
		else
		{*/



			/*if(gstrcmp(ts->name,"world.obe") == 0)
			{
				theActor->flags = ACTOR_DRAW_ALWAYS;
				globalLevelActor = theActor;
			}

			for( i=0; i<4; i++ )
				tmp[i] = ts->name[i];
			tmp[4] = '\0';
			// If a water object, draw always
			if( !gstrcmp( tmp, "wat_\0" ) )
			{
				theActor->flags = ACTOR_WATER | ACTOR_DRAW_ALWAYS;

				if (ts->name[4]=='f')
					theActor->flags |= ACTOR_SLIDYTEX;

#ifdef N64_VERSION
				AddN64WaterObjectResource(theActor->actor);
#endif
			}

			tv = ToFixed(ts->rot.y);
			ts->rot.y = ts->rot.z;
//bb
//			ts->rot.z = tv/4096;
			ts->rot.z = tv/4096.0F;

			GetQuaternionFromRotation (&theActor->actor->qRot,&ts->rot);

			actorAnimate(theActor->actor,0,YES,NO,1433/16,0);
			if(ts->name[0] == 'a')
			{
				fixed rMin,rMax,rNum;
				if(ts->name[2] == '_')
				{
					rMin = ToFixed(ts->name[1] - 30);
					if(rMin == ToFixed(0)) 
						rMin = ToFixed(10);
					rMin /= 10;
					actorAnimate(theActor->actor,0,YES,NO,rMin>>4, 0);
				}
				else if(ts->name[3] == '_')
				{
					rMin = ToFixed(ts->name[1] - 30);
					if(rMin == ToFixed(0)) 
						rMin = ToFixed(10);
					rMin /= 10;
				
					rMax = ToFixed(ts->name[1] - 30);
					if (rMax == ToFixed(0)) 
						rMax = ToFixed(10);
					rMax /= 10;

					rNum = ToFixed(Random(1000));
					rNum= rMin + FDiv( FMul(rNum,(rMax-rMin)), ToFixed(1000) );
										
					actorAnimate(theActor->actor,0,YES,NO,rNum>>4, 0);
				}
			}
			actCount++;
		}
*/
		cur = cur->next;

	}
	// ENDWHILE

//	dprintf"\n\n** ADDED %d ACTORS **\n\n",actCount));
}


/*

extern unsigned short	currentPal16[16];
extern unsigned short	*VRAMpalBlock;
extern unsigned short	*VRAMpalCLUT;
extern unsigned short	currentPal256[256];
extern unsigned short	*VRAMpal256Block;
extern unsigned long	*VRAMpal256CLUT;
int VRAM_PALETTES = 500;
int VRAM_256PALETTES = 40;


*/




/*void froggerShowVRAM(unsigned char palMode)
{
	DISPENV		dispenv;
	int			xOffs,yOffs, viewMode, currTex;

	currTex = 0;
	xOffs = 512;
	yOffs = 0;
	viewMode = 0;
	SetDefDispEnv(&dispenv, 0,0, 512,256);
	while ((padData.digital[0] & PAD_START)==0)
	{
		myPadHandleInput();
		switch(viewMode)
		{
		case 0:
			froggerVRAMviewNormal(&dispenv, &xOffs,&yOffs, palMode);
			break;
		case 1:
			froggerVRAMviewTextures(&currTex);
			break;
		}
		if (padData.debounce[0] & PAD_SELECT)
			viewMode = (viewMode+1) % 2;
	}
}*/


/*static void froggerVRAMviewNormal(DISPENV *dispenv, int *xOffs,int *yOffs, unsigned char palMode)
{
	if (padData.digital[0] & PAD_LEFT)
		*xOffs -= 6;
	if (padData.digital[0] & PAD_RIGHT)
		*xOffs += 6;
	if (*xOffs<0)
		*xOffs = 0;
	if (*xOffs>1024-512)
		*xOffs = 1024-512;
	if (padData.digital[0] & PAD_UP)
		*yOffs -= 4;
	if (padData.digital[0] & PAD_DOWN)
		*yOffs += 4;
	if (*yOffs<0)
		*yOffs = 0;
	if (*yOffs>512-((palMode)?(256):(240)))
		*yOffs = 512-((palMode)?(256):(240));
	dispenv->disp.x = *xOffs;
	dispenv->disp.y = *yOffs;
	if(palMode)
		dispenv->screen.y = 24;

	VSync(0);
	PutDispEnv(dispenv);
}*/

/*static void froggerVRAMviewTextures(int *currTex)
{
	POLY_FT4 	*ft4;
	TextureType	*tex;
	char		str[40];
	int			loop, yu = 0, yd = 0, f;

	static int	padDelay = 0;

	currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
	ClearOTagR(currentDisplayPage->ot, 1024);
	currentDisplayPage->primPtr = currentDisplayPage->primBuffer;

	sprintf(str, "VRAM: TEXTURE VIEW");
	fontPrintScaled(font, -230,-110, str, 128,128,128,4096);

	tex = textureFindTextureN(*currTex);
	if (tex!=NULL)
	{
		BEGINPRIM(ft4, POLY_FT4);
		setPolyFT4(ft4);
		setXYWH(ft4, 150-tex->w/2,-tex->h/2, tex->w,tex->h-1);
		setRGB0(ft4, 128,128,128);
		setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
		ft4->tpage = tex->tpage;
		ft4->clut = tex->clut;
		ENDPRIM(ft4, 0, POLY_FT4);

		BEGINPRIM(ft4, POLY_FT4);
		setPolyFT4(ft4);
		setXYWH(ft4, -128-tex->w,-tex->h, tex->w*3,tex->h*2);
		setRGB0(ft4, 128,128,128);
		setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
		ft4->tpage = tex->tpage;
		ft4->clut = tex->clut;
		ENDPRIM(ft4, 1, POLY_FT4);

		if(tex->tpage & (1 << 7))
			froggerVRAMdrawPalette256(tex->clut, -90);
		else
			froggerVRAMdrawPalette(tex->clut, -90);

		sprintf(str, "Texture #%d (%dx%d) (used %dx)", *currTex, tex->w,tex->h, tex->refCount);
		fontPrintScaled(font, -fontExtentWScaled(font, str,4096)/2,75, str, 128,128,128,4096);
		yu = -tex->h/2-8;
		yd = tex->h/2+8;
	}

	for(loop=1; loop<5; loop++)
	{
		tex = textureFindTextureN(*currTex-loop);
		f = 100-loop*20;
		if (tex!=NULL)
		{
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			setXYWH(ft4, 150-tex->w/2,yu-tex->h, tex->w,tex->h-1);
			setRGB0(ft4, f,f,f);
			setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
			ft4->tpage = tex->tpage;
			ft4->clut = tex->clut;
			ENDPRIM(ft4, 2, POLY_FT4);
			yu -= tex->h+8;
		}
		tex = textureFindTextureN(*currTex+loop);
		if (tex!=NULL)
		{
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			setXYWH(ft4, 150-tex->w/2,yd, tex->w,tex->h-1);
			setRGB0(ft4, f,f,f);
			setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
			ft4->tpage = tex->tpage;
			ft4->clut = tex->clut;
			ENDPRIM(ft4, 2, POLY_FT4);
			yd += tex->h+8;
		}
	}

	if (padDelay==0)
	{
		if ((padData.digital[0] & (PAD_LEFT|PAD_UP)) && (*currTex>0))
		{
			*currTex = (*currTex)-1;
			padDelay = 3;
			if (padData.debounce[0] & (PAD_LEFT|PAD_UP))
				padDelay = 20;
		}
		if (padData.digital[0] & (PAD_RIGHT|PAD_DOWN))
		{
			if(textureFindTextureN(*currTex+1))
			{
				*currTex = (*currTex)+1;
				padDelay = 3;
				if (padData.debounce[0] & (PAD_RIGHT|PAD_DOWN))
					padDelay = 20;
			}
		}
	}
	else
	{
		if ((padData.digital[0] & (PAD_LEFT|PAD_RIGHT|PAD_UP|PAD_DOWN))==0)
			padDelay = 0;
		if (padDelay>0)
			padDelay--;
	}

	DrawSync(0);
	VSync(0);
	PutDispEnv(&currentDisplayPage->dispenv);
	PutDrawEnv(&currentDisplayPage->drawenv);
	DrawOTag(currentDisplayPage->ot+(1024-1));
}*/

/*static void froggerVRAMdrawPalette(unsigned long clut, int y)
{
	POLY_F4		*f4;
	char		str[40];
	int			pal, loop;
	RECT		rect;

	rect.x = (clut & 0x3f) << 4;
	rect.y = (clut >> 6);
	rect.w = 16;
	rect.h = 1;

	DrawSync(0);
	StoreImage(&rect, currentPal16);
   	
	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	setXYWH(f4, -230-2,y-1, 16*20+3,10);
	setRGB0(f4, 128,128,128);
	ENDPRIM(f4, 1, POLY_F4);
	for(loop=0; loop<16; loop++)
	{
		BEGINPRIM(f4, POLY_F4);
		setPolyF4(f4);
		setXYWH(f4, -230+loop*20,y, 20,8);
		setRGB0(f4, 8*((currentPal16[loop]>>0) & 0x1f),
					8*((currentPal16[loop]>>5) & 0x1f),
					8*((currentPal16[loop]>>10) & 0x1f));
   		ENDPRIM(f4, 0, POLY_F4);
   	}

	for(pal=0; pal<VRAM_PALETTES; pal++)
   	{
   		if ((VRAMpalBlock[pal]) && (clut==VRAMpalCLUT[pal]))
   		{
			sprintf(str, "Palette #%d (used %dx)", pal, VRAMpalBlock[pal]);
   			fontPrintScaled(font, -230,y+13, str, 128,128,128,4096);
			break;
		}
	}

}*/

/*static void froggerVRAMdrawPalette256(unsigned long clut, int y)
{
	POLY_F4			*f4;
	char			str[40];
	int				pal, loop, loop2;
	RECT			rect;
	unsigned short	*palPtr;

	rect.x = (clut & 0x3f) << 4;
	rect.y = (clut >> 6);
	rect.w = 256;
	rect.h = 1;

	DrawSync(0);
	StoreImage(&rect, currentPal256);
   	
	BEGINPRIM(f4, POLY_F4);
	setPolyF4(f4);
	setXYWH(f4, -230-2,y-1, 16*20+3,10);
	setRGB0(f4, 128,128,128);
	ENDPRIM(f4, 1, POLY_F4);

	palPtr = &currentPal256[0];

	for(loop = 0; loop < 4; loop ++)
	{
		for(loop2 = 0; loop2 < 64; loop2 ++)
		{
			BEGINPRIM(f4, POLY_F4);
			setPolyF4(f4);
			setXYWH(f4, -230+loop2*5,y, 5, 2);
			setRGB0(f4, 8*(((*palPtr)>>0) & 0x1f),
						8*(((*palPtr)>>5) & 0x1f),
						8*(((*palPtr)>>10) & 0x1f));
   			ENDPRIM(f4, 0, POLY_F4);
			palPtr ++;
   		}
		y += 2;
	}

	for(pal=0; pal<VRAM_256PALETTES; pal++)
   	{
   		if ((VRAMpal256Block[pal]) && (clut==VRAMpal256CLUT[pal]))
   		{
			sprintf(str, "256Palette #%d (used %dx)", pal, VRAMpal256Block[pal]);
   			fontPrintScaled(font, -230,y+5, str, 128,128,128,4096);
			break;
		}
	}
}*/


int cursPos = 0;

void PsxNameEntryInit(void)
{
	int j;

	for(j = 0;j < NAME_LENGTH;j++)
		if(textString[j] == 0)
			textString[j] = '-';

	if(textString[0] == '-')
		textString[0] = 'A';

	textString[NAME_LENGTH] = 0;
}

void PsxNameEntryFrame(void)
{
	int j;
//	myPadHandleInput();

	//move cursor
	if(padData.debounce[0] & PAD_LEFT)
	{
		if(cursPos > 0)
		{
			textString[cursPos] = '-';
			cursPos--;
			PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
		}
	}

	if(padData.debounce[0] & (PAD_RIGHT | PAD_CROSS))
	{
		if((cursPos < NAME_LENGTH - 1) || (padData.debounce[0] & PAD_CROSS))
		{
			if(cursPos < NAME_LENGTH)
			{
				cursPos++;

				textString[cursPos]='A';
				if(cursPos == NAME_LENGTH - 1)
				{
					textString[cursPos + 1] = 0;
				}
				else
					textString[cursPos + 1] = '-';
				if(cursPos == NAME_LENGTH)
				{
					for(j = 0;j < NAME_LENGTH;j++)
						if(textString[j] == '-')
							textString[j] = 0;
					textString[NAME_LENGTH] = 0;
					textEntry = 0;
					cursPos--;
					PlaySample(genSfx[GEN_COLLECT_BABY],NULL,0,SAMPLE_VOLUME,-1);
				}
				else
					PlaySample(genSfx[GEN_SUPER_HOP],NULL,0,SAMPLE_VOLUME,-1);
			}
		}
	}


	//change char under cursor
	if(padData.debounce[0] & PAD_UP)
	{
		PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
		if((textString[cursPos]==' ') || (textString[cursPos] == '-'))
			textString[cursPos]='A';
		else if(textString[cursPos] == 'Z')
			textString[cursPos]=' ';
		else if(textString[cursPos]<'Z')
			textString[cursPos]++;
	}

	if(padData.debounce[0] & PAD_DOWN)
	{
		PlaySample(genSfx[GEN_FROG_HOP],NULL,0,SAMPLE_VOLUME,-1);
		if(textString[cursPos]=='A')
			textString[cursPos]=' ';
		else if((textString[cursPos]==' ') || (textString[cursPos] == '-'))
			textString[cursPos]='Z';
		else if(textString[cursPos]>'A')
			textString[cursPos]--;
	}

	//done?
	if(padData.debounce[0]&PAD_START)
	{
		for(j = 0;j < NAME_LENGTH;j++)
			if(textString[j] == '-')
				textString[j] = 0;
		textString[NAME_LENGTH] = 0;
		textEntry=0;
	}
}






#define ____DRAW_SCREEN_CLIP

//sets act->clipped, and prepares for drawing
void PsiActor2ClipCheck(ACTOR2* act)
{
//bbxx - PAL/NTSC specifics needed here
	#define CLIP_TOP	-120
	#define CLIP_RIGHT	256
	#define CLIP_BOTT	120
	#define CLIP_LEFT	-256

	long sxy,sz;
	long sx, sy;
	int distTop, distRight, distBott, distLeft, distIn, distOut;
	int radius, FOV;

	SVECTOR pos = act->actor->position;
	pos.vx = -pos.vx;
	pos.vy = -pos.vy;

	if(act->flags & ACTOR_DRAW_ALWAYS)
	{
		//TIMER_START1(TIMER_UPANI);
		actorUpdateAnimations(act->actor);
		//TIMER_STOP_ADD1(TIMER_UPANI);

		if ( act->actor->psiData.flags & ACTOR_DYNAMICSORT )
		{
			if ( act->actor->animation.blendSpeed )
			{
				actorSetAnimation2 ( act->actor,	act->actor->animation.blendStartFrame,
																					act->actor->animation.blendEndFrame, act->actor->animation.blendFrame );
			}
			else
				actorSetAnimation ( act->actor, act->actor->animation.frame, 1 );
		}
		else
			actorSetAnimation ( act->actor, act->actor->animation.frame, 1 );
		

		QuatToPSXMatrix(&act->actor->qRot, &act->actor->psiData.object->matrix);
		act->actor->psiData.object->matrix.t[0] = -act->actor->position.vx;
		act->actor->psiData.object->matrix.t[1] = -act->actor->position.vy;
		act->actor->psiData.object->matrix.t[2] =  act->actor->position.vz;

		act->clipped = 0;
		return;
	}

	//calc screen coords of actor
	//bbxx n.b only needed for psi actors.
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);

	gte_ldv0(&pos);
	gte_rtps();
	gte_stsxy(&sxy);
	gte_stsz(&sz);


	//not too close/far
	if( sz <= 0 )
	{
		act->clipped = 1;
		return;
	}

	//psi clipping.
	//calculate the screen position of the object,
	//and it's screen radius.
//	else 
//	{
		sx = (short)(sxy&0xffff);
		sy = (short)(sxy>>16);

		//bbxx
		//this needs to be changed, so that we use
		//the (as yet unwritten) true centre and true radius.
		//(which we need to calc in psiLoad)

		//now with radius check
		gte_ReadGeomScreen(&FOV);
		radius = (act->actor->radius *FOV) /sz; //n.b *4 'cos we got z/4, not z
		distTop		= (sy+radius) - CLIP_TOP;
		distRight	= (sx-radius) - CLIP_RIGHT;
		distBott	= (sy-radius) - CLIP_BOTT;
		distLeft	= (sx+radius) - CLIP_LEFT;
		distIn		= sz+radius;
		distOut		= sz-radius;

		//clip?
		if( (distTop<0) || (distRight>0) || (distBott>0) || (distLeft<0) || (distOut>fog.max) )
		{
			act->clipped = 1;
		}
		else
		{
			//TIMER_START1(TIMER_UPANI);
			actorUpdateAnimations(act->actor);
			//TIMER_STOP_ADD1(TIMER_UPANI);

			//TIMER_START1(TIMER_SETANI);
			actorSetAnimation ( act->actor, act->actor->animation.frame, 1 );
			//TIMER_STOP_ADD1(TIMER_SETANI);

			QuatToPSXMatrix(&act->actor->qRot, &act->actor->psiData.object->matrix);
			act->actor->psiData.object->matrix.t[0] = -act->actor->position.vx;
			act->actor->psiData.object->matrix.t[1] = -act->actor->position.vy;
			act->actor->psiData.object->matrix.t[2] =  act->actor->position.vz;
			act->clipped = 0;
		}
//	}

	//miles away?
// 	if(act->clipped)
// 	{
// 		int debugTemp = 0;
// 		int milesDist = 100;
// 
// 		if( (distTop<-milesDist) || (distRight>milesDist)
// 		 || (distBott>milesDist) || (distLeft<-milesDist) )
// 		{
// 			debugTemp++;
// 		}
// 	}


	//debug - draw clipping rectangle
#ifdef DRAW_SCREEN_CLIP
	//is this a psi actor?
	if(!act->clipped)
	{
		int x0,x1;
		int y0,y1;
		TextureType *tempTex;

		x0 = sx - radius;
		x1 = sx + radius;
		y0 = sy - (radius>>1);
		y1 = sy + (radius>>1);


		tempTex = FindTexture("NEBU");
		if(!tempTex)
		{
			POLY_F4 *f4;

			//draw poly
			BEGINPRIM(f4, POLY_F4);
			setPolyF4(f4);
			f4->x0 = x0;
			f4->y0 = y0;
			f4->x1 = x1;
			f4->y1 = y0;
			f4->x2 = x0;
			f4->y2 = y1;
			f4->x3 = x1;
			f4->y3 = y1;
			f4->r0 = 128;
			f4->g0 = 0;
			f4->b0 = 0;
			f4->code |= 2;//semi-trans on
	//		SetSemiTrans(f4,1);
			ENDPRIM(f4, 1, POLY_F4);
		}
		else
		{
			POLY_FT4 *ft4;

			//draw poly
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			ft4->x0 = x0;
			ft4->y0 = y0;
			ft4->x1 = x1;
			ft4->y1 = y0;
			ft4->x2 = x0;
			ft4->y2 = y1;
			ft4->x3 = x1;
			ft4->y3 = y1;
			ft4->r0 = 16;
			ft4->g0 = 16;
			ft4->b0 = 16;
			ft4->u0 = tempTex->u0;
			ft4->v0 = tempTex->v0;
			ft4->u1 = tempTex->u1;
			ft4->v1 = tempTex->v1;
			ft4->u2 = tempTex->u2;
			ft4->v2 = tempTex->v2;
			ft4->u3 = tempTex->u3;
			ft4->v3 = tempTex->v3;
			ft4->tpage = tempTex->tpage;
			ft4->clut  = tempTex->clut;
			ft4->code  |= 2;//semi-trans on
		 	ft4->tpage |= 32;//add
	// 		ft4->tpage = si->tpage | 64;//sub
			ENDPRIM(ft4, 1, POLY_FT4);
		}

	}//end if(!act->clipped)

#endif //DRAW_SCREEN_CLIP

}

//sets act->clipped, and prepares for drawing
void FmaActor2ClipCheck(ACTOR2* act)
{
//bbxx - PAL/NTSC specifics needed here
	#define CLIP_TOP	-120
	#define CLIP_RIGHT	256
	#define CLIP_BOTT	120
	#define CLIP_LEFT	-256

	long sz;
	char* oldStackPointer;

	SVECTOR pos = act->actor->position;
	pos.vx = -pos.vx;
	pos.vy = -pos.vy;

	if(act->flags & ACTOR_DRAW_ALWAYS)
	{
		FMA_MESH_HEADER **mesh = ((char*)act->bffActor) + sizeof(FMA_WORLD);
		MATRIX tx, rY;

		//need to do this to transform points
//#if GOLDCD==1
//		oldStackPointer = SetSp(0x1f800400);
//#endif
		QuatToPSXMatrix(&act->actor->qRot, &act->actor->bffMatrix);
//#if GOLDCD==1
//		SetSp(oldStackPointer);
//#endif

		act->actor->bffMatrix.t[0] = -act->actor->position.vx;
		act->actor->bffMatrix.t[1] = act->actor->position.vy;
		act->actor->bffMatrix.t[2] = act->actor->position.vz;

		(*mesh)->posx = -act->actor->position.vx;
		(*mesh)->posy = -act->actor->position.vy;
		(*mesh)->posz = act->actor->position.vz;

		//calculate local to screen coords for fma mesh.
		//(camera matrix and objects' pos/rot matrix)

		gte_SetRotMatrix(&GsWSMATRIX);
		gte_SetTransMatrix(&GsWSMATRIX);

		// Unnecessary maths for landscape segments, where pos is always zero.
		gte_ldlvl( &(*mesh)->posx);
		gte_rtirtr();
		gte_stlvl(&tx.t);

/*			
		gte_MulMatrix0(&GsWSMATRIX, &act->actor->bffMatrix, &tx);
		rY.m[0][0] = rY.m[1][1] = rY.m[2][2] = act->actor->size.vx;
		rY.m[0][1] = rY.m[0][2] = rY.m[1][0] = rY.m[1][2] = rY.m[2][0] = rY.m[2][1] = 0;
		RotMatrixY(2048, &rY);
		gte_MulMatrix0(&tx, &rY, &tx);
		gte_SetRotMatrix(&tx);
		gte_SetTransMatrix(&tx);
*/

		gte_MulMatrix0(&GsWSMATRIX, &act->actor->bffMatrix, &tx);
		rY.m[0][1] = rY.m[0][2] = rY.m[1][0] = rY.m[1][2] = rY.m[2][0] = rY.m[2][1] = 0;
		rY.m[0][0] = -act->actor->size.vx;
		rY.m[1][1] =  act->actor->size.vy;
		rY.m[2][2] = -act->actor->size.vz;
		gte_MulMatrix0(&tx, &rY, &tx);
		gte_SetRotMatrix(&tx);
		gte_SetTransMatrix(&tx);

		act->clipped = 0;
		return;
	}

	//calc screen coords of actor
	//bbxx n.b only needed for psi actors.
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_SetRotMatrix(&GsWSMATRIX);

	gte_ldv0(&pos);
	gte_rtps();
//	gte_stsxy(&sxy);
	gte_stsz(&sz);



	//not too close/far
//	if( sz <= 0 || sz >= fog.max )
	if( sz <= 10 || sz >= fog.max )
	{
		act->clipped = 1;
	}
	else 
	{
		FMA_MESH_HEADER **mesh = ((char*)act->bffActor) + sizeof(FMA_WORLD);
		MATRIX tx, rY;

		//need to do this to transform points
//#if GOLDCD==1
//		oldStackPointer = SetSp(0x1f800400);
//#endif
		QuatToPSXMatrix(&act->actor->qRot, &act->actor->bffMatrix);
//#if GOLDCD==1
//		SetSp(oldStackPointer);
//#endif

		act->actor->bffMatrix.t[0] = -act->actor->position.vx;
		act->actor->bffMatrix.t[1] = act->actor->position.vy;
		act->actor->bffMatrix.t[2] = act->actor->position.vz;

		(*mesh)->posx = -act->actor->position.vx;
		(*mesh)->posy = -act->actor->position.vy;
		(*mesh)->posz = act->actor->position.vz;

		//calculate local to screen coords for fma mesh.
		//(camera matrix and objects' pos/rot matrix)

		gte_SetRotMatrix(&GsWSMATRIX);
		gte_SetTransMatrix(&GsWSMATRIX);

		// Unnecessary maths for landscape segments, where pos is always zero.
		gte_ldlvl( &(*mesh)->posx);
		gte_rtirtr();
		gte_stlvl(&tx.t);

/*			
		gte_MulMatrix0(&GsWSMATRIX, &act->actor->bffMatrix, &tx);
		rY.m[0][0] = rY.m[1][1] = rY.m[2][2] = act->actor->size.vx;
		rY.m[0][1] = rY.m[0][2] = rY.m[1][0] = rY.m[1][2] = rY.m[2][0] = rY.m[2][1] = 0;
		RotMatrixY(2048, &rY);
		gte_MulMatrix0(&tx, &rY, &tx);
		gte_SetRotMatrix(&tx);
		gte_SetTransMatrix(&tx);
*/
//bbopt
		gte_MulMatrix0(&GsWSMATRIX, &act->actor->bffMatrix, &tx);
		rY.m[0][1] = rY.m[0][2] = rY.m[1][0] = rY.m[1][2] = rY.m[2][0] = rY.m[2][1] = 0;
		rY.m[0][0] = -act->actor->size.vx;
		rY.m[1][1] =  act->actor->size.vy;
		rY.m[2][2] = -act->actor->size.vz;
		gte_MulMatrix0(&tx, &rY, &tx);
		gte_SetRotMatrix(&tx);
		gte_SetTransMatrix(&tx);

		SetVectorFF( &fmaActorScale, &act->actor->size );
		
		
		if(FmaActor_ClipCheck(*mesh))
			act->clipped=0;
		else
			act->clipped=1;
	}

	//miles away?
// 	if(act->clipped)
// 	{
// 		int debugTemp = 0;
// 		int milesDist = 100;
// 
// 		if( (distTop<-milesDist) || (distRight>milesDist)
// 		 || (distBott>milesDist) || (distLeft<-milesDist) )
// 		{
// 			debugTemp++;
// 		}
// 	}


	//debug - draw clipping rectangle
#ifdef DRAW_SCREEN_CLIP
	if(!act->clipped)
	{
		int x0,x1;
		int y0,y1;
		TextureType *tempTex;
		int i;

		MATRIX tx, rY;
		SHORTXY sBox[8];
//		FMA_MESH_HEADER **mesh = ((char*)act->bffActor) + sizeof(FMA_WORLD);
		FMA_MESH_HEADER **mesh = ADD2POINTER ( act->bffActor, sizeof ( FMA_WORLD ) );


		//calculate local to screen coords for fma mesh.
		//(camera matrix and objects' pos/rot matrix)
		gte_SetRotMatrix(&GsWSMATRIX);
		gte_SetTransMatrix(&GsWSMATRIX);

		QuatToPSXMatrix(&act->actor->qRot, &act->actor->bffMatrix);

		act->actor->bffMatrix.t[0] = -act->actor->position.vx;
		act->actor->bffMatrix.t[1] = act->actor->position.vy;
		act->actor->bffMatrix.t[2] = act->actor->position.vz;

		(*mesh)->posx = -act->actor->position.vx;
		(*mesh)->posy = -act->actor->position.vy;
		(*mesh)->posz = act->actor->position.vz;



		gte_ldlvl( &(*mesh)->posx);
		gte_rtirtr();
		gte_stlvl(&tx.t);
		
				gte_MulMatrix0(&GsWSMATRIX, &act->actor->bffMatrix, &tx);
		rY.m[0][0] = rY.m[1][1] = rY.m[2][2] = act->actor->size.vx;
		rY.m[0][1] = rY.m[0][2] = rY.m[1][0] = rY.m[1][2] = rY.m[2][0] = rY.m[2][1] = 0;
		RotMatrixY(2048, &rY);
		gte_MulMatrix0(&tx, &rY, &tx);
		gte_SetRotMatrix(&tx);
		gte_SetTransMatrix(&tx);



		//get screen coords of bounding box
//		FMA_MESH_HEADER **mesh = ((char*)act->actor->bffActor) + sizeof(FMA_WORLD);
//		SHORTXY sBox[8];
		FmaActor_GetSBox(*mesh, sBox);

		//calc min\max of box
		x0=256;
		x1=-256;
		y0=128;
		y1=-128;
		for(i=0; i<8; i++)
		{
			if(sBox[i].x < x0)
				x0 = sBox[i].x;

			else if(sBox[i].x > x1)
				x1 = sBox[i].x;

			if(sBox[i].y < y0)
				 y0 = sBox[i].y;

			else if(sBox[i].y > y1)
				 y1 = sBox[i].y;
		}


		tempTex = FindTexture("NEBU");
		if(!tempTex)
		{
			POLY_F4 *f4;

			//draw poly
			BEGINPRIM(f4, POLY_F4);
			setPolyF4(f4);
			f4->x0 = x0;
			f4->y0 = y0;
			f4->x1 = x1;
			f4->y1 = y0;
			f4->x2 = x0;
			f4->y2 = y1;
			f4->x3 = x1;
			f4->y3 = y1;
			f4->r0 = 128;
			f4->g0 = 0;
			f4->b0 = 0;
			f4->code |= 2;//semi-trans on
	//		SetSemiTrans(f4,1);
			ENDPRIM(f4, 1, POLY_F4);
		}
		else
		{
			POLY_FT4 *ft4;

			//draw poly
			BEGINPRIM(ft4, POLY_FT4);
			setPolyFT4(ft4);
			ft4->x0 = x0;
			ft4->y0 = y0;
			ft4->x1 = x1;
			ft4->y1 = y0;
			ft4->x2 = x0;
			ft4->y2 = y1;
			ft4->x3 = x1;
			ft4->y3 = y1;
			ft4->r0 = 16;
			ft4->g0 = 16;
			ft4->b0 = 16;
			ft4->u0 = tempTex->u0;
			ft4->v0 = tempTex->v0;
			ft4->u1 = tempTex->u1;
			ft4->v1 = tempTex->v1;
			ft4->u2 = tempTex->u2;
			ft4->v2 = tempTex->v2;
			ft4->u3 = tempTex->u3;
			ft4->v3 = tempTex->v3;
			ft4->tpage = tempTex->tpage;
			ft4->clut  = tempTex->clut;
			ft4->code  |= 2;//semi-trans on
			ft4->tpage |= 32;//add
// 			ft4->tpage = si->tpage | 64;//sub
			ENDPRIM(ft4, 1, POLY_FT4);
		}

	}//end if(!act->clipped)

#endif //DRAW_SCREEN_CLIP

}



static unsigned short ascii_table[3][2] = {		// LookUp Tables for the Ascii to Sjis Function.
	{0x824f, 0x30},	// 0-9 
	{0x8260, 0x41},	// A-Z 
	{0x8281, 0x61},	// a-z 
};

static unsigned short ascii_k_table[] = {		// ASCII code to Shift-JIS code transfer table (kigou)
	0x8140,		//   //
	0x8149,		// ! //
	0x8168,		// " //
	0x8194,		// # //
	0x8190,		// $ //
	0x8193,		// % //
	0x8195,		// & //
	0x8166,		// ' //
	0x8169,		// ( //
	0x816a,		// ) //
	0x8196,		// * //
	0x817b,		// + //
	0x8143,		// , //
	0x817c,		// - //
	0x8144,		// . //
	0x815e,		// / //
	0x8146,		// : //
	0x8147,		// ; //
	0x8171,		// < //
	0x8181,		// = //
	0x8172,		// > //
	0x8148,		// ? //
	0x8197,		// @ //
	0x816d,		// [ //
	0x818f,		// \ //
	0x816e,		// ] //
	0x814f,		// ^ //
	0x8151,		// _ //
	0x8165,		// ` //
	0x816f,		// { //
	0x8162,		// | //
	0x8170,		// } //
	0x8150,		// ~ //
};

void asciiStringToSJIS(unsigned char *string, unsigned char *dest) 
{
	int	i;
	
	int sjis_code;
	int ascii_code;
	
	unsigned char stmp;
	unsigned char stmp2;

	unsigned char *dest2;
						   
	dest2 = dest;

	for( i=0; i<32; i++ )
	{
		*dest2++ = 0x81;				  
		*dest2++ = 0x40;
	}
	
							    
	while(*string) {
		stmp2 = 0;
		ascii_code = *string++;
		if ((ascii_code >= 0x20) && (ascii_code <= 0x2f))
			stmp2 = 1;
		else
		if ((ascii_code >= 0x30) && (ascii_code <= 0x39))
			stmp = 0;
		else
		if ((ascii_code >= 0x3a) && (ascii_code <= 0x40))
			stmp2 = 11;
		else
		if ((ascii_code >= 0x41) && (ascii_code <= 0x5a))
			stmp = 1;
		else
		if ((ascii_code >= 0x5b) && (ascii_code <= 0x60))
			stmp2 = 37;
		else
		if ((ascii_code >= 0x61) && (ascii_code <= 0x7a))
			stmp = 2;
		else
		if ((ascii_code >= 0x7b) && (ascii_code <= 0x7e))
			stmp2 = 63;
		else {
			printf("bad ASCII code 0x%x\n", ascii_code);
			exit(1);
		}

		if(stmp2)
			sjis_code = ascii_k_table[ascii_code - 0x20 - (stmp2 - 1)];
		else
			sjis_code = ascii_table[stmp][0] + ascii_code - ascii_table[stmp][1];

		// Write sjis
		*dest++ = (sjis_code&0xff00)>>8;
		*dest++ = (sjis_code&0xff);
	}
}

void myPadHandleInput()
{
	padHandleInput();
	if((padData.present[1] == 0) || (padData.numPads[0] != 4))
	{
		padData.present[1] = padData.present[4];
		padData.digital[1] = padData.digital[4];
		padData.debounce[1] = padData.debounce[4];
	}
}

static short videoKeyPress()
{
	if(padData.digital[0] & PAD_START)
		quitAllVideo = 1;
		
	return ((padData.digital[0] & (PAD_CROSS|PAD_START))>0);
}

void StartVideoPlayback(int num)
{
	RECT rect;
	StrDataType str;
	char strname[32];

	rect.x = rect.y = 0;
	rect.w = 512;
	rect.h = 512;


	quitAllVideo = 0;
	if (XAgetStatus())
	{
		LoadCodeOverlay(VIDEO_OVERLAY);
		ClearImage(&rect, 0,0,0);
   		sprintf(strname,"\\%s.STR;1",fmv[num].name);
		str.strName = &strname;
		utilPrintf("Playing stream %s\n",str.strName);
   		str.mode = STR_MODE24;
   		str.drawBorders = STR_BORDERS_ON;
   		str.scrWidth = 320;
   		str.x = 0;
   		str.y = 24+PALMODE*8;
   		str.width = 320;
   		str.height = 192;
   		str.endFrame = fmv[num].len;
   		str.vlcBufSize = 70000;
   		str.volume = (127*globalMusicVol)/MAX_SOUND_VOL;
   		videoPlayStream(&str, PALMODE, videoKeyPress);
		LoadCodeOverlay(GAME_OVERLAY);
	}
}
