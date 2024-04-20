/*!
  
   @file all_setpathinfo.h  
 
   @brief all_SetPathInfo utility function header, shared along all code

   (c) osFree Project 2024, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>
*/

#ifndef _ALL_SETPATHINFO_H_
#define _ALL_SETPATHINFO_H_

APIRET all_SetPathInfo(PSZ pszPathName,
                       ULONG ulInfoLevel,
                       PVOID pInfoBuf,
                       ULONG cbInfoBuf,
                       ULONG flOptions);

#endif /* _ALL_SETPATHINFO_H_ */
