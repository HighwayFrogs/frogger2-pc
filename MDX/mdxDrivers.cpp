#ifdef __cplusplus
extern "C"
{
#endif

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <crtdbg.h>
#include <stdio.h>

#include "mgeReport.h"
#include "mdxInfo.h"
#include "mdxDDraw.h"
#include "mdxException.h"

struct DDDEVICE
{
	GUID *guid;
	DDCAPS caps;
	char *desc;
	char *name;
	long idx;
};

struct DDDEVICELIST
{	
	DDDEVICE list[100];
	unsigned long num;
};

DDDEVICELIST devices;

/*  --------------------------------------------------------------------------------
    Function      : EnumDDDevices
	Purpose       :	-
	Parameters    : (GUID FAR* lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext))
	Returns       : static BOOL FAR PASCAL 
	Info          : Call with : DirectDrawEnumerate(EnumDDDevices, NULL);
*/

static BOOL FAR PASCAL EnumDDDevices(GUID FAR* lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext)
{
    LPDIRECTDRAW	lpDD;
    DDCAPS			ddCaps;
	LPDIRECTDRAW4	lpDD4;
	DDDEVICEIDENTIFIER ddId;
	HRESULT result;
	
	// Create a temporary DD object
	if (DirectDrawCreate(lpGUID, &lpDD, NULL)!=DD_OK)				
		return DDENUMRET_OK;										// Cannot get, so continue on to the next one

	// get A dd4 pointer from it
	if (lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&lpDD4)!=DD_OK)
		return DDENUMRET_OK;

	// And get an ID for it!
	if (lpDD4->GetDeviceIdentifier(&ddId,0)!=DD_OK)
		return DDENUMRET_OK;
	
	// Get the caps for the device
	DDINIT(ddCaps);													
 	if ((result = lpDD4->GetCaps(&ddCaps, NULL))!=DD_OK)
	{
		// Oops couldn't get it
		ddShowError(result);

		// Release both surfaces
		lpDD->Release();
		lpDD4->Release();

		// Continue looking.
		return DDENUMRET_OK;
	}
	
	// If we get here, then we have a device we can possibly use, so add it to the list.
	strcpy( devices.list[devices.num].desc = (char *) AllocMem(sizeof(char)*strlen (ddId.szDescription)),ddId.szDescription);
	strcpy( devices.list[devices.num].name = (char *) AllocMem(sizeof(char)*strlen (ddId.szDriver)),ddId.szDriver);

	// Implicit structure copy.
	devices.list[devices.num].caps = ddCaps;						

	// If we are not the primary device, (Ie, not NULL for the guid)
	if (lpGUID)
	{
		// Make a copy of the guid
	    devices.list[devices.num].guid = (GUID *) AllocMem(sizeof(GUID));
		memcpy(devices.list[devices.num].guid, lpGUID, sizeof(GUID));		
	}
	else
		devices.list[devices.num].guid = NULL;

	// Onto the next device!
	devices.num++;
	lpDD->Release();
    return DDENUMRET_OK;
}

#ifdef __cplusplus
}
#endif




