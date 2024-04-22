/*!

   @file all_setpathinfo.c

   @brief utility functions for set path info
   shared along all code

   (c) osFree Project 2024, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>
*/

#include <all_shared.h> /*  shared functions and defines */

APIRET all_SetPathInfo(PSZ pszPathName,
                       ULONG ulInfoLevel,
                       PVOID pInfoBuf,
                       ULONG cbInfoBuf,
                       ULONG flOptions)
{
#ifdef __386__
 return DosSetPathInfo(pszPathName, ulInfoLevel, pInfoBuf, cbInfoBuf, flOptions);
#else
 return DosSetPathInfo(pszPathName, ulInfoLevel, (PBYTE)pInfoBuf, cbInfoBuf, flOptions, 0);
#endif
}
