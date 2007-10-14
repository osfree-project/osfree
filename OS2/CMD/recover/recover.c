/*!
   $Id: recover.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $

   @file recover.c

   @brief recover command - frontend for recover entry in fs utility dll

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSERRORS
#include <osfree.h> /* system header */
#include <cmd_shared.h> /* comand line tools' shared functions and defines*/

/* C standard library headers */
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'


/*!
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main (int argc, char* argv[], char* envp[])
{
  PSZ pszFSName;  /* FS Name */
  PSZ disk="   ";
  int i;          /* counter and etc.. */
  APIRET rc;
  ULONG ulAvailableDisks;

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?')&& (argv[1][2]=='\0'))
 {
   cmd_ShowSystemMessage(cmd_MSG_RECOVER_HELP,0L);
   return NO_ERROR;
 };

 /* is there a disk specified on a command line? if not, use current */
 for (i=1;i<argc;i++)
  if (argv[i][1]==':')
  {
    strncpy(disk,argv[i],2);
    break;
  };

  /* get current disk drive name */
  if (i==argc)
   if ((rc=cmd_QueryCurrentDisk(disk,&ulAvailableDisks))!=NO_ERROR)
     return cmd_ERROR_EXIT;

 /* get name of FS for the specified disk */
 pszFSName=calloc(CCHMAXPATH,1);

 /* execute appriate function from FS utility dll */
 if (cmd_QueryFSName(disk,pszFSName))
 {
   free(pszFSName);
   return cmd_ERROR_EXIT;
 };

 cmd_ShowSystemMessage(cmd_MSG_TYPE_OF_FS_IS,1L,"%s",pszFSName);

 rc=cmd_ExecFSEntry(pszFSName,cmd_FS_RECOVER,FALSE,argc,argv,envp);

 free(pszFSName);
 return rc;
};
