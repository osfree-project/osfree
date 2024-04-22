/*!

   @file all_querypathinfo.c

   @brief utility functions for query path info
   shared along all code

   (c) osFree Project 2024, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>
*/

#include <all_shared.h> /*  shared functions and defines */

APIRET all_QueryPathInfo(PSZ  pszPathName,
                        ULONG ulInfoLevel,
                        PVOID pInfo,
                        ULONG cbInfoBuf)
{
#ifdef __386__
 return DosQueryPathInfo(pszPathName, ulInfoLevel, pInfo, cbInfoBuf);
#else
 return DosQPathInfo(pszPathName, ulInfoLevel, (PBYTE)pInfo, cbInfoBuf, 0);
#endif
}
