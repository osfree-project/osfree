/*!
   $Id: all_messages.h,v 1.1.1.1 2003/10/04 08:27:20 prokushev Exp $ 
   
   @file all_messages.h

   @brief message-related utility functions header, shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _ALL_MESSAGES_H_
#define _ALL_MESSAGES_H_

#include <osfree.h>

/* C standard library headers */
#include <stdarg.h>

PSZ all_GetSystemErrorMessage(ULONG ulRc);
PSZ all_GetSystemErrorHelp(ULONG ulRc);
void __cdecl all_prepareArgTable(ULONG ulParams,PSZ pszArgTable[],...);
void __cdecl all_vprepareArgTable(ULONG ulParams,PSZ pszArgTable[],
                                  va_list args);

#endif /* _ALL_MESSAGES_H_ */
