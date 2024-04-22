/*!
   $Id: cmd_QueryCurrentDisk.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $ 
   
   @file cmd_querycurrentdisk.c
   
   @brief utility function for getting current disk name and available disks
   bitmap, shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
//#include <osfree.h>         
//#include <os2.h>

/* C standard library headers */
#include <stdio.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

/*!
  Queries current disk name and disks availability bitmap

  @param diskName    user allocated buffer for disk name, must be at least 3
                     bytes long
  @param available   pointer to variable, where disks availability bitmap
                     will be stored (each bit is for each drive letter - see
                     label command sources for more

  @return
        - 0 - successfull completition
        - rc from DosQueryCurrentDisk on error, and error message on console
*/
APIRET cmd_QueryCurrentDisk(PSZ diskName,ULONG *available)
{
  APIRET rc;
  rc=all_QueryCurrentDisk(diskName,available);

  if (rc)
  {
	#ifdef __386__
    printf(all_GetSystemErrorMessage(rc));
	#else
    printf("%Ws", all_GetSystemErrorMessage(rc));
	#endif
    return rc;
  };

  return NO_ERROR;
};
