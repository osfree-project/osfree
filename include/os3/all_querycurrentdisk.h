/*!
   $Id: all_querycurrentdisk.h,v 1.1.1.1 2003/10/04 08:27:20 prokushev Exp $ 
   
   @file all_querycurrentdisk.c
  
   @brief utility function headr for getting current disk name and available disks 
   bitmap, shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _ALL_QUERYCURRENTDISK_H_
#define _ALL_QUERYCURRENTDISK_H_

#include <osfree.h>

APIRET all_QueryCurrentDisk(PSZ pszDiskName,ULONG *available);

#endif /* _ALL_QUERYCURRENTDISK_H_ */
