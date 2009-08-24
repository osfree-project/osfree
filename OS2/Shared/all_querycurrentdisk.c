/*!
   $Id: all_querycurrentdisk.c,v 1.1.1.1 2003/10/04 08:36:16 prokushev Exp $

   @file all_querycurrentdisk.c
   @brief utility function for getting current disk name and available disks
   bitmap, shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

/* C standard library headers */
#include <stdio.h>

#include <all_shared.h> /* shared functions and defines */

/*!
  Queries current disk name and disks availability bitmap

  @param diskName    user allocated buffer for disk name, must be at least 3
                     bytes long
  @param available   pointer to variable, where disks availability bitmap
                     will be stored (each bit is for each drive letter - see
                     label command sources for more

  @return
        - 0 - successfull completition
        - rc from DosQueryCurrentDisk on error
*/
APIRET all_QueryCurrentDisk(PSZ diskName,ULONG *available)
{
  APIRET rc;
  ULONG disknum;

  rc=DosQueryCurrentDisk(&disknum,available);
  if (rc) return rc;

  diskName[0] = (CHAR)(disknum - 1 + 'A');
  diskName[1] = ':';
  diskName[2] = '\0';

  return NO_ERROR;
};
