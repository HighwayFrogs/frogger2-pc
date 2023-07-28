/******************************************************************************************
	CRTEST PS   (c) 1999-2001 ISL

	snapshot.h:			BMP snapshot grabber
******************************************************************************************/

#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_

/*
// Example call (note final backslash)
	if(padData.debounce[4] & PAD_START)
	{
		SnapShot("C:\\PSX\\CRTEST\\SNAPSHOT\\");
	}
*/

int SnapShot(char *root);

#endif
