/*!
   $Id: eject.c,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $

   @file eject.c

   @brief eject command - removable media eject utility

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>

   @bug there is no help screen for this command in system msg file
*/

#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <osfree.h>

#include <string.h>

#include <cmd_shared.h> /* comand line tools' shared functions */

/* characters to be used as parameter specifiers */
#define PARAM_CHAR1 '/'
#define PARAM_CHAR2 '-'



/*!
  eject's main routine
*/
int main(int argc, char * argv[])
{
 PSZ pszDrive="C:\0";     /* drive name */
 ULONG ulAvailableDisks;  /* drives aviability bitmap */
 APIRET rc;               /* functions' return code */
 int i;
 PHFILE hFile;
  ULONG cbParamLenMax = 2;
  ULONG cbParamLen = 2;

#ifdef __WATCOMC__
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif
  struct
  {
    UCHAR command;
    UCHAR unit;
  }
  Params;
#ifdef __WATCOMC__
#pragma pack(pop)
#else
#pragma pack()
#endif

 /* get the current disk and available ones */
 if (cmd_QueryCurrentDisk(pszDrive,&ulAvailableDisks))
     return cmd_ERROR_EXIT;

 if (argc==2)
 {
    if (((argv[1][0]==PARAM_CHAR1) || (argv[1][0]==PARAM_CHAR2)) &&
       (argv[1][1]=='?') && (argv[1][2]=='\0'))
    {
    //  cmd_ShowSystemMessage(cmd_MSG_EJECT_HELP,0L);
      printf("Eject command help (todo)\n");
     return NO_ERROR;
    };

    if  ((strlen(argv[1])==2)&&(argv[1][1]==':')&&
         (toupper(argv[1][0])>='A')&&(toupper(argv[1][0])<='Z'))
     {
      pszDrive[0]=toupper(argv[1][0]);

      if (!((1<<pszDrive[0]-'A')&ulAvailableDisks))
      {
       cmd_ShowSystemMessage(MSG_INVALID_DRIVE,0L);
       return cmd_ERROR_EXIT;
      };
     } else
     {
       cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
       return 1; //! @todo: fix return code
     };

 } else
 if (argc>2)
 {
    cmd_ShowSystemMessage(MSG_BAD_SYNTAX,0L);
    return 1; //! @todo: fix return code
 };

  Params.command = (UCHAR) 2;
  Params.unit = (UCHAR) (pszDrive[0] - 'A');

  rc =
    DosDevIOCtl ((HFILE) - 1, IOCTL_DISK, DSK_UNLOCKEJECTMEDIA,
                 (PVOID) & Params, cbParamLenMax, &cbParamLen, NULL, 0, NULL);

 if (rc)
 {
  cmd_ShowSystemMessage(cmd_MSG_DEVICE_CMD_UNKNOWN,0);
  return 1;
 };

  return NO_ERROR;
};
