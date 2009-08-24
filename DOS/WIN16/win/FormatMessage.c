/*    
	FormatMessage.c	1.7
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include "windows.h"
#include "Win_Base.h"
#include "stdarg.h"

/********************************************************************

    FormatMessage()

    DWORD dwFlags,	 source and processing options 
    LPCVOID lpSource,	 pointer to  message source 
    DWORD dwMessageId,	 requested message identifier 
    DWORD dwLanguageId,	 language identifier for requested message 
    LPTSTR lpBuffer,	 pointer to message buffer 
    LPTSTR lpBuffer,	 pointer to message buffer 
    DWORD nSize,	 maximum size of message buffer 
    va_list *Arguments 	 address of array of message inserts 

**********************************************************************/
DWORD WINAPI
FormatMessage(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPTSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments 
   )
{
   if(dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
   {
   }
   else
   {    
   }

   if(dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS)
      ;
   if(dwFlags & FORMAT_MESSAGE_FROM_STRING)
      ;
   if(dwFlags & FORMAT_MESSAGE_FROM_HMODULE)
      ;
   if(dwFlags & FORMAT_MESSAGE_FROM_SYSTEM)
      ;
   if(dwFlags & FORMAT_MESSAGE_ARGUMENT_ARRAY)
      ;
   if(dwFlags & FORMAT_MESSAGE_MAX_WIDTH_MASK)
      ;

   return 0;
}
 

