/*!
   $Id: vol.c,v 1.1 2003/12/12 18:18:48 prokushev Exp $

   @file vol.c

   @brief vol command - displays disk volume label and serial number if exists

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


/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'

/*!
  @bug  ? original checks whole command line first and if finds a swith (/)
        reports sys1003 (syntax incorrect), should we do this also?
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main(int argc, char * argv[])
{
 PSZ pszDrive="C:\0";     /* drive name */
 ULONG ulAvailableDisks;  /* drives aviability bitmap */
 APIRET rc;               /* functions' return code */
 int i;

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?') && (argv[1][2]=='\0'))
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_VOL_HELP,0L);
  return NO_ERROR;
 };

 /* get the current disk and available ones */
 if (cmd_QueryCurrentDisk(pszDrive,&ulAvailableDisks))
     return cmd_ERROR_EXIT;

 if (argc==1) /* use current disk */
 {
   cmd_ShowVolumeInfo(pszDrive,TRUE);
 } else /* there are disks specified */
 {
   for (i=1;i<argc;i++)
   {
     rc=cmd_ShowVolumeInfo(argv[i],TRUE);

     /* this mysterious argc!=1 is to work exacly as oryginal */
     if ((rc) && (argc!=1))
     cmd_ShowSystemMessage(cmd_MSG_ERROR_WHILE_PROCESSING,1L,"%s",argv[i]);
   };
 };

  return NO_ERROR;
};

