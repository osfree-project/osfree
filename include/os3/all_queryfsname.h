/*!
   $Id: all_queryfsname.h,v 1.1.1.1 2003/10/04 08:27:20 prokushev Exp $ 
  
   @file all_queryfsname.h  
 
   @brief all_QueryFSName utility function header, shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _ALL_QUERYFSNAME_H_
#define _ALL_QUERYFSNAME_H_

//#include <os2.h>

APIRET all_QueryFSName(PSZ pszGivenDriveName,PSZ pszFSName);

#endif /* _ALL_QUERYFSNAME_H_ */
