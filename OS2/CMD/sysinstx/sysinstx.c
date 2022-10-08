/*!

   @file

   @brief sysinstx command - frontend for sys entry in fs utility dll

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSERRORS
/* system header */
#include <os2.h>
#include <cmd_shared.h> /* comand line tools' shared functions and defines */

/* C standard library headers */
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

/*!
  @todo: add help message to shared message files
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main (int argc, char* argv[], char* envp[])
{
  PSZ pszFSName;  /* FS Name */
  int i;          /* counter and etc.. */
  PSZ disk="   ";
  APIRET rc;

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?') && (argv[1][2]=='\0'))
 {
  printf("We should print here some usage info...\n");
/*   cmd_ShowSystemMessage(cmd_MSG_SYSINSTX_HELP,0L); */
   return NO_ERROR;
 };

  if ((argc!=2) && (argc!=3) && (argv[1][1]!=':'))
  {
    cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return cmd_ERROR_EXIT;
  };

 strncpy(disk,argv[1],2);

 /* get name of FS for the specified disk */
 pszFSName=calloc(CCHMAXPATH,1);

 /* execute the appropriate function from FS utility dll */
 if (cmd_QueryFSName(disk,pszFSName))
 {
   free(pszFSName);
   return cmd_ERROR_EXIT;
 };

 cmd_ShowSystemMessage(cmd_MSG_TYPE_OF_FS_IS,1L,"%s",pszFSName);

 rc=cmd_ExecFSEntry(pszFSName,cmd_FS_SYS,FALSE,argc,argv,envp);

 free(pszFSName);
 return rc;
};
