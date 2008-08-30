/*!

   @file

   @brief format command - frontend for format entry in fs utility dll

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

/* default fs to be used */
#define DEFAULT_FS_NAME "FAT"

/*!
  @todo add exception handling (ctrl+c/brak) and returning correct rc then
*/
int main (int argc, char* argv[], char* envp[])
{
  PSZ pszFSName=NULL;  /* New FS Name */
  PSZ pszOldFSName=NULL; /* Old FS Name */
  PSZ disk="   ";
  BOOL fAlreadyFormatted=TRUE;
  char *tmpString;
  int i;          /* counter and etc.. */
  APIRET rc;
  ULONG ulAvailableDisks;

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?')&& (argv[1][2]=='\0'))
 {
   cmd_ShowSystemMessage(cmd_MSG_FORMAT_HELP,0L);
   return NO_ERROR;
 };

 /* is there a disk specified on a command line? if not, print error info */
 for (i=1;i<argc;i++)
  if (argv[i][1]==':')
  {
    strncpy(disk,argv[i],2);
    break;
  };

  if (i==argc)
   {
     cmd_ShowSystemMessage(MSG_HPFS_SPECIFY_DRIVE_LETTER,0L);
     return cmd_ERROR_EXIT;
   };

  /* first, get the old file system (if available) */
  pszOldFSName=calloc(CCHMAXPATH,1);
  if (pszOldFSName==NULL) return cmd_ERROR_EXIT;

  if (all_QueryFSName(disk,pszOldFSName)!=0) fAlreadyFormatted=FALSE;

  pszFSName=calloc(CCHMAXPATH,1);
  if (pszFSName==NULL)
  {
    free(pszOldFSName);
    return cmd_ERROR_EXIT;
  };

 /* check for /FS: parameter */
 for (i=1;i<argc;i++)
   if ((argv[i][0]==PARAM_CHAR1) || (argv[i][0]==PARAM_CHAR2))
   {
     tmpString=argv[i]+1;
     if (strnicmp(tmpString,"fs:",3)==0)
     {
        tmpString=argv[i]+4;
        if (strlen(tmpString)>0)
           strncpy(pszFSName,tmpString,CCHMAXPATH-1);
        break;
     };
   };

 /* there was no fs specified, check for current volume fs */
 if (i==argc)
 {
   if (fAlreadyFormatted==TRUE)
   {
     strcpy(pszFSName,pszOldFSName);
   } else
   {
     tmpString=getenv("FORMAT");
     if ((tmpString!=NULL) && (strlen(tmpString)>0))
        strncpy(pszFSName,tmpString,CCHMAXPATH-1);
      else pszFSName=DEFAULT_FS_NAME;
   };
 };

 if ((fAlreadyFormatted) && (stricmp(pszOldFSName,"UNKNOWN")!=0))
   cmd_ShowSystemMessage(cmd_MSG_TYPE_OF_FS_IS,1L,"%s",pszOldFSName);

 /* free no-longer needed memory */
 free(pszOldFSName);

 rc=cmd_ExecFSEntry(pszFSName,cmd_FS_FORMAT,FALSE,argc,argv,envp);

 free(pszFSName);
 return rc;
};
