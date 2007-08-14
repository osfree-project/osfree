/*!
   $Id: cmd_ShowVolumeInfo.h,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $ 
   
   @file cmd_showvolumeinfo.h 
   @brief utility function header, shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _CMD_SHOWVOLUMEINFO_H_
#define _CMD_SHOWVOLUMEINFO_H_

#include <osfree.h>

APIRET cmd_ShowVolumeInfo(PSZ pszDrive,BOOL fUseSemicolon);

#endif /* _CMD_SHOWVOLUMEINFO_H_ */
