#ifndef __CRC32_H__
#define __CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
	Function : InitCRCTable
	Purpose : Initialise the CRC table
	Parameters : 
	Returns : 
	Info : 
*/

void InitCRCTable();


/*	--------------------------------------------------------------------------------
	Function : UpdateCRC
	Purpose : update a CRC for specified data
	Parameters : existing CRC, data ptr, size of data in bytes
	Returns : 
	Info : 
*/

unsigned long UpdateCRC(char *ptr);
unsigned long UpdateCRCData(char *ptr,int numBytes);

extern unsigned long objectCRC;


#ifdef __cplusplus
}
#endif
#endif