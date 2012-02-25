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
#include <os2.h>
#include <cmd_shared.h> /* command line tools' shared functions */

/* C standard library headers */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* this one is from cmd_FSEntry16.c */
USHORT _far16 _pascal FSENTRY16(void *, short, char _far16 * _far16 *, char _far16 * _far16 *);

ULONG DosFlatToSel(ULONG);

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

APIRET16 APIENTRY16 (*func16)(USHORT, char * _Seg16 * _Seg16, char * _Seg16 * _Seg16);

APIRET cmd_ExecFSEntry(char *pszFSName,char *pszEntryName,BOOL fVerbose,
                       ULONG ulArgc, char **argv, char **envp)
{
  APIRET rc;
  ULONG ulType;
  int i;
  ULONG ulEnvpCount;
  PSZ pszUtilDllName=NULL;
  HMODULE hUtilDllHandle;
  UCHAR LoadError[256] =""; /* area for load failure information */
  char    **_Seg16 buf;
  char    * _Seg16 * _Seg16 newargv; /* 16bit pointers for argv and envp */
  ULONG   *pulnewargv = (PULONG)&newargv;
  char    * _Seg16 * _Seg16 newenvp;
  void    * _Seg16 pWork16;
  ULONG   *pulW16 = (PULONG)&pWork16;
  PFN     func;
  ULONG p;

/*!
  @todo add exception handling; failure of execution fs entry point can blow
        out whole program that calls cmd_ExecFSEntry
  @todo possibly check is there a _System entry (name lowercase with only first
        letter upper, eg.: Chkdsk, Sys, Format), which would be 32bit, call it
        if exist, else process as normal...
  @todo fix return codes
*/

  printf("pszFSName=%s, pszEntryName=%s, fVerbose=%lu, argc=%lu\n",
         pszFSName, pszEntryName, fVerbose, ulArgc);

  for (i = 0; i < ulArgc; i++)
    printf("argv[%lu]=%s\n", i, argv[i]);

  pszUtilDllName=(PSZ)calloc(strlen(pszFSName)+1,1);
  if (pszUtilDllName==NULL) return cmd_ERROR_EXIT;

  /* create utility-dll module name */
  strcpy(pszUtilDllName,"U");
  strcat(pszUtilDllName, pszFSName);

  rc=DosLoadModule(LoadError,sizeof(LoadError),
                   pszUtilDllName,&hUtilDllHandle);

  printf("0\n");

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
   free(pszUtilDllName);
   return cmd_ERROR_EXIT;
  };

  /* get requested entry procedure address */
  rc= DosQueryProcAddr(hUtilDllHandle,0L,
                       strupr(pszEntryName),(PFN*)&func16);

  if (rc!=NO_ERROR)
  {
   printf(all_GetSystemErrorMessage(rc));
   free(pszUtilDllName);
   return cmd_ERROR_EXIT;
  };

  //printf("pvFSEntry=0x%lx\n", pvFSEntry);
  printf("1\n");

//  func16 = (PVOID) (ULONG) (PVOID16) func16;

  /* this was only for debug purposes ;) */
  DosQueryProcType(hUtilDllHandle,0L,strupr(pszEntryName),&ulType);

  if (ulType==0) printf("\n 16bit Proc!!\n"); else printf("\n 32bit Proc\n");
 /* */

  /* unfortunatelly we need to convert current argv and envp, to 16bit
     pointers */
  buf = malloc(ulArgc * sizeof(char * _Seg16));

  printf("2\n");

  for (i = 0; i < ulArgc; i++)
  {
    p = (ULONG)DosFlatToSel((ULONG)argv[i]);
    buf[i] = (char * _Seg16)p; //MAKE16P(SELECTOROF(argv[i]),OFFSETOF(argv[i]));
  }
    /* get number of envs */
  for (ulEnvpCount = 0; environ[ulEnvpCount]; ulEnvpCount++) ;

  //printf("ulEnvpCount = %d\n", ulEnvpCount);

  //newenvp = malloc((ulEnvpCount + 1)*sizeof(char _far16 *));

  //for (i = 0; i < ulEnvpCount; i++)
  //{
  //  p = (ULONG)DosFlatToSel((ULONG)environ[i]);
  //  newenvp[i] = (char  _far16 *)p; // MAKE16P(SELECTOROF(environ[i]),OFFSETOF(environ[i]));
    //printf("newenvp[%d] = 0x%x\n", i, newenvp[i]);
  //}

  //newenvp[ulEnvpCount] = NULL;

  printf("3\n");

  if (fVerbose)
     cmd_ShowSystemMessage(cmd_MSG_FSUTIL_HAS_STARTED,1L,"%s",pszFSName);

  //func16 = pvFSEntry;

  printf("4\n");

  newargv = (char * _Seg16 * _Seg16)DosFlatToSel((ULONG)buf); //MAKE16P(SELECTOROF(newargv), OFFSETOF(newargv));
  //newenvp = (char  _far16 * _far16 *)DosFlatToSel((ULONG)newenvp); //MAKE16P(SELECTOROF(newenvp), OFFSETOF(newenvp));

  //rc=func16((short)ulArgc, newargv, newenvp);
  //printf("newargv = 0x%x, newenvp = 0x%x\n", newargv, newenvp);
  printf("newargv = 0x%lx\n", newargv);

  // KLUDGE for WATCOM C problems in converting a pointer to a 16-bit segment.
  // We have to convert to a long first, then store it, otherwise Watcom C
  // converts it to 16-bit and immediately back to 32-bit

  //*pulW16 = ( selLocalSeg << 16 );
  //gpLIS = (PLINFOSEG)(void * _Seg16)pWork16;

  //newargv = (char * _Seg16)(PULONG)

  func16 = (void * _Seg16)DosSelToFlat(func16);  

  printf("func16=%lx\n", (ULONG)func16);

  rc=func16((short)ulArgc, newargv, 0);

//  rc=FSENTRY16(pvFSEntry, (short)ulArgc, newargv, newenvp);
//  rc=(pvFSEntry)((short)ulArgc, newargv, newenvp);

  printf("5\n");

  DosFreeModule(hUtilDllHandle);
  free(pszUtilDllName);

  return rc;
};
