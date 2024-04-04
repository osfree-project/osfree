/*!
   $Id: cmd_ExecFSEntry.h,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $

   @file cmd_execfsentry.h

   @brief cmd_ExecFSEntry utility function header,
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _CMD_EXECFSENTRY_H_
#define _CMD_EXECFSENTRY_H_

//#include <os2.h>

/* FS utlity dll entry points name definitions */
#define cmd_FS_CHKDSK  "CHKDSK"
#define cmd_FS_FORMAT  "FORMAT"
#define cmd_FS_RECOVER "RECOVER"
#define cmd_FS_SYS     "SYS"

//APIRET cmd_ExecFSEntry(PSZ pszFSName,PSZ pszEntryName,BOOL fVerbose,
//                       ULONG argc, PSZ argv[],PSZ envp[]);

APIRET cmd_ExecFSEntry(char *pszFSName, char *pszEntryName, BOOL fVerbose,
                       ULONG argc, char **argv, char **envp);

#endif /* _CMD_EXECFSENTRY_H_ */
