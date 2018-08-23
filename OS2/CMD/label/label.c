/*!

   @file

   @brief label command - creates,changes or erases volume identification label
   on disk

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
   @todo: replaced gets by cmd_getLine (to be implemented first)
*/

#define INCL_DOSERRORS
#include <osfree.h>

#include <cmd_shared.h> /* comand line tools' shared functions */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


/* local functions */
BOOL isVolumeLabelValid(PSZ szVolume);
void getNewLabel(PSZ *szLabel);

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'


int main(int argc, char * argv[])
{
 PSZ pszDrive="C:\0";     /* drive name */
 ULONG ulAvailableDisks;  /* drives aviability bitmap */
 APIRET rc;               /* functions' return code */
 PSZ szArg=NULL;          /* argument pointer */
 BOOL fLabelOnCmd=FALSE;  /* is label specified on cmdline */
 VOLUMELABEL fsiLabelBuf; /* buffer for volume label (for DosSetFSInfo) */

 /* check if user wants info from us (/? or -?) */
 if (argc>1)
   if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
      (argv[1][1]=='?')&& (argv[1][2]=='\0'))
 {
  /* show help message */
  cmd_ShowSystemMessage(cmd_MSG_LABEL_HELP,0L);
  return NO_ERROR;
 };

 /* get the current disk and available ones */
 if (cmd_QueryCurrentDisk(pszDrive,&ulAvailableDisks))
     return cmd_ERROR_EXIT;

 /* we take only one argument, all the rest is ignored */
 if (argc>1)
 {
   szArg=argv[1];
   fLabelOnCmd=TRUE; /* we don't know is there a label, so assume it's there */

   /* was there a drive specified? */
   if (szArg[1]==':')
   {
     /* is such disk available */
     if ((toupper(szArg[0])>='A')&&(toupper(szArg[0])<='Z')&&
         ((1<<toupper(szArg[0])-'A')&ulAvailableDisks))
     {
       pszDrive[0]=toupper(szArg[0]);
       if (strlen(szArg)>2)
        szArg+=2; /* skip drive letter and semicolon */
       else
         {
          szArg=NULL;
          fLabelOnCmd=FALSE; /* there is no label on cmd line... */
         };
     } else
     {
      cmd_ShowSystemMessage(MSG_INVALID_DRIVE,0L);
      return cmd_ERROR_EXIT;
     };
   }; /* END: if (argv[1][1]==':') */
 }; /* END: if (argc>1) */

 /* display volume label and serial, if needed, and ask for new */
 if (!fLabelOnCmd)
 {
   if (cmd_ShowVolumeInfo(pszDrive,FALSE))
    return cmd_ERROR_EXIT;

   /* get the new label */
//   szArg=calloc(1,14);
   getNewLabel(&szArg);

 } else
 {
   PSZ tmpArg=calloc(1,15);
   strncpy(tmpArg,szArg,14);
   szArg=tmpArg;
 };

 fLabelOnCmd=FALSE; /* not it becomes other flag:
                       true - if label was truncated */

 if (strlen(szArg)==0) /* no label change */
 {
  free(szArg);
  return 0;
 };

 /* truncate label if it's too long */
 if (strlen(szArg)>11)
 {
  fLabelOnCmd=TRUE;
  szArg[11]='\0';
 };


 while (!isVolumeLabelValid(szArg))
 {
  cmd_ShowSystemMessage(MSG_INVALID_NAME,0L),
   free(szArg);
   getNewLabel(&szArg);
   fLabelOnCmd=FALSE; /* new label is not truncated (yet) */

   if (strlen(szArg)==0) /* no label change */
   {
    free(szArg);
    return 0;
   };

   /* truncate label if it's too long */
   if (strlen(szArg)>11)
   {
    fLabelOnCmd=TRUE;
    szArg[11]='\0';
   };
 };


 /* fill buffer for DosSetFSInfo, and set the label */
 strcpy(fsiLabelBuf.szVolLabel,szArg);
 fsiLabelBuf.cch=strlen(szArg);

 free(szArg);

 rc = DosSetFSInfo((toupper(pszDrive[0])-'A'+1),FSIL_VOLSER,&fsiLabelBuf,
      sizeof(VOLUMELABEL));
 if (rc)
 {
  printf(all_GetSystemErrorMessage(rc));
  return cmd_ERROR_EXIT;
 };

 if (fLabelOnCmd) /* was the label truncated? */
   cmd_ShowSystemMessage(MSG_VOLUME_TOO_LONG,0L);

  return NO_ERROR;
};

/*!
  Checks is given volume is valid

  @param szVolume    volume name to be checked

  @retval TRUE - if volume name is valid
  @retval FALSE - if volume name is not valid
*/
BOOL isVolumeLabelValid(PSZ szVolume)
{
  static char forbidden_chars[] = {
                                 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                                 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
                                 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                 '\\', '/', ':', '*', '?', '"', '<', '>', '|', ',',
                                 '+', '=', '[', ']', ';','.',
                                 '\0'
                                };
   if (strpbrk(szVolume, (char *)forbidden_chars) > 0)
    return FALSE;

  return TRUE;
};

/*!
  gets new label from user

  @todo replace it with something like cmd_readline
  @bug  doesn't work like supposed
*/
void getNewLabel(PSZ *szLabel)
{
  char *newLabel=calloc(1,300);
  cmd_ShowSystemMessage(cmd_MSG_ENTER_NEW_LABEL,0L);
  flushall();

  gets(newLabel);
  *szLabel=newLabel;
};
