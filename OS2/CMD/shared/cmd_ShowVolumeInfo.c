/*!
   $Id: cmd_ShowVolumeInfo.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $

   @file cmd_showvolumeinfo.c

   @brief utility function displaying volume label and serial number
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSERRORS
#define INCL_DOSMISC
//#include <osfree.h>
//#include <os2.h>

/* C standard library headers */
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <cmd_shared.h> /* comand line tools' shared functions and defines */

/*!
  Queries and displays volume serial and label information

  @param pszDrive         disk name (eg.: "d:") info about, we want to display
  @param fUseSemicolon    if true, semicolon is used as serial number
                          separator (A43D:C45S) /vol.exe style/ otherwise pause
                          (?dunno correct name) is used (A43D-C45S)
                          /label.exe style/

  @return
        - 0 - upon successfull completition
        - rc from DosQueryFSInfo on error
*/
APIRET cmd_ShowVolumeInfo(PSZ pszDrive,BOOL fUseSemicolon)
{
 APIRET rc;              /* return code*/
 FSINFO fsiBuffer;       /* file system information buffer */
 BOOL fNoLabel=FALSE;    /* is there a label on volume */
 PSZ pszDisk=pszDrive;   /* volume we check */
 CHAR chDisk;
 PSZ serial="         "; /* place for serial number */

 /* first check is the parameter correct */
#ifdef __386__
 if ((strlen(pszDrive)!=2)||(pszDrive[2]==':'))
#else
 if ((_fstrlen(pszDrive)!=2)||(pszDrive[2]==':'))
#endif
 {
  printf("\n");
  printf(all_GetSystemErrorMessage(ERROR_INVALID_DRIVE));
  return ERROR_INVALID_DRIVE;
 };

 chDisk=pszDisk[0];

 /* ask for label and serial */
#ifdef __386__
 rc= DosQueryFSInfo((toupper(pszDrive[0])-'A'+1),FSIL_VOLSER,
                   (PVOID)&fsiBuffer,sizeof(fsiBuffer));
#else
 rc= DosQFSInfo((toupper(pszDrive[0])-'A'+1),FSIL_VOLSER,
                   (PVOID)&fsiBuffer,sizeof(fsiBuffer));
#endif

 if (rc)
 {
   if (rc==ERROR_NO_VOLUME_LABEL) fNoLabel=TRUE;
   else
   {
     printf("\n");
     printf(all_GetSystemErrorMessage(rc));
     return rc;
   };
 };

 if ((fNoLabel)||(fsiBuffer.vol.cch==0))
  /* there is no label */
     cmd_ShowSystemMessage(cmd_MSG_VOLUME_HAS_NO_LABEL,1L,"%c",chDisk);
   else
   /* show the label */
   cmd_ShowSystemMessage(cmd_MSG_VOLUME_LABEL_IS,2L,"%c",chDisk,
                                                "%s",fsiBuffer.vol.szVolLabel);

  /*! @todo check if the date/time (this is serial number) are equal to 0,
      if this happens do not show serial */

  /* serial number in fact, consists of time:date of volume creation,
     represented in hex format */
  cmd_ShowSystemMessage(cmd_MSG_VOLUME_SERIAL_IS,1L,
  "%4.4X%c%4.4X",
  fsiBuffer.fdateCreation,(fUseSemicolon ? ':' : '-'),fsiBuffer.ftimeCreation);

 return NO_ERROR;
};
