/*!
  
   @file all_querypathinfo.h  
 
   @brief all_QueryPathInfo utility function header, shared along all code

   (c) osFree Project 2024, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Yuri Prokushev <yuri.prokushev@gmail.com>
*/

#ifndef _ALL_QUERYPATHINFO_H_
#define _ALL_QUERYPATHINFO_H_

APIRET all_QueryPathInfo(PSZ  pszPathName,
                        ULONG ulInfoLevel,
                        PVOID pInfo,
                        ULONG cbInfoBuf);

#endif /* _ALL_QUERYPATHINFO_H_ */
