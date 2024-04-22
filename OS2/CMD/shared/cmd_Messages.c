/*!
   $Id: cmd_Messages.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $ 
  
   @file cmd_messages.c 
   
   @brief utility functions related to retrieving and displaying messages,
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>

   @author Cristiano Guadagnino <criguada@tin.it>
   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSERRORS
#define INCL_DOSMISC

/* C standard library headers */
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

#define MAX_MESSAGE 2048 /* message buffer size */

/* criguada@tin.it - Cristiano Guadagnino                               */

/* A little note: a standard 80x25 screenful is 2000 bytes worth, so we */
/* should always be safe with a buffer of 2Kb (AFAIK there are no       */
/* messages occupying more than one screenful). To be sure one can      */
/* test the cbMsgL returned, if it is > 2048 then the message has been  */
/* truncated.                                                           */

/* example 1: a call to show a message that has no parameters:          */
/*                                                                      */
/*           cmd_prntmsg(NULL, 0L, 1001L,"OSO001.MSG");                                  */
/*                                                                      */
/*                                                                      */
/* example 2: a call to show a message that has 1 parameter:            */
/*                                                                      */
/*  CHAR           *pTable[1];                                          */
/*  CHAR            fname[CCHMAXPATH] = "";                             */
/*                                                                      */
/*           strcpy(fname, "C:\\CONFIG.SYS");                           */
/*           pTable[0] = fname;                                         */
/*           cmd_prntmsg(pTable, 1L, 1480L,"OSO001.MSG");                                */
/*                                                                      */

int cmd_prntmsg(PCHAR *pTable, ULONG cTable, ULONG ulMsgID,PSZ pszFileName)
{
    UCHAR   pBuf[MAX_MESSAGE] = "";
#ifdef __386__
    ULONG   cbMsgL;
#else
    USHORT   cbMsgL;
#endif
    APIRET  ulrc;

    ulrc = DosGetMessage(pTable, cTable, pBuf, sizeof(pBuf), ulMsgID, pszFileName, &cbMsgL);

    if (ulrc != NO_ERROR) 
	{
		#ifdef __386__
        printf(all_GetSystemErrorMessage(ulrc));
		#else
        printf("%Ws", all_GetSystemErrorMessage(ulrc));
		#endif
	}
    else
        printf("%s", pBuf);

    return ulrc;
}


/*!
   cmd_ShowSystemMessage - retrieves and shows system (OSO001.MSG) messages

   @param ulMsgID    message id
   @param ulParams   parameter count for message (max 9 allowed)
   @param ...        variable arguments, for every message parameter do:
                        - char *fmt - format specifier for parameter - the same
                                      as for printf
                        - values to put in parameter
                     there is max 9 arguments for each message parameter
                     allowed

   example 1: call for message without parameters

   cmd_ShowSystemMessage(1001L,0L);
 
   example 2: call for message with two, string parameters

   PSZ pszString1="String1";
   PSZ pszString2="String2";

   cmd_ShowSystemMessage(1001L,2,"%s",pszString1,"%s",pszString2);
                                 ^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^
                                     param 1          param 2

   example 3: call for message with two paremeters; the first one is string,
   the second is built from two characters into hexadecimal value

   UCHAR ch1='c';
   UCHAR ch2='d';
   PSZ pszString="String";

   cmd_ShowSystemMessage(1001L,2,"%s",pszString,"0x%x:0x%x",ch1,ch2);
                                 ^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^
                                    param 1           param2  

*/
void __cdecl cmd_ShowSystemMessage(ULONG ulMsgID,ULONG ulParams,...)
{
  va_list args;
  va_start (args,ulParams);
  cmd_vShowMessage("OSO001.MSG",ulMsgID,ulParams,args);
  va_end (args);
};

/*!
   cmd_ShowMessage - retrieves and shows messages (for examples look
   at cmd_ShowSystemMessage function documentation)

   @param pszFileName  message file name
   @param ulMsgID      message id
   @param ulParams     parameter count for message (max 9 allowed)
   @param ...          variable arguments, for every message parameter do:
                        - char *fmt - format specifier for parameter - the same
                                      as for printf
                        - values to put in parameter
                       there is max 9 arguments for each message parameter
                       allowed
*/
void __cdecl cmd_ShowMessage(PSZ pszFileName,ULONG ulMsgID,ULONG ulParams,...)
{
  va_list args;
  va_start (args,ulParams);
  cmd_vShowMessage(pszFileName,ulMsgID,ulParams,args);
  va_end (args);
};

/*!
   cmd_vShowMessage - retrieves and shows messages - va_list version

   @param pszFileName  message file name
   @param ulMsgID      message id
   @param ulParams     parameter count for message (max 9 allowed)
   @param args         opened va_list (for variable number of parameters)
*/
void __cdecl cmd_vShowMessage(PSZ pszFileName,ULONG ulMsgID,ULONG ulParams,va_list args)
{
  PSZ argTable[9];
  ULONG i;

  for (i=0;i<ulParams;i++) argTable[i]=NULL; /* fix? */

  all_vprepareArgTable(ulParams,argTable,args);
  cmd_prntmsg(argTable,ulParams,ulMsgID,pszFileName);

  for (i=0;i<ulParams;i++)
#ifdef __386__
   free(argTable[i]);
#else
   _ffree(argTable[i]);
#endif
};

