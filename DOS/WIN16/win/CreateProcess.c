/*    
	CreateProcess.c	1.2
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

BOOL WINAPI
CreateProcess
(
  LPCTSTR  lpApplicationName,	/* pointer to name of executable module */
  LPTSTR  lpCommandLine,	/* pointer to command line string */
  LPSECURITY_ATTRIBUTES  lpProcessAttributes,	/* pointer to process security attributes */
  LPSECURITY_ATTRIBUTES  lpThreadAttributes,	/* pointer to thread security attributes */
  BOOL  bInheritHandles,	/* handle inheritance flag */
  DWORD  dwCreationFlags,	/* creation flags */ 
  LPVOID  lpEnvironment,	/* pointer to new environment block */
  LPCTSTR  lpCurrentDirectory,	/* pointer to current directory name */ 
  LPSTARTUPINFO  lpStartupInfo,	/* pointer to STARTUPINFO */ 
  LPPROCESS_INFORMATION  lpProcessInformation 	/* pointer to PROCESS_INFORMATION */
)
{
  return( FALSE );
}
