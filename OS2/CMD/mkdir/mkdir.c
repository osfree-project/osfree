/*!

   @file

   @brief mkdir command - makes a directory

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @todo: add support of 4OS2 extensions
*/

#define INCL_DOSERRORS
#include <osfree.h>

#include <cmd_shared.h> /* comand line tools' shared functions */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <cmd_shared.h> /* comand line tools' shared functions */
/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

/*!
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main(int argc, char * argv[])
{
 APIRET rc;               /* functions' return code */
 BOOL bErrorHappened=FALSE;
 int i;

 if (argc==1)
 {
    cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return 1; //! @todo: fix return code
 };

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?')&& (argv[1][2]=='\0'))
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_MKDIR_HELP,0L);
  return NO_ERROR;
 };

 for (i=1;i<argc;i++)
 {
  rc=DosCreateDir(argv[i],NULL);
  if (rc)
  {
    bErrorHappened=TRUE;
    switch (rc)
    {
     case MSG_PATH_NOT_FOUND:
       cmd_ShowSystemMessage(MSG_CANNOT_MAKE,0);
       break;
     case MSG_ACCESS_DENIED:
       cmd_ShowSystemMessage(cmd_MSG_ALREADYEXIST,1,"%s",argv[i]);
       break;
     default:
     cmd_ShowSystemMessage(rc,0);
    };
  };
 };

 if (bErrorHappened) return 1;

 return NO_ERROR;
};

