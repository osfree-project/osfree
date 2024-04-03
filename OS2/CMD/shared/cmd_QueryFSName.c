/*!
   $Id: cmd_QueryFSName.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $

   @file cmd_queryfsname.c

   @brief utility functions for querying File System name for specified disk
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
//#include <osfree.h>
//#include <os2.h>

/* C standard library headers */
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

/*!
  Query file system name for specified disk
  (simply calls all_QueryFSName)

  @param szGivenDriveName    disk name (eg.: "d:")
  @param szFSName            buffer for file system name (must be allocated by
                             user, and be enough big)

  @return
        - 0 - if completed successully
        - rc from DosQueryFSAttach, when error
*/
APIRET cmd_QueryFSName(PSZ szGivenDriveName,PSZ szFSName)
{
  APIRET rc=all_QueryFSName(szGivenDriveName,szFSName);
  if (rc) printf(all_GetSystemErrorMessage(rc));
  return rc;
};
