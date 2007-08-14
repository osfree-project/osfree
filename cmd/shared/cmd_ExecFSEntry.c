/*!
   $Id: cmd_ExecFSEntry.c,v 1.1.1.1 2003/10/04 08:24:20 prokushev Exp $

   @file cmd_execfsentry.c

   @brief utility function to execute entry from fs utility dll
   shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <osFree.h>
#include <cmd_shared.h> /* command line tools' shared functions */

/* C standard library headers */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* this one is from cmd_FSEntry16.c */
USHORT _far16 _pascal FSENTRY16(void *, short, char _far16 * _far16 *, char _far16 * _far16 *);


/*!
  Executes an entry from FS utility dll

  @param pszFSName    File system name (eg.: "FAT32")
  @param pszEntryName Entry function name. One should use cmd_FS_* defines
                      (defined in cmd_execfsentry.h file)
  @param fVerbose     "Verbose execution" - if true, prints message that entry
                      has been started
  @param ulArgc       Argument count (should be the same as argc from main())
  @param argv         Argument list
  @param envp         Environment list

  @return
         - if entry ran succesfully - rc from fs entry, otherwise ....
         not sure yet

  @bug due Watcom's bug/feature we need to use environ here instead of envp...
*/
APIRET cmd_ExecFSEntry(PSZ pszFSName,PSZ pszEntryName,BOOL fVerbose,
                       ULONG ulArgc, PSZ argv[],PSZ envp[])
{
  APIRET rc;
  ULONG ulType;
  int i;
  ULONG ulEnvpCount;
  VOID *pvFSEntry;
  PSZ pszUtilDllName=NULL;
  HMODULE hUtilDllHandle;
  UCHAR LoadError[256] =""; /* area for load failure information */
  char _far16 * _far16 * newargv; /* 16bit pointers for argv and envp */
  char _far16 * _far16 * newenvp;


/*!
  @todo add exception handling; failure of execution fs entry point can blow
        out whole program that calls cmd_ExecFSEntry
  @todo possibly check is there a _System entry (name lowercase with only first
        letter upper, eg.: Chkdsk, Sys, Format), which would be 32bit, call it
        if exist, else process as normal...
  @todo fix return codes
*/


  pszUtilDllName=(PSZ)calloc(strlen(pszFSName)+1,1);
  if (pszUtilDllName==NULL) return cmd_ERROR_EXIT;

  /* create utility-dll module name */
  strcpy(pszUtilDllName,"U");
  strcat(pszUtilDllName, pszFSName);

  rc=DosLoadModule(LoadError,sizeof(LoadError),
                   pszUtilDllName,&hUtilDllHandle);

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
   free(pszUtilDllName);
   return cmd_ERROR_EXIT;
  };

  /* get requested entry procedure address */
  rc= DosQueryProcAddr(hUtilDllHandle,0L,
                       strupr(pszEntryName),(PFN*)&pvFSEntry);

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
   free(pszUtilDllName);
   return cmd_ERROR_EXIT;
  };

  /* this was only for debug purposes ;)
  DosQueryProcType(hUtilDllHandle,0L,strupr(pszEntryName),&ulType);

  if (ulType==0) printf("\n 16bit Proc!!\n"); else printf("\n 32bit Proc\n");
  */

  /* unfortunatelly we need to convert current argv and envp, to 16bit
     pointers */
  newargv= (char _far16 * _far16 *)malloc(ulArgc*sizeof(char _far16*));

  for (i=0;i<ulArgc;i++)
    newargv[i]=(char far *)argv[i];

    /* get number of envs */
  for (ulEnvpCount=0;environ[ulEnvpCount];ulEnvpCount++);

  newenvp= (char _far16 * _far16 *)malloc((ulEnvpCount+1)*sizeof(char _far16*));

  for (i=0;i<ulEnvpCount;i++)
    newenvp[i]=(char far *)environ[i];

  newenvp[ulEnvpCount]=NULL;

  if (fVerbose)
     cmd_ShowSystemMessage(cmd_MSG_FSUTIL_HAS_STARTED,1L,"%s",pszFSName);

  rc=FSENTRY16(pvFSEntry, (short)ulArgc, newargv, newenvp);

  DosFreeModule(hUtilDllHandle);
  free(pszUtilDllName);

  return rc;
};
