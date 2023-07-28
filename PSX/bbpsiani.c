#include "bbpsiani.h"

//#define LAST_KEY_CHECK

typedef struct{
	SHORT x,y,z,w;
}SHORTQUAT;


//debug
//#define COUNT_OBJECTS

#ifdef COUNT_OBJECTS
int rotatedObjects;
int scaledObjects;
int movedObjects;
#endif




void bb_psiSetKeyFrames(PSIOBJECT *world, ULONG frame)
{
	bb_psiSetMoveKeyFrames(world, frame);
	bb_psiSetScaleKeyFrames(world, frame);
	bb_psiSetRotateKeyFrames(world, frame);
}

void bb_psiSetRotateKeyFrames(PSIOBJECT *world, ULONG frame)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;


	while(world)
	{
#ifdef COUNT_OBJECTS
		rotatedObjects++;
#endif

		mesh = world->meshdata;
		tmprotatekeys = mesh->rotatekeys;
		
		if ((!frame) || (mesh->numRotateKeys<=1))
		{
			ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
		}
		else
		{
			int quickFound = 0;

			////////////////////////////////////////////////////////////////
			//N.B set temprotatekeys to keyframe at END of range it's in, //
			//or keyframe AT time. (to fit in with existing functionality)//
			////////////////////////////////////////////////////////////////

			//ok, we know the last keyframe we used.
			//is it still valid?
#ifdef LAST_KEY_CHECK
			//was it 0?
			if(mesh->lastrotatekey==0)
			{
				//We don't want to do some of the checks here,
				//like access lastrotatekey-1.

				//still 0?
				if(frame == mesh->rotatekeys[0].time)
//				if(frame == 0)
				{
					tmprotatekeys = &mesh->rotatekeys[0];
					quickFound=1;
				}

				//between 0 and 1?
				else if(frame <= mesh->rotatekeys[1].time)
				{
					//yipee! still the keyframe
					tmprotatekeys = &mesh->rotatekeys[1];
					quickFound=1;
				}

				//else
					//just let it search the whole lot
			}

			//ok, it was above 0.
			//still above last's prev?
			else if(frame > mesh->rotatekeys[mesh->lastrotatekey-1].time)
			{
				//still between lastrotatekey and it's prev?
				if(frame <= mesh->rotatekeys[mesh->lastrotatekey].time)
				{
					//yipee! still the keyframe
					tmprotatekeys = &mesh->rotatekeys[mesh->lastrotatekey];
					quickFound=1;
				}

				//no? ok, it may have moved over to the next.
				//check it.
				else if(mesh->lastrotatekey+1 < mesh->numRotateKeys)
				{
					//is it the next one on?
					if(frame <= mesh->rotatekeys[mesh->lastrotatekey+1].time)
					{
						//yipee it's moved on to this one
						tmprotatekeys = &mesh->rotatekeys[mesh->lastrotatekey+1];
						quickFound=1;
					}

					//ok, it's somewhere abouve lastrotatekey+1.
					//set up the search to continue from here.
					else
					{
						tmprotatekeys = &mesh->rotatekeys[mesh->lastrotatekey+1];
//						tmprotatekeys = &mesh->rotatekeys[mesh->lastrotatekey+2];
//						keystep = (mesh->numRotateKeys - (mesh->lastrotatekey+1)) >>1;
						keystep = (mesh->numRotateKeys - (mesh->lastrotatekey+2)) >>1;
					}

				}
			}
			//Ah, perhaps it's animating backwards,
			//and has moved to the prev one.
			else if(mesh->lastrotatekey-2 >= 0)
			{
				//is it the last's prev one?
				if(frame > mesh->rotatekeys[mesh->lastrotatekey-2].time)
				{
					//yipee it's moved back to this one
					tmprotatekeys = &mesh->rotatekeys[mesh->lastrotatekey-1];
					quickFound=1;
				}

				//ok, it's somewhere before lastrotatekey-1.
				//set up the search to continue from here.
				else
				{
					tmprotatekeys = &mesh->rotatekeys[0];
					keystep = mesh->lastrotatekey >>1;
				}
			}
#endif

			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			if(!quickFound)
			{
				keystep = mesh->numRotateKeys / 2;
				tmprotatekeys = &mesh->rotatekeys[keystep];
				if(keystep>1)
	//				keystep /= 2;
					keystep >>= 1;
				
	 			while (frame != tmprotatekeys->time)
	 			{
	 				if (frame > tmprotatekeys->time)
	 				{
	 					if ( frame <= tmprotatekeys[1].time )
	 					{
	 						tmprotatekeys++;
	 						break;
	 					}
	 					else
	 					{
	 						tmprotatekeys += keystep;
	 					}
	 				}
	 				else
	 				{
	 					tmprotatekeys -= keystep;
	 				}
	 
	 				if(keystep>1)
	// 					keystep /= 2;
	 					keystep >>= 1;
	 			}

			}//end if !quickFound


#ifdef LAST_KEY_CHECK

			//bbopt - store a pointer to the key instead.
			//also, can;t seem to wroite to these without
			//causing a bug.

			//bbopt - part of anim optimisation
			mesh->lastrotatekey = (((int)tmprotatekeys)-((int)mesh->rotatekeys)) / sizeof(SQKEYFRAME);
#endif

			////////////////////////////////////////////
	
			if (tmprotatekeys->time == frame) // it's on the keyframe 
			{
				ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
			}
			else // work out the differences 
			{
				tmprotatekeyslast = tmprotatekeys - 1;

				if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
				{
					ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
				}
				else
				{
					t =  tmprotatekeyslast->time;
					t =  ((frame - t) << 12)/(tmprotatekeys->time - t);
					ShortquaternionSlerpMatrix((SHORTQUAT *)&tmprotatekeyslast->vect,
									(SHORTQUAT *)&tmprotatekeys->vect,
									t,
									&world->matrix);
//bbopt - noticable opn some enemies
//					ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
				}
			}
		}
	   
		if((world->rotate.vx) || (world->rotate.vy) || (world->rotate.vz))
		{
			RotMatrixYXZ_gte(&world->rotate,&rotmat1);
			gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);
		}

		
		if(world->child)
			bb_psiSetRotateKeyFrames(world->child,frame);

		world = world->next;

	}//end looping objects
}

