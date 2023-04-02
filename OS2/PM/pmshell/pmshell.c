/*  PM/WPS starter program
 *  @author valerius
 *  (c) osFree project, 2012 Mar 24
 */

#define  INCL_DOSMODULEMGR
#define  INCL_DOSPROCESS
#define  INCL_DOSSESMGR
#define  INCL_DOSFILEMGR
#define  INCL_DOSMISC
#define  INCL_WINWINDOWMGR
#define  INCL_DOSEXCEPTIONS
#include <os2.h>

#include <string.h>	// strcasecmp

#define ORD_SHLSTARTWORKPLACE   211
#define ORD_SHLEXCEPTIONHANDLER 212
#define ORD_SHLSAVEENV          213
#define ORD_MESSAGELOOPPROC     284

APIRET APIENTRY (*ShlSaveEnv)(PEXCEPTIONREGISTRATIONRECORD pERegRec, PVOID handler);
APIRET APIENTRY (*ShlStartWorkplace)(HAB hab, HMQ hmq);
APIRET APIENTRY (*MessageLoopProc)(HAB hab);
APIRET APIENTRY (*ShlExceptionHandler)(void);

APIRET APIENTRY SaveEnv (PEXCEPTIONREGISTRATIONRECORD pERegRec, PVOID handler)
{
  return 1;
}

APIRET APIENTRY StartWorkplace (HAB hab, HMQ hmq)
{
  STARTDATA sd;
  CHAR      objbuf[256];
  ULONG     ulSessID, ulPID;
  PSZ       pszValue;
  APIRET    rc;

  if (!(rc = DosScanEnv("RUNWORKPLACE", &pszValue)))
  {
    sd.Length    = sizeof(STARTDATA);
    sd.Related   = 0;
    sd.FgBg      = 0;
    sd.TraceOpt  = 0;
    sd.PgmTitle  = 0;
    sd.PgmName   = pszValue;
    sd.PgmInputs = 0;
    sd.TermQ     = 0;
    sd.Environment = 0;
    sd.InheritOpt  = 0;
    sd.SessionType = SSF_TYPE_WINDOWABLEVIO;
    sd.IconFile  = 0;
    sd.PgmHandle = 0;
    sd.PgmControl = 0;
    sd.InitXPos  = 0x14;
    sd.InitYPos  = 0x14;
    sd.InitXSize = 0xc8;
    sd.InitYSize = 0xc8;
    sd.Reserved  = 0;
    sd.ObjectBuffer  = objbuf;
    sd.ObjectBuffLen = (ULONG)sizeof(objbuf);

    rc = DosStartSession(&sd, &ulSessID, &ulPID);
  }

  return rc;
}

APIRET APIENTRY MsgLoop (HAB hab)
{
  QMSG   qmsg;
  HWND   hwnd;

  for (;;)
  {
    while (WinGetMsg(hab, &qmsg, 0, 0, 0))
      WinDispatchMsg(hab, &qmsg);

    hwnd = (HWND)qmsg.mp2;

    if (((HWND)qmsg.msg == WM_QUIT) && (WinIsWindow(hab, hwnd)))
      WinSendMsg(hwnd, (ULONG)WM_SYSCOMMAND, (MPARAM)SC_CLOSE, (MPARAM)CMDSRC_MENU);
  }

  return 0;
}

int getfunc (BOOL bNoWPS)
{
  CHAR    LoadError[256];
  HMODULE hmod;
  APIRET  rc;

  ShlExceptionHandler = 0;

  // Not required. Will be set in any case or not used
//  ShlStartWorkplace   = 0;
//  ShlSaveEnv          = 0;
//  MessageLoopProc     = 0;

  if (bNoWPS)
  {
    ShlSaveEnv = (void *)&SaveEnv;
    MessageLoopProc = (void *)&MsgLoop;
    ShlStartWorkplace = (void *)&StartWorkplace;
  }
  else
  {
    if (!(rc = DosLoadModule(LoadError, sizeof(LoadError), "PMWP", &hmod)))
    if (!(rc = DosQueryProcAddr(hmod, ORD_SHLSTARTWORKPLACE  , 0, (PFN *)&ShlStartWorkplace)))
    if (!(rc = DosQueryProcAddr(hmod, ORD_SHLEXCEPTIONHANDLER, 0, (PFN *)&ShlExceptionHandler)))
    if (!(rc = DosQueryProcAddr(hmod, ORD_SHLSAVEENV         , 0, (PFN *)&ShlSaveEnv)))
    if (!(rc = DosQueryProcAddr(hmod, ORD_MESSAGELOOPPROC,     0, (PFN *)&MessageLoopProc)))
      return rc;
  }

  return 0;
}

int main (int argc, char **argv)
{
  EXCEPTIONREGISTRATIONRECORD err;
  /* Env. variable name */
  CHAR   pszName[] = "WORKPLACE_PROCESS";
  PSZ    pszValue;
  HAB    hab;
  HMQ    hmq;
  QMSG   qmsg;
  PTIB   ptib;
  PPIB   ppib;
  APIRET rc;
  BOOL   bNoWPS = FALSE;

  DosGetInfoBlocks(&ptib, &ppib);
  ppib->pib_ultype = 3; // "morph" into a PM application

  if (argc > 1 && (!strcasecmp(argv[1], "/nowps") || 
      !strcasecmp(argv[1], "-nowps")))
    bNoWPS = TRUE;

  if (rc = getfunc(bNoWPS))
    return rc;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 100);

  if (ShlSaveEnv(&err, &ShlExceptionHandler) && 
      !(rc = DosScanEnv(pszName, &pszValue)))
    ShlStartWorkplace(hab, hmq);

  MessageLoopProc(hab);

  DosUnsetExceptionHandler(&err);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);

  return 0;
}
