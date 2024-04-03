/*!
   $Id: cmd_Messages.h,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $ 
   
   @file cmd_messages.h

   @brief message-related utility functions header, 
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _CMD_MESSAGES_H_
#define _CMD_MESSAGES_H_

//#include <os2.h>

/* C standard library headers */
#include <stdarg.h>

void __cdecl cmd_ShowSystemMessage(ULONG ulMsgID,ULONG ulParams,...);
void __cdecl cmd_ShowMessage(PSZ pszFileName,ULONG ulMsgID,ULONG ulParams,...);
void __cdecl cmd_vShowMessage(PSZ pszFileName,ULONG ulMsgID,ULONG ulParams,
                              va_list args);

int cmd_prntmsg(PCHAR *pTable, ULONG cTable, ULONG ulMsgID,PSZ pszFileName);

#endif /* _CMD_MESSAGES_H_ */