void bb_psiSetScaleKeyFrames(PSIOBJECT *world, ULONG frame)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	USHORT		oldframe=frame;
	LONG		t;
	LONG keystep;
	PSIMESH		*mesh;


	while(world)
	{
#ifdef COUNT_OBJECTS
		scaledObjects++;
#endif

		mesh = world->meshdata;
		tmpscalekeys = mesh->scalekeys;

		if ((!frame) || (mesh->numScaleKeys<=1))
		{
			world->scale.vx = (tmpscalekeys->vect.x)<<2;
			world->scale.vy = (tmpscalekeys->vect.y)<<2;
			world->scale.vz = (tmpscalekeys->vect.z)<<2;
		}
		else
		{
			int quickFound = 0;

			//N.B set tempscalekeys to keyframe at END of range it's in,
			//or keyframe At time. (to fit in with existing functionality)


			//ok, we know the last keyframe we used.
			//is it still valid?
#ifdef LAST_KEY_CHECK
			//was it 0?
			if(mesh->lastscalekey==0)
			{
				//We don't want to do some of the checks here,
				//like access lastrotatekey-1.

				//still 0?
				if(frame == mesh->scalekeys[0].time)
//				if(frame == 0)
				{
					tmpscalekeys = &mesh->scalekeys[0];
					quickFound=1;
				}

				//between 0 and 1?
				else if(frame <= mesh->scalekeys[1].time)
				{
					//yipee! still the keyframe
					tmpscalekeys = &mesh->scalekeys[1];
					quickFound=1;
				}

				//else
					//just let it search the whole lot
			}

			//ok, it was above 0.
			//still above last's prev?
			else if(frame > mesh->scalekeys[mesh->lastscalekey-1].time)
			{
				//still between this and the next?
				if(frame <= mesh->scalekeys[mesh->lastscalekey].time)
				{
					//yipee! still the keyframe
					tmpscalekeys = &mesh->scalekeys[mesh->lastscalekey];
					quickFound=1;
				}

				//no? ok, it may have moved over to the next.
				//check it.
				else if(mesh->lastscalekey+1 < mesh->numScaleKeys)
				{
					//is it the next one on?
					if(frame <= mesh->scalekeys[mesh->lastscalekey+1].time)
					{
						//yipee it's moved on to this one
						tmpscalekeys = &mesh->scalekeys[mesh->lastscalekey+1];
						quickFound=1;
					}

					//ok, it's somewhere abouve lastrotatekey+1.
					//set up the search to continue from here.
					else
					{
						tmpscalekeys = &mesh->scalekeys[mesh->lastscalekey+1];
//						tmpscalekeys = &mesh->scalekeys[mesh->lastscalekey+2];
//						keystep = (mesh->numScaleKeys - (mesh->lastscalekey+1)) >>1;
						keystep = (mesh->numScaleKeys - (mesh->lastscalekey+2)) >>1;
					}
				}
			}
			//Ah, perhaps it's animating backwards,
			//and has moved to the prev one.
			else if(mesh->lastscalekey-2 >= 0)
			{
				//is it the last's prev one?
				if(frame > mesh->scalekeys[mesh->lastscalekey-2].time)
				{
					//yipee it's moved back to this one
					tmpscalekeys = &mesh->scalekeys[mesh->lastscalekey-1];
					quickFound=1;
				}

				//ok, it's somewhere before lastrotatekey-1.
				//set up the search to continue from here.
				else
				{
					tmpscalekeys = &mesh->scalekeys[0];
					keystep = mesh->lastscalekey >>1;
				}
			}
#endif

			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			if(!quickFound)
			{
				keystep = mesh->numScaleKeys / 2;
				tmpscalekeys = &mesh->scalekeys[keystep];
				if(keystep>1)
	//				keystep /= 2;
					keystep >>= 1;
				
	 			while (frame != tmpscalekeys->time)
	 			{
	 				if (frame > tmpscalekeys->time)
	 				{
	 					if ( frame <= tmpscalekeys[1].time )
	 					{
	 						tmpscalekeys++;
	 						break;
	 					}
	 					else
	 					{
	 						tmpscalekeys += keystep;
	 					}
	 				}
	 				else
	 				{
	 					tmpscalekeys -= keystep;
	 				}
	 
	 				if(keystep>1)
	// 					keystep /= 2;
	 					keystep >>= 1;
	 			}
			}

#ifdef LAST_KEY_CHECK
			//bbopt - part of anim optimisation
			mesh->lastscalekey = (tmpscalekeys - mesh->scalekeys) / sizeof(SVKEYFRAME);
#endif

			////////////////////////////////////////////

			if(tmpscalekeys->time == frame)	// it's on the keyframe 
			{
				world->scale.vx = (tmpscalekeys->vect.x)<<2;
				world->scale.vy = (tmpscalekeys->vect.y)<<2;
				world->scale.vz = (tmpscalekeys->vect.z)<<2;
			}
			else // work out the differences 
			{
				tmpscalekeyslast = tmpscalekeys - 1;

				if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
				{
					world->scale.vx = (tmpscalekeys->vect.x)<<2;
					world->scale.vy = (tmpscalekeys->vect.y)<<2;
					world->scale.vz = (tmpscalekeys->vect.z)<<2;
				}
				else
				{
					VECTOR temp1;
					VECTOR temp2;
					temp1.vx =((int)(tmpscalekeyslast->vect.x))<<2;
					temp1.vy =((int)(tmpscalekeyslast->vect.y))<<2;
					temp1.vz =((int)(tmpscalekeyslast->vect.z))<<2;
					temp2.vx =((int)(tmpscalekeys->vect.x))<<2;
					temp2.vy =((int)(tmpscalekeys->vect.y))<<2;
					temp2.vz =((int)(tmpscalekeys->vect.z))<<2;

					t = tmpscalekeyslast->time;
					t = ((frame - t) << 12) / (tmpscalekeys->time - t);

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&temp1);						// load source
					gte_ldfc(&temp2);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->scale);				// store interpolated vector

				}

			}
		}	


		if(world->child)
			bb_psiSetScaleKeyFrames(world->child,oldframe);

		world = world->next;

	}//end looping objects
}

