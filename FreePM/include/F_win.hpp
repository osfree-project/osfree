/*
 $Id: F_win.hpp,v 1.2 2003/07/14 21:21:18 evgen2 Exp $
*/
/* F_win.hpp */
#ifndef FREEPM_WIN
  #define FREEPM_WIN

#include "F_def.hpp"
#include "F_messages.hpp"
#ifdef __cplusplus
      extern "C" {
#endif

/*** include everything if requested ***************************************/
#ifdef F_INCL_WIN
   #define F_INCL_WINWINDOWMGR
   #define F_INCL_WINMESSAGEMGR
   #define F_INCL_WININPUT
   #define F_INCL_WINDIALOGS
   #define F_INCL_WINSTATICS
   #define F_INCL_WINBUTTONS
   #define F_INCL_WINENTRYFIELDS
   #define F_INCL_WINMLE
   #define F_INCL_WINLISTBOXES
   #define F_INCL_WINMENUS
   #define F_INCL_WINSCROLLBARS
   #define F_INCL_WINFRAMEMGR
   #define F_INCL_WINFRAMECTLS
   #define F_INCL_WINRECTANGLES
   #define F_INCL_WINSYS
   #define F_INCL_WINTIMER
   #define F_INCL_WINACCELERATORS
   #define F_INCL_WINTRACKRECT
   #define F_INCL_WINCLIPBOARD
   #define F_INCL_WINCURSORS
   #define F_INCL_WINPOINTERS
   #define F_INCL_WINHOOKS
   #define F_INCL_WINSWITCHLIST
   #define F_INCL_WINPROGRAMLIST
   #define F_INCL_WINSHELLDATA
   #define F_INCL_WINCOUNTRY
   #define F_INCL_WINHEAP
   #define F_INCL_WINATOM
   #define F_INCL_WINERRORS
   #define F_INCL_WINDDE
   #define F_INCL_WINHELP
   #define F_INCL_WINSEI
   #define F_INCL_WINLOAD
   #define F_INCL_WINTYPES
   #define F_INCL_WINDESKTOP
   #define F_INCL_WINPALETTE
   #define F_INCL_WINTHUNKAPI
   #define F_INCL_WINSTDDLGS
   #define F_INCL_WINBIDI
#else /* F_INCL_WIN */
   /*** include resource compiler required sections ************************/
   #ifdef RC_INVOKED
      #define F_INCL_WININPUT
      #define F_INCL_WINDIALOGS
      #define F_INCL_WINSTATICS
      #define F_INCL_WINBUTTONS
      #define F_INCL_WINENTRYFIELDS
      #define F_INCL_WINLISTBOXES
      #define F_INCL_WINMENUS
      #define F_INCL_WINSCROLLBARS
      #define F_INCL_WINFRAMEMGR
      #define F_INCL_WINFRAMECTLS
      #define F_INCL_WINACCELERATORS
      #define F_INCL_WINPOINTERS
      #define F_INCL_WINMESSAGEMGR
      #define F_INCL_WINMLE
      #define F_INCL_WINHELP
      #define F_INCL_WINSTDDLGS
      #define F_INCL_WINSYS
   #endif /* RC_INVOKED */
#endif /* F_INCL_WIN */


#ifndef __VACPASCAL__
   #define __VACPASCAL__

   #define PASCAL   _Pascal
   #define pascal   _Pascal
#endif

#ifndef __VACCDECL__
   #define __VACCDECL__

   #define CDECL    __cdecl
   #define cdecl    __cdecl
   #define _cdecl   __cdecl
#endif

#ifndef __VACSTDCALL__
  #define __VACSTDCALL__

  #define  _stdcall  _Stdcall
#endif

#ifndef __VACSYSCALL__
  #define __VACSYSCALL__

  #define _syscall  _System
#endif

#define CENTRY CDECL                 // to declare misc. C-based interfaces
#define EXPENTRY    _System          // to declare exported routines

/*** WinInitialize/WinTerminate Interface declarations ******************/
#ifndef  APIENTRY
   #define APIENTRY    _System
#endif

   HAB    APIENTRY /*F_*/ WinInitialize(ULONG flOptions);
   BOOL   APIENTRY /*F_*/ WinTerminate(HAB hab);

   /*** The following structure is used by the WinQueryQueueInfo() routine */
   typedef struct _MQINFO    /* mqi */
   {
      ULONG   cb;
      PID     pid;
      TID     tid;
      ULONG   cmsgs;
      PVOID   pReserved;
   } MQINFO;
   typedef MQINFO *PMQINFO;
/*
#ifndef _QMSG_
#define _QMSG_
	typedef struct _QMSG
	{
	  HWND	 hwnd;
	  ULONG	 msg;
	  MPARAM mp1;
	  MPARAM mp2;
	  ULONG	 time;
	  POINTL ptl;
	  ULONG	 reserved;
	} QMSG;
	
typedef QMSG *PQMSG;
#endif
*/
   MRESULT APIENTRY /*F_*/ WinSendMsg(HWND hwnd,
                               ULONG msg,
                               MPARAM mp1,
                               MPARAM mp2);

   HMQ     APIENTRY /*F_*/ WinCreateMsgQueue(HAB hab,
                                      LONG cmsg);

   BOOL    APIENTRY /*F_*/ WinDestroyMsgQueue(HMQ hmq);
   BOOL    APIENTRY /*F_*/ WinQueryQueueInfo(HMQ hmq,
                                      PMQINFO pmqi,
                                      ULONG cbCopy);

   #ifndef F_INCL_SAADEFS
      BOOL    APIENTRY /*F_*/ WinCancelShutdown(HMQ hmq,
                                         BOOL fCancelAlways);
   #endif /* F_INCL_SAADEFS */

   BOOL    APIENTRY /*F_*/ WinGetMsg(HAB hab,
                              PQMSG pqmsg,
                              HWND hwndFilter,
                              ULONG msgFilterFirst,
                              ULONG msgFilterLast);

   /*** WinPeekMsg() constants *********************************************/
   #define PM_REMOVE                  0x0001
   #define PM_NOREMOVE                0x0000

   BOOL    APIENTRY /*F_*/ WinPeekMsg(HAB hab,
                               PQMSG pqmsg,
                               HWND hwndFilter,
                               ULONG msgFilterFirst,
                               ULONG msgFilterLast,
                               ULONG fl);

   MRESULT APIENTRY /*F_*/ WinDispatchMsg(HAB hab,
                                   PQMSG pqmsg);

   BOOL    APIENTRY /*F_*/ WinPostMsg(HWND hwnd,
                               ULONG msg,
                               MPARAM mp1,
                               MPARAM mp2);

   BOOL    APIENTRY /*F_*/ WinRegisterUserMsg(HAB hab,
                                       ULONG msgid,
                                       LONG datatype1,
                                       LONG dir1,
                                       LONG datatype2,
                                       LONG dir2,
                                       LONG datatyper);



#ifdef F_INCL_WINERRORS
   #include "FreePM_err.hpp"

/*?? may be move Error def stuff to F_basedef.HPP ?? */
/*** Common Error definitions from os2def.h ****/

typedef ULONG ERRORID;  /* errid */
typedef ERRORID *PERRORID;

/* Combine severity and error code to produce ERRORID */
#define MAKEERRORID(sev, error) (ERRORID)(MAKEULONG((error), (sev)))

/* Extract error number from an errorid */
#define ERRORIDERROR(errid)            (LOUSHORT(errid))

/* Extract severity from an errorid */
#define ERRORIDSEV(errid)              (HIUSHORT(errid))

/* Severity codes */
#define SEVERITY_NOERROR                    0x0000
#define SEVERITY_WARNING                    0x0004
#define SEVERITY_ERROR                      0x0008
#define SEVERITY_SEVERE                     0x000C
#define SEVERITY_UNRECOVERABLE              0x0010

/* Base component error values */

#define WINERR_BASE     0x1000  /* Window Manager                  */
#define GPIERR_BASE     0x2000  /* Graphics Presentation Interface */
#define DEVERR_BASE     0x3000  /* Device Manager                  */
#define SPLERR_BASE     0x4000  /* Spooler                         */

/*** end of Common Error definitions from os2def.h ****/
#define FREPMERR_BASE   0x6000  /* FreePM errors                   */


   /* Get/Set Error Information Interface declarations */


/* Error-information structure. */

 typedef struct _ERRINFO {
   ULONG       cbFixedErrInfo;  /*  Length of fixed data to this structure. */
   ERRORID     idError;         /*  Error identity. */
   ULONG       cDetailLevel;    /*  Number of levels of detail. */
   ULONG       offaoffszMsg;    /*  Offset to the array of message offsets. */
   ULONG       offBinaryData;   /*  Offset to the binary data. */
 } ERRINFO;

 typedef ERRINFO *PERRINFO;

   ERRORID     APIENTRY    F_WinGetLastError(HAB hab);
   ERRORID     APIENTRY    F_WinGetLastError2(void);  /* FreePM ext */

   PERRINFO    APIENTRY    F_WinGetErrorInfo(HAB hab);
   BOOL        APIENTRY    F_WinFreeErrorInfo(PERRINFO perrinfo);

#endif  /* F_INCL_WINERRORS */
/***************************************************************************/
/***        General Window Management types, constants and macros        ***/
/***************************************************************************/

/*** Macros to make an MPARAM from standard types. *************************/

#define MPVOID                     ((MPARAM)0L)
#define MPFROMP(p)                 ((MPARAM)((ULONG)(p)))
#define MPFROMHWND(hwnd)           ((MPARAM)(HWND)(hwnd))
#define MPFROMCHAR(ch)             ((MPARAM)(USHORT)(ch))
#define MPFROMSHORT(s)             ((MPARAM)(USHORT)(s))
#define MPFROM2SHORT(s1, s2)       ((MPARAM)MAKELONG(s1, s2))
#define MPFROMSH2CH(s, uch1, uch2) ((MPARAM)MAKELONG(s, MAKESHORT(uch1, uch2)))
#define MPFROMLONG(l)              ((MPARAM)(ULONG)(l))

/*** Macros to extract standard types from an MPARAM ***********************/

#define PVOIDFROMMP(mp)            ((VOID *)(mp))
#define HWNDFROMMP(mp)             ((HWND)(mp))
#define CHAR1FROMMP(mp)            ((UCHAR)((ULONG)mp))
#define CHAR2FROMMP(mp)            ((UCHAR)((ULONG)mp >> 8))
#define CHAR3FROMMP(mp)            ((UCHAR)((ULONG)mp >> 16))
#define CHAR4FROMMP(mp)            ((UCHAR)((ULONG)mp >> 24))
#define SHORT1FROMMP(mp)           ((USHORT)(ULONG)(mp))
#define SHORT2FROMMP(mp)           ((USHORT)((ULONG)mp >> 16))
#define LONGFROMMP(mp)             ((ULONG)(mp))

/*** Macros to make an MRESULT from standard types. ************************/

#define MRFROMP(p)                 ((MRESULT)(VOID *)(p))
#define MRFROMSHORT(s)             ((MRESULT)(USHORT)(s))
#define MRFROM2SHORT(s1, s2)       ((MRESULT)MAKELONG(s1, s2))
#define MRFROMLONG(l)              ((MRESULT)(ULONG)(l))

/*** Macros to extract standard types from an MRESULT **********************/

#define PVOIDFROMMR(mr)            ((VOID *)(mr))
#define SHORT1FROMMR(mr)           ((USHORT)((ULONG)mr))
#define SHORT2FROMMR(mr)           ((USHORT)((ULONG)mr >> 16))
#define LONGFROMMR(mr)             ((ULONG)(mr))


/****************************************************************************
* This is the standard function definition for window procedures.
* Typically they are names like "XxxxxxxxWndProc", where the prefix
* "Xxxxxxxxx" is replaced by some name descriptive of the window procedure
* being declared.  Window procedures must be EXPORTED in the definitions
* file used by the linker.
*
* MRESULT EXPENTRY MyclassWndProc(HWND hwnd,   * window handle        *
*                                 ULONG msg,   * message number       *
*                                 MPARAM mp1,  * 1st (packed) parms   *
*                                 MPARAM mp2); * 2nd (packed) parms   *
*
****************************************************************************/

/*** Pointer to a window procedure function ********************************/
typedef MRESULT (EXPENTRY FNWP)(HWND, ULONG, MPARAM, MPARAM);
typedef FNWP *PFNWP;

#include "FreePM_winConstants.hpp"
/*
Standard Window Classes  WC_xxx
Standard Window Styles   WS_xxx
Dialog manager styles    WS_GROUP, WS_TABSTOP,WS_MULTISELECT
Class styles             CS_xxx
*/

/***************************************************************************/
/*** Window Manager Subsection part 1 **************************************/
/***************************************************************************/


BOOL    APIENTRY /*F_*/ WinRegisterClass(HAB hab,
                                  PCSZ  pszClassName,
                                  PFNWP pfnWndProc,
                                  ULONG flStyle,
                                  ULONG cbWindowData);

   MRESULT APIENTRY F_WinDefWindowProc(HWND hwnd,
                                     ULONG msg,
                                     MPARAM mp1,
                                     MPARAM mp2);

   BOOL    APIENTRY /*F_*/ WinDestroyWindow(HWND hwnd);

   BOOL    APIENTRY F_WinShowWindow(HWND hwnd,
                                  BOOL fShow);

   BOOL    APIENTRY F_WinQueryWindowRect(HWND hwnd,
                                       PRECTL prclDest);

   HPS     APIENTRY F_WinGetPS(HWND hwnd);

   BOOL    APIENTRY /*F_*/ WinReleasePS(HPS hps);

   BOOL    APIENTRY F_WinEndPaint(HPS hps);

   BOOL  APIENTRY F_WinFillRect(HPS hps,
                              PRECTL prcl,
                              LONG lColor);
   
   HPS   APIENTRY F_WinBeginPaint(HWND hwnd,
                                HPS hps,
                                PRECTL prclPaint);

   HDC   APIENTRY F_WinOpenWindowDC(HWND hwnd);
   

/* General Window fuctions */
HWND    APIENTRY F_WinCreateWindow(HWND hwndParent,
                                 PCSZ  pszClass,
                                 PCSZ  pszName,
                                 ULONG flStyle,
                                 LONG x,
                                 LONG y,
                                 LONG cx,
                                 LONG cy,
                                 HWND hwndOwner,
                                 HWND hwndInsertBehind,
                                 ULONG id,
                                 PVOID pCtlData,
                                 PVOID pPresParams);

BOOL    APIENTRY F_WinEnableWindow(HWND hwnd,
                                 BOOL fEnable);

BOOL    APIENTRY F_WinIsWindowEnabled(HWND hwnd);

BOOL    APIENTRY F_WinEnableWindowUpdate(HWND hwnd,
                                       BOOL fEnable);

BOOL    APIENTRY F_WinIsWindowVisible(HWND hwnd);

LONG    APIENTRY F_WinQueryWindowText(HWND hwnd,
                                    LONG cchBufferMax,
                                    PCH pchBuffer);

BOOL    APIENTRY F_WinSetWindowText(HWND hwnd,
                                  PCSZ  pszText);

LONG    APIENTRY F_WinQueryWindowTextLength(HWND hwnd);

HWND    APIENTRY F_WinWindowFromID(HWND hwndParent,
                                 ULONG id);

HWND    APIENTRY F_WinWindowFromDC(HDC hdc);

HDC     APIENTRY F_WinQueryWindowDC(HWND hwnd);

BOOL    APIENTRY F_WinIsWindow(HAB hab,
                             HWND hwnd);

HWND    APIENTRY F_WinQueryWindow(HWND hwnd,
                                LONG cmd);

LONG    APIENTRY F_WinMultWindowFromIDs(HWND hwndParent,
                                      PHWND prghwnd,
                                      ULONG idFirst,
                                      ULONG idLast);


/*** WinQueryWindow() codes ************************************************/

#define QW_NEXT         0
#define QW_PREV         1
#define QW_TOP          2
#define QW_BOTTOM       3
#define QW_OWNER        4
#define QW_PARENT       5
#define QW_NEXTTOP      6
#define QW_PREVTOP      7
#define QW_FRAMEOWNER   8

BOOL   APIENTRY F_WinSetParent(HWND hwnd,
                             HWND hwndNewParent,
                             BOOL fRedraw);

BOOL   APIENTRY F_WinIsChild(HWND hwnd,
                           HWND hwndParent);
BOOL   APIENTRY F_WinSetOwner(HWND hwnd,
                            HWND hwndNewOwner);

#ifndef F_INCL_SAADEFS
   BOOL   APIENTRY F_WinQueryWindowProcess(HWND hwnd,
                                         PPID ppid,
                                         PTID ptid);
#endif /* !F_INCL_SAADEFS */

HWND   APIENTRY F_WinQueryObjectWindow(HWND hwndDesktop);
HWND   APIENTRY F_WinQueryDesktopWindow(HAB hab,
                                      HDC hdc);

/*** Window positioning functions */

/* WinSetMultWindowPos() structure */

typedef struct _SWP    /* swp */
{
   ULONG   fl;
   LONG    cy;
   LONG    cx;
   LONG    y;
   LONG    x;
   HWND    hwndInsertBehind;
   HWND    hwnd;
   ULONG   ulReserved1;
   ULONG   ulReserved2;
} SWP;
typedef SWP *PSWP;

BOOL   APIENTRY F_WinSetWindowPos(HWND hwnd,
                                HWND hwndInsertBehind,
                                LONG x,
                                LONG y,
                                LONG cx,
                                LONG cy,
                                ULONG fl);
BOOL   APIENTRY F_WinSetMultWindowPos(HAB hab,
                                    PSWP pswp,
                                    ULONG cswp);
BOOL   APIENTRY F_WinQueryWindowPos(HWND hwnd,
                                  PSWP pswp);

/* Values returned from WM_ADJUSTWINDOWPOS and passed to WM_WINDOWPOSCHANGED */

#define AWP_MINIMIZED              0x00010000L
#define AWP_MAXIMIZED              0x00020000L
#define AWP_RESTORED               0x00040000L
#define AWP_ACTIVATE               0x00080000L
#define AWP_DEACTIVATE             0x00100000L

/* WinSetWindowPos() flags */

#define SWP_SIZE                   0x0001
#define SWP_MOVE                   0x0002
#define SWP_ZORDER                 0x0004
#define SWP_SHOW                   0x0008
#define SWP_HIDE                   0x0010
#define SWP_NOREDRAW               0x0020
#define SWP_NOADJUST               0x0040
#define SWP_ACTIVATE               0x0080
#define SWP_DEACTIVATE             0x0100
#define SWP_EXTSTATECHANGE         0x0200
#define SWP_MINIMIZE               0x0400
#define SWP_MAXIMIZE               0x0800
#define SWP_RESTORE                0x1000
#define SWP_FOCUSACTIVATE          0x2000
#define SWP_FOCUSDEACTIVATE        0x4000
#define SWP_NOAUTOCLOSE            0x8000    /* Valid in PROGDETAILS struct only */

/* Window painting */
//pmwin.h 582 - пока достаточно

//pmwin.h 705
/***************************************************************************/
/****                 Window Manager Subsection part 2                  ****/
#if (defined(F_INCL_WINWINDOWMGR) || !defined(F_INCL_NOCOMMON))

   BOOL APIENTRY F_WinSetActiveWindow(HWND hwndDesktop,
                                    HWND hwnd);

#endif /* Window Manager COMMON subsection */

#ifdef F_INCL_WINWINDOWMGR
   /* WM_CREATE structure */

   typedef struct _CREATESTRUCT    /* crst */
   {
      PVOID   pPresParams;
      PVOID   pCtlData;
      ULONG   id;
      HWND    hwndInsertBehind;
      HWND    hwndOwner;
      LONG    cy;
      LONG    cx;
      LONG    y;
      LONG    x;
      ULONG   flStyle;
      PSZ     pszText;
      PSZ     pszClass;
      HWND    hwndParent;
   } CREATESTRUCT;
   typedef CREATESTRUCT *PCREATESTRUCT;

   /* WinQueryClassInfo() structure */

   typedef struct _CLASSINFO    /* clsi */
   {
      ULONG   flClassStyle;
      PFNWP   pfnWindowProc;
      ULONG   cbWindowData;
   } CLASSINFO;
   typedef CLASSINFO *PCLASSINFO;

   BOOL   APIENTRY F_WinQueryClassInfo(HAB hab,
                                     PCSZ  pszClassName,
                                     PCLASSINFO pClassInfo);

#endif


   HWND  APIENTRY F_WinWindowFromPoint(HWND hwnd,
                                     PPOINTL pptl,
                                     BOOL fChildren);
   
   BOOL  APIENTRY F_WinMapWindowPoints(HWND hwndFrom,
                                     HWND hwndTo,
                                     PPOINTL prgptl,
                                     LONG cwpt);
   
#define WM_VRNDISABLED             0x007e
#define WM_VRNENABLED              0x007f

BOOL APIENTRY F_WinSetVisibleRegionNotify( HWND hwnd,
                                        BOOL fEnable);

ULONG APIENTRY F_WinQueryVisibleRegion( HWND hwnd,
                                     HRGN hrgn);


/* 1255 pmwin.h */
/*** Keyboard and mouse ****************************************************/

#if (defined(INCL_WININPUT) || !defined(INCL_NOCOMMON))

   /*** Keyboard and mouse input COMMON subsection *************************/
   BOOL  APIENTRY F_WinSetFocus(HWND hwndDesktop,
                              HWND hwndSetFocus);

   BOOL  APIENTRY /*F_*/ WinFocusChange(HWND hwndDesktop,
                                 HWND hwndSetFocus,
                                 ULONG flFocusChange);
#endif
/* 2663 pmwin.h */
#if (defined(F_INCL_WINFRAMEMGR) || !defined(F_INCL_NOCOMMON))
   /*** Frame Manager Common subsection */

   #pragma pack(2) /* force structure alignment packing */

   /* Binary resources may be bound into application executables or */
   /* passed as a parameter when creating a window or dialog        */
   /* Currently they must be the same for both 16-bit and 32-bit    */
   /* so we pack the structures.                                    */
   typedef struct _FRAMECDATA    /* fcdata */
   {
      USHORT  cb;
      ULONG   flCreateFlags;
      USHORT  hmodResources;
      USHORT  idResources;
   } FRAMECDATA;
   typedef FRAMECDATA *PFRAMECDATA;

   #pragma pack()  /* restore packing to default */

   /* Frame window styles */
   /* All unused FCF_xxx bits are reserved */
   #define FCF_TITLEBAR               0x00000001L
   #define FCF_SYSMENU                0x00000002L
   #define FCF_MENU                   0x00000004L
   #define FCF_SIZEBORDER             0x00000008L
   #define FCF_MINBUTTON              0x00000010L
   #define FCF_MAXBUTTON              0x00000020L
   #define FCF_MINMAX                 0x00000030L /* minmax means BOTH buttons */
   #define FCF_VERTSCROLL             0x00000040L
   #define FCF_HORZSCROLL             0x00000080L
   #define FCF_DLGBORDER              0x00000100L
   #define FCF_BORDER                 0x00000200L
   #define FCF_SHELLPOSITION          0x00000400L
   #define FCF_TASKLIST               0x00000800L
   #define FCF_NOBYTEALIGN            0x00001000L
   #define FCF_NOMOVEWITHOWNER        0x00002000L
   #define FCF_ICON                   0x00004000L
   #define FCF_ACCELTABLE             0x00008000L
   #define FCF_SYSMODAL               0x00010000L
   #define FCF_SCREENALIGN            0x00020000L
   #define FCF_MOUSEALIGN             0x00040000L
   #define FCF_HIDEBUTTON             0x01000000L
   #define FCF_HIDEMAX                0x01000020L /* hidemax means BOTH buttons */
   #define FCF_CLOSEBUTTON            0x04000000L /* use when no other min/max  */
   /*   button is present        */
   #ifdef F_INCL_NLS
      #define FCF_DBE_APPSTAT         0x80000000L
   #endif /* F_INCL_NLS */
   #define FCF_AUTOICON               0x40000000L

   /* FCF_TITLEBAR | FCF_SYSMENU | FCF_MENU | FCF_SIZEBORDER | FCF_MINMAX |
   FCF_ICON | FCF_ACCELTABLE | FCF_SHELLPOSITION | FCF_TASKLIST */
   #define FCF_STANDARD            0x0000CC3FL


   #define FS_ICON                    0x00000001L
   #define FS_ACCELTABLE              0x00000002L

   #ifndef F_INCL_SAADEFS
      #define FS_SHELLPOSITION        0x00000004L
   #endif /* !F_INCL_SAADEFS */

   #define FS_TASKLIST                0x00000008L
   #define FS_NOBYTEALIGN             0x00000010L
   #define FS_NOMOVEWITHOWNER         0x00000020L
   #define FS_SYSMODAL                0x00000040L
   #define FS_DLGBORDER               0x00000080L
   #define FS_BORDER                  0x00000100L
   #define FS_SCREENALIGN             0x00000200L
   #define FS_MOUSEALIGN              0x00000400L
   #define FS_SIZEBORDER              0x00000800L
   #define FS_AUTOICON                0x00001000L
   #ifdef F_INCL_NLS
      #define FS_DBE_APPSTAT          0x00008000L
   #endif /* F_INCL_NLS */

   /* FS_ICON | FS_ACCELTABLE | FS_SHELLPOSITION | FS_TASKLIST */
   #define FS_STANDARD                0x0000000FL

   #ifndef F_INCL_SAADEFS
      #define FF_FLASHWINDOW             0x0001
      #define FF_ACTIVE                  0x0002
      #define FF_FLASHHILITE             0x0004
      #define FF_OWNERHIDDEN             0x0008
      #define FF_DLGDISMISSED            0x0010
      #define FF_OWNERDISABLED           0x0020
      #define FF_SELECTED                0x0040
      #define FF_NOACTIVATESWP           0x0080
      #define FF_DIALOGBOX               0x0100

   #endif /* !F_INCL_SAADEFS */


   HWND  APIENTRY /*F_*/ WinCreateStdWindow(HWND hwndParent,
                                     ULONG flStyle,
                                     PULONG pflCreateFlags,
                                     PSZ pszClientClass,
                                     PSZ pszTitle,
                                     ULONG styleClient,
                                     HMODULE hmod,
                                     ULONG idResources,
                                     PHWND phwndClient);

#endif /* Frame Manager Common subsection */

/* size & position managment */
   BOOL  APIENTRY F_WinCalcFrameRect(HWND hwndFrame,
                                   PRECTL prcl,
                                   BOOL fClient);
   
   BOOL  APIENTRY F_WinGetMinPosition(HWND hwnd,
                                    PSWP pswp,
                                    PPOINTL pptl);
   BOOL  APIENTRY F_WinGetMaxPosition(HWND hwnd,
                                       PSWP pswp);

/*************************/
/**** Pointer manager ****/
/*************************/
   
   BOOL     APIENTRY F_WinSetPointer(HWND hwndDesktop,
                                   HPOINTER hptrNew);
   BOOL     APIENTRY F_WinSetPointerOwner(HPOINTER hptr,
                                        PID pid,
                                        BOOL fDestroy);
   BOOL     APIENTRY F_WinShowPointer(HWND hwndDesktop,
                                    BOOL fShow);
   BOOL     APIENTRY F_WinLockPointerUpdate(HWND hwndDesktop,
                                          HPOINTER hptrNew,
                                          ULONG ulTimeInterval);
   HPOINTER APIENTRY F_WinQuerySysPointer(HWND hwndDesktop,
                                        LONG iptr,
                                        BOOL fLoad);
   
   BOOL APIENTRY F_WinQuerySysPointerData(HWND hwndDesktop,
                                        ULONG iptr,
                                        PICONINFO pIconInfo);
   BOOL APIENTRY F_WinSetSysPointerData(HWND hwndDesktop,
                                      ULONG iptr,
                                      PICONINFO pIconInfo);
   
   /* System pointers (NOTE: these are 1-based) */
   
   #define SPTR_ARROW                 1
   #define SPTR_TEXT                  2
   #define SPTR_WAIT                  3
   #define SPTR_SIZE                  4
   #define SPTR_MOVE                  5
   #define SPTR_SIZENWSE              6
   #define SPTR_SIZENESW              7
   #define SPTR_SIZEWE                8
   #define SPTR_SIZENS                9
   #define SPTR_APPICON               10
   
   #define SPTR_ICONINFORMATION       11
   #define SPTR_ICONQUESTION          12
   #define SPTR_ICONERROR             13
   #define SPTR_ICONWARNING           14
   
   #define SPTR_ILLEGAL               18
   #define SPTR_FILE                  19
   #define SPTR_FOLDER                20
   #define SPTR_MULTFILE              21
   #define SPTR_PROGRAM               22
   #define SPTR_DISPLAY_PTRS          22
   #define SPTR_PENFIRST              23
   #define SPTR_PENLAST               39
   #define SPTR_CPTR                  39  /* total # of system pointers */
   
   /* backward compatibility */
   #define SPTR_HANDICON         SPTR_ICONERROR
   #define SPTR_QUESICON         SPTR_ICONQUESTION
   #define SPTR_BANGICON         SPTR_ICONWARNING
   #define SPTR_NOTEICON         SPTR_ICONINFORMATION
   
   
   HPOINTER APIENTRY F_WinLoadPointer(HWND hwndDesktop,
                                    HMODULE hmod,
                                    ULONG idres);
   HPOINTER APIENTRY F_WinCreatePointer(HWND hwndDesktop,
                                      HBITMAP hbmPointer,
                                      BOOL fPointer,
                                      LONG xHotspot,
                                      LONG yHotspot);
   BOOL     APIENTRY F_WinSetPointerPos(HWND hwndDesktop,
                                      LONG x,
                                      LONG y);
   BOOL     APIENTRY F_WinDestroyPointer(HPOINTER hptr);
   
   HPOINTER APIENTRY F_WinQueryPointer(HWND hwndDesktop);
   BOOL     APIENTRY F_WinQueryPointerPos(HWND hwndDesktop,
                                        PPOINTL pptl);
   
   typedef struct _POINTERINFO    /* ptri */
   {
      ULONG   fPointer;
      LONG    xHotspot;
      LONG    yHotspot;
      HBITMAP hbmPointer;
      HBITMAP hbmColor;
      HBITMAP hbmMiniPointer;
      HBITMAP hbmMiniColor;
   } POINTERINFO;
   typedef POINTERINFO *PPOINTERINFO;
   
   HPOINTER APIENTRY F_WinCreatePointerIndirect(HWND hwndDesktop,
                                              PPOINTERINFO pptri);
   BOOL     APIENTRY F_WinQueryPointerInfo(HPOINTER hptr,
                                         PPOINTERINFO pPointerInfo);
   BOOL     APIENTRY F_WinDrawPointer(HPS hps,
                                    LONG x,
                                    LONG y,
                                    HPOINTER hptr,
                                    ULONG fs);
   
   
   
   /* WinDrawPointer() constants */
   
   #define DP_NORMAL                  0x0000
   #define DP_HALFTONED               0x0001
   #define DP_INVERTED                0x0002
   #define DP_MINI                    0x0004
   
   
   HBITMAP APIENTRY F_WinGetSysBitmap(HWND hwndDesktop,
                                    ULONG ibm);
   
   /* System bitmaps (NOTE: these are 1-based) */
   
   #define SBMP_OLD_SYSMENU           1
   #define SBMP_OLD_SBUPARROW         2
   #define SBMP_OLD_SBDNARROW         3
   #define SBMP_OLD_SBRGARROW         4
   #define SBMP_OLD_SBLFARROW         5
   #define SBMP_MENUCHECK             6
   #define SBMP_OLD_CHECKBOXES        7
   #define SBMP_BTNCORNERS            8
   #define SBMP_OLD_MINBUTTON         9
   #define SBMP_OLD_MAXBUTTON         10
   #define SBMP_OLD_RESTOREBUTTON     11
   #define SBMP_OLD_CHILDSYSMENU      12
   #define SBMP_DRIVE                 15
   #define SBMP_FILE                  16
   #define SBMP_FOLDER                17
   #define SBMP_TREEPLUS              18
   #define SBMP_TREEMINUS             19
   #define SBMP_PROGRAM               22
   #define SBMP_MENUATTACHED          23
   #define SBMP_SIZEBOX               24
   
   #define SBMP_SYSMENU               25
   #define SBMP_MINBUTTON             26
   #define SBMP_MAXBUTTON             27
   #define SBMP_RESTOREBUTTON         28
   #define SBMP_CHILDSYSMENU          29
   #define SBMP_SYSMENUDEP            30
   #define SBMP_MINBUTTONDEP          31
   #define SBMP_MAXBUTTONDEP          32
   #define SBMP_RESTOREBUTTONDEP      33
   #define SBMP_CHILDSYSMENUDEP       34
   #define SBMP_SBUPARROW             35
   #define SBMP_SBDNARROW             36
   #define SBMP_SBLFARROW             37
   #define SBMP_SBRGARROW             38
   #define SBMP_SBUPARROWDEP          39
   #define SBMP_SBDNARROWDEP          40
   #define SBMP_SBLFARROWDEP          41
   #define SBMP_SBRGARROWDEP          42
   #define SBMP_SBUPARROWDIS          43
   #define SBMP_SBDNARROWDIS          44
   #define SBMP_SBLFARROWDIS          45
   #define SBMP_SBRGARROWDIS          46
   #define SBMP_COMBODOWN             47
   #define SBMP_CHECKBOXES            48
   #define SBMP_HIDE                  50
   #define SBMP_HIDEDEP               51
   #define SBMP_CLOSE                 52
   #define SBMP_CLOSEDEP              53
   
/*******************************/
/**** endof Pointer manager ****/
/*******************************/



/******* Virtual key values *******************/
   #define VK_BUTTON1                 0x01
   #define VK_BUTTON2                 0x02
   #define VK_BUTTON3                 0x03
   #define VK_BREAK                   0x04
   #define VK_BACKSPACE               0x05
   #define VK_TAB                     0x06
   #define VK_BACKTAB                 0x07
   #define VK_NEWLINE                 0x08
   #define VK_SHIFT                   0x09
   #define VK_CTRL                    0x0A
   #define VK_ALT                     0x0B
   #define VK_ALTGRAF                 0x0C
   #define VK_PAUSE                   0x0D
   #define VK_CAPSLOCK                0x0E
   #define VK_ESC                     0x0F
   #define VK_SPACE                   0x10
   #define VK_PAGEUP                  0x11
   #define VK_PAGEDOWN                0x12
   #define VK_END                     0x13
   #define VK_HOME                    0x14
   #define VK_LEFT                    0x15
   #define VK_UP                      0x16
   #define VK_RIGHT                   0x17
   #define VK_DOWN                    0x18
   #define VK_PRINTSCRN               0x19
   #define VK_INSERT                  0x1A
   #define VK_DELETE                  0x1B
   #define VK_SCRLLOCK                0x1C
   #define VK_NUMLOCK                 0x1D
   #define VK_ENTER                   0x1E
   #define VK_SYSRQ                   0x1F
   #define VK_F1                      0x20
   #define VK_F2                      0x21
   #define VK_F3                      0x22
   #define VK_F4                      0x23
   #define VK_F5                      0x24
   #define VK_F6                      0x25
   #define VK_F7                      0x26
   #define VK_F8                      0x27
   #define VK_F9                      0x28
   #define VK_F10                     0x29
   #define VK_F11                     0x2A
   #define VK_F12                     0x2B
   #define VK_F13                     0x2C
   #define VK_F14                     0x2D
   #define VK_F15                     0x2E
   #define VK_F16                     0x2F
   #define VK_F17                     0x30
   #define VK_F18                     0x31
   #define VK_F19                     0x32
   #define VK_F20                     0x33
   #define VK_F21                     0x34
   #define VK_F22                     0x35
   #define VK_F23                     0x36
   #define VK_F24                     0x37
   #define VK_ENDDRAG                 0x38
   #define VK_CLEAR                   0x39
   #define VK_EREOF                   0x3A
   #define VK_PA1                     0x3B
   #define VK_ATTN                    0x3C
   #define VK_CRSEL                   0x3D
   #define VK_EXSEL                   0x3E
   #define VK_COPY                    0x3F
   #define VK_BLK1                    0x40
   #define VK_BLK2                    0x41
   
   #define VK_MENU                    VK_F10

      LONG APIENTRY F_WinGetKeyState(HWND hwndDesktop,
                                   LONG vkey);

#ifdef __cplusplus
      }
#endif

//test 
#include "PM_api.h"
#endif
  /* FREEPM_WIN */
