/*!
   $Id: ver.c,v 1.1 2003/12/12 18:18:48 prokushev Exp $

   @file ver.c

   @brief ver command - displays system version number

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

*/

#define INCL_DOSERRORS
#define INCL_DOSMISC
#include <osfree.h>

#include <cmd_shared.h> /* comand line tools' shared functions */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

/*!
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
  @bug well, command works but much in current OS/2 style, and, returns
       revision as 0! that is beacause
       DosQuerySysInfo's bug that does not return correct revision
*/
int main(int argc, char * argv[])
{
  BOOL bShowRevision=FALSE;
  ULONG aulValues[3];
  APIRET rc;

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?') && (argv[1][2]=='\0') )
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_VER_HELP,0L);
  return NO_ERROR;
 };

 switch (argc)
 {
   case 1: /* no arguments, skip */
    break;
   case 2:
    if (strlen(argv[1])==2)
    {
      if ( ((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
           ((argv[1][1]=='r') || (argv[1][1]=='R'))  )
      {
       bShowRevision=TRUE;
       break;
      };
    };
   default: /* all the rest, means wrong syntax */
    cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return 1; //! @todo: fix return code
 };

 rc= DosQuerySysInfo(QSV_VERSION_MAJOR, QSV_VERSION_REVISION, &aulValues, sizeof(aulValues));

 if (rc) return 1; //!@todo: fix return code

 cmd_ShowSystemMessage(cmd_MSG_OSVERSION,2,"%d",aulValues[1]/10,"%d",(aulValues[1]%10)*10);
 printf("\n");
 if (bShowRevision)
 cmd_ShowSystemMessage(cmd_MSG_OSREVISION,2,"%d",aulValues[2]/10,"%d",(aulValues[2]%10)*10);
 printf("\n");
 return NO_ERROR;
};