void bb_psiSetMoveKeyFrames(PSIOBJECT *world, ULONG frame)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		source, dest;
	long		keystep;
	PSIMESH 	*mesh;


	while(world)
	{
#ifdef COUNT_OBJECTS
		movedObjects++;
#endif

		mesh = world->meshdata;
		tmpmovekeys = mesh->movekeys;//+frame;

		if ((!frame) || (mesh->numMoveKeys<=1) )
		{
			world->matrix.t[0] = (tmpmovekeys->vect.x);
			world->matrix.t[1] = -(tmpmovekeys->vect.y);
			world->matrix.t[2] = (tmpmovekeys->vect.z);
		}
		else
		{
			int quickFound = 0;

			//N.B set tempmovekeys to keyframe at END of range it's in,
			//or keyframe At time. (to fit in with existing functionality)


			//ok, we know the last keyframe we used.
			//is it still valid?
#ifdef LAST_KEY_CHECK
			//was it 0?
			if(mesh->lastmovekey==0)
			{
				//We don't want to do some of the checks here,
				//like access lastrotatekey-1.

				//still 0?
				if(frame == mesh->movekeys[0].time)
//				if(frame == 0)
				{
					tmpmovekeys = &mesh->movekeys[0];
					quickFound=1;
				}

				//between 0 and 1?
				else if(frame <= mesh->movekeys[1].time)
				{
					//yipee! still the keyframe
					tmpmovekeys = &mesh->movekeys[1];
					quickFound=1;
				}

				//else
					//just let it search the whole lot
			}

			//ok, it was above 0.
			//still above last's prev?
			else if(frame > mesh->movekeys[mesh->lastmovekey-1].time)
			{
				//still between this and the next?
				if(frame <= mesh->movekeys[mesh->lastmovekey].time)
				{
					//yipee! still the keyframe
					tmpmovekeys = &mesh->movekeys[mesh->lastmovekey];
					quickFound=1;
				}

				//no? ok, it may have moved over to the next.
				//check it.
				else if(mesh->lastmovekey+1 < mesh->numMoveKeys)
				{
					//is it the next one on?
					if(frame <= mesh->movekeys[mesh->lastmovekey+1].time)
					{
						//yipee it's moved on to this one
						tmpmovekeys = &mesh->movekeys[mesh->lastmovekey+1];
						quickFound=1;
					}

					//ok, it's somewhere abouve lastrotatekey+1.
					//set up the search to continue from here.
					else
					{
						tmpmovekeys = &mesh->movekeys[mesh->lastmovekey+1];
//						tmpmovekeys = &mesh->movekeys[mesh->lastmovekey+2];
//						keystep = (mesh->numMoveKeys - (mesh->lastmovekey+1)) >>1;
						keystep = (mesh->numMoveKeys - (mesh->lastmovekey+2)) >>1;
					}
				}
			}
			//Ah, perhaps it's animating backwards,
			//and has moved to the prev one.
			else if(mesh->lastmovekey-2 >= 0)
			{
				//is it the last's prev one?
				if(frame > mesh->movekeys[mesh->lastmovekey-2].time)
				{
					//yipee it's moved back to this one
					tmpmovekeys = &mesh->movekeys[mesh->lastmovekey-1];
					quickFound=1;
				}

				//ok, it's somewhere before lastrotatekey-1.
				//set up the search to continue from here.
				else
				{
					tmpmovekeys = &mesh->movekeys[0];
					keystep = mesh->lastmovekey >>1;
				}
			}
#endif

			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			if(!quickFound)
			{
				keystep = mesh->numMoveKeys / 2;
				tmpmovekeys = &mesh->movekeys[keystep];
				if(keystep>1)
	//				keystep /= 2;
					keystep >>= 1;
				
	 			while (frame != tmpmovekeys->time)
	 			{
	 				if (frame > tmpmovekeys->time)
	 				{
	 					if ( frame <= tmpmovekeys[1].time )
	 					{
	 						tmpmovekeys++;
	 						break;
	 					}
	 					else
	 					{
	 						tmpmovekeys += keystep;
	 					}
	 				}
	 				else
	 				{
	 					tmpmovekeys -= keystep;
	 				}
	 
	 				if(keystep>1)
	// 					keystep /= 2;
	 					keystep >>= 1;
	 			}
			}

#ifdef LAST_KEY_CHECK
			//bbopt - part of anim optimisation
			mesh->lastmovekey = (tmpmovekeys - mesh->movekeys) / sizeof(SVKEYFRAME);
#endif

			////////////////////////////////////////////

			if(tmpmovekeys->time == frame)	// it's on the keyframe 
			{
				world->matrix.t[0] = (tmpmovekeys->vect.x);
				world->matrix.t[1] = -(tmpmovekeys->vect.y);
				world->matrix.t[2] = (tmpmovekeys->vect.z);
			}
			else // work out the differences 
			{
				workingkeys = tmpmovekeys - 1;

				if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
				{
					world->matrix.t[0] = (tmpmovekeys->vect.x);
					world->matrix.t[1] = -(tmpmovekeys->vect.y);
					world->matrix.t[2] = (tmpmovekeys->vect.z);
				}
				else
				{
					t = workingkeys->time;
					t = ((frame - t) << 12) / (tmpmovekeys->time - t);
					
					source.vx = workingkeys->vect.x;
					source.vy = -workingkeys->vect.y;
					source.vz = workingkeys->vect.z;

					dest.vx = tmpmovekeys->vect.x;
					dest.vy = -tmpmovekeys->vect.y;
					dest.vz = tmpmovekeys->vect.z;

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&source);						// load source
					gte_ldfc(&dest);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->matrix.t);			// store interpolated vector
				}
			}
		}	


		if(world->child)
			bb_psiSetMoveKeyFrames(world->child,frame);
		
		world = world->next;

	}//end looping objects
}


