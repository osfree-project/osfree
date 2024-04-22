/*!
   $Id: cmd_QueryCurrentDisk.h,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $ 
   
   @file cmd_querycurrentdisk.c
  
   @brief utility function header,shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _CMD_QUERYCURRENTDISK_H_
#define _CMD_QUERYCURRENTDISK_H_

//#include <os2.h>

APIRET cmd_QueryCurrentDisk(PSZ pszDiskName,ULONG *available);

#endif /* _CMD_QUERYCURRENTDISK_H_ */
