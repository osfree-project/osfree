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
//#include <osfree.h>
//#include <os2.h>
#include <cmd_shared.h> /* command line tools' shared functions */

/* C standard library headers */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* this one is from cmd_FSEntry16.c */
//USHORT _Far16 _Pascal FSENTRY16(void *, short, char _far16 * _far16 *, char _far16 * _far16 *);

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

USHORT _Far16 _Pascal (_Far16 * func16)(USHORT, char _Far16 * _Far16 *, char _Far16 * _Far16 *);

APIRET cmd_ExecFSEntry(char *pszFSName,char *pszEntryName,BOOL fVerbose,
                       ULONG ulArgc, char **argv, char **envp)
{
  APIRET  rc;
  ULONG   ulType;
  int     i;
  ULONG   ulEnvpCount;
  PSZ     pszUtilDllName=NULL;
  HMODULE hUtilDllHandle;
  UCHAR   LoadError[256] =""; /* area for load failure information */
  PVOID   pvFSEntry;
  char    _Far16 * *abuf;
  char    _Far16 * *ebuf;
  char    _Far16 * _Far16 * newargv; /* 16bit pointers for argv and envp */
  char    _Far16 * _Far16 * newenvp;

/*!
  @todo add exception handling; failure of execution fs entry point can blow
        out whole program that calls cmd_ExecFSEntry
  @todo possibly check is there a _System entry (name lowercase with only first
        letter upper, eg.: Chkdsk, Sys, Format), which would be 32bit, call it
        if exist, else process as normal...
  @todo fix return codes
*/

  //printf("pszFSName=%s, pszEntryName=%s, fVerbose=%lu, argc=%lu\n",
  //       pszFSName, pszEntryName, fVerbose, ulArgc);

  //for (i = 0; i < ulArgc; i++)
  //  printf("argv[%lu]=%s\n", i, argv[i]);

#ifdef __386__
  pszUtilDllName=(PSZ)calloc(strlen(pszFSName)+1,1);
#else
  pszUtilDllName=(PSZ)calloc(_fstrlen(pszFSName)+1,1);
#endif

  if (pszUtilDllName==NULL) return cmd_ERROR_EXIT;

  /* create utility-dll module name */
#ifdef __386__
  strcpy(pszUtilDllName,"U");
  strcat(pszUtilDllName, pszFSName);
#else
  _fstrcpy(pszUtilDllName,"U");
  _fstrcat(pszUtilDllName, pszFSName);
#endif

  rc=DosLoadModule(LoadError,sizeof(LoadError), ////// ERROR_NOT_ENOUGH_MEMORY == 8
                   pszUtilDllName,&hUtilDllHandle);

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
#ifdef __386__
   free(pszUtilDllName);
#else
   _ffree(pszUtilDllName);
#endif
   return cmd_ERROR_EXIT;
  };

  /* get requested entry procedure address */
#ifdef __386__
  rc= DosQueryProcAddr(hUtilDllHandle,0L,
                       strupr(pszEntryName),(PFN*)&pvFSEntry);
#else
  rc= DosGetProcAddr(hUtilDllHandle, strupr(pszEntryName), (PFN*)&pvFSEntry);
#endif

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
#ifdef __386__
   free(pszUtilDllName);
#else
   _ffree(pszUtilDllName);
#endif
   return cmd_ERROR_EXIT;
  };

  /* this was only for debug purposes ;) */
  //DosQueryProcType(hUtilDllHandle,0L,strupr(pszEntryName),&ulType);
  //if (ulType==0) printf("\n 16bit Proc!!\n"); else printf("\n 32bit Proc\n");

  /* unfortunatelly we need to convert current argv and envp, to 16bit
     pointers */
  abuf = malloc(ulArgc * sizeof(char _Far16 *));

  for (i = 0; i < ulArgc; i++)
    abuf[i] = (char _Far16 *)argv[i];

  newargv = (char _Far16 * _Far16 *)abuf;

  /* get number of envs */
  for (ulEnvpCount = 0; environ[ulEnvpCount]; ulEnvpCount++) ;

  ebuf = malloc((ulEnvpCount + 1) * sizeof(char _Far16 *));

  for (i = 0; i < ulEnvpCount; i++)
    ebuf[i] = (char  _Far16 *)environ[i];

  ebuf[ulEnvpCount] = NULL;

  newenvp = (char _Far16 * _Far16 *)ebuf;

  if (fVerbose)
     cmd_ShowSystemMessage(cmd_MSG_FSUTIL_HAS_STARTED,1L,"%s",pszFSName);

  func16 = pvFSEntry;

  rc = func16(ulArgc, newargv, newenvp);

  DosFreeModule(hUtilDllHandle);
#ifdef __386__
  free(pszUtilDllName);
#else
  _ffree(pszUtilDllName);
#endif
  free(ebuf);
  free(abuf);

  return rc;
};
