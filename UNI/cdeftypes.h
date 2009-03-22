#ifndef __CDEFTYPES_H__
#define __CDEFTYPES_H__
#define CDEFTYPES_INCLUDED

/* Default includes (default modifier of uses) */

#define INCL_OS2
#define INCL_OS2DEF
#define INCL_BSE
#define INCL_BSEDOS
#define INCL_CDEFTYPES2
#define INCL_HEVDEF
#define INCL_KBD
#define INCL_VIO
#define INCL_MOU

#define INCL_DOSSEMAPHORESCOMMON
#define INCL_DOSSEMAPHORES
#define INCL_WINWINDOWMGR
#define INCL_WINWINDOWMGRCOMMON
#define INCL_WINMESSAGEMGRCOMMON
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS
#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCESCOMMON
#define INCL_DOSRESOURCES
#define INCL_WINFRAMEMGRCOMMON
#define INCL_WINCLIPBOARDDDE
#define INCL_WINCLIPBOARD
#define INCL_WINSHELLDATA
#define INCL_WINSWITCHLIST
#define INCL_WINERRORS
#define INCL_SHLERRORS
#define INCL_WPERRORS
#define INCL_PMERR
#define INCL_GPIERRORS
#define INCL_SPLERRORS
#define INCL_PICERRORS
#define INCL_WINSWITCHLISTCOMMON
#define INCL_DOSDATETIMECOMMON
#define INCL_DOSFILEMGRCOMMON
#define INCL_DOSMEMMGRCOMMON
#define INCL_BSEMEMF
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#define INCL_WINDIALOGSCOMMON
#define INCL_WINDIALOGS
#define INCL_DOSDEVICES
#define INCL_DOSEXCEPTIONS
#define INCL_BSEXCPT
#define INCL_DOSEXCPTFUNC
#define INCL_DOSSESMGR
#define INCL_DOSPROCESSCOMMON
#define INCL_DOSNMPIPES
#define INCL_BSETIB
#define INCL_PMWIN
#define INCL_PMDEV
#define INCL_DEVCOMMON
#define INCL_PMGPI
#define INCL_PMSHL
#define INCL_WINPROGRAMLIST
#define INCL_WININPUT
#define INCL_WININPUTCOMMON
#define INCL_WINHOOKS
#define INCL_PMSTDDLG
#define INCL_WINSTDBOOK
#define INCL_WINENTRYFIELDS
#define INCL_WINBUTTONS
#define INCL_WINCOUNTRY
#define INCL_WINTIMER
#define INCL_WINLISTBOXES
#define INCL_GPIPRIMITIVES
#define INCL_GPILCIDS
#define INCL_GPICONTROLCOMMON
#define INCL_GPIBITMAPSCOMMON
#define INCL_GPIBITMAPS
#define INCL_GPILOGCOLTABLE

#ifdef INCL_DOSNMPIPES
#define INCL_DOS
#endif

#ifdef INCL_AVIO
#define INCL_PMAVIO
#endif

#ifdef INCL_SPL
#define INCL_PMSPL
#endif

#ifdef INCL_PIC
#define INCL_PMPIC
#endif

#ifdef INCL_ORDERS
#define INCL_PMORD
#endif

#ifdef INCL_FONTFILEFORMAT
#define INCL_PMFONT
#endif

/* Original OS/2 default definitions */

#ifdef INCL_WINMESSAGEMGR
#define INCL_WIN
#endif

#ifdef INCL_WINSHELLDATA
#define INCL_WIN
#endif

#ifdef INCL_WINSYS
#define INCL_WIN
#endif

#ifdef INCL_WIN
#define INCL_PM
#endif

#ifdef INCL_PM
#define INCL_PMDEF
#endif

#ifdef INCL_BASE
    #define INCL_DOS
    #define INCL_SUB
    #define INCL_DOSERRORS
#endif


/* Aliases (alias modifier of uses) */

#ifdef INCL_VIO
  #define INCL_SUB
#endif

#ifdef INCL_MOU
  #define INCL_SUB
#endif

#ifdef INCL_KBD
  #define INCL_SUB
#endif

#ifdef INCL_BSESUB
  #define INCL_SUB
#endif

#ifdef INCL_SUB
  #define INCL_BSESUB
#endif

#ifdef INCL_ERRORS
  #define INCL_BSEERR
#endif

#ifdef INCL_DOSERRORS
  #define INCL_BSEERR
#endif

#ifdef INCL_DOSDEVIOCTL
  #define INCL_BSEDEV
#endif

#ifdef INCL_ORDINALS
  #define INCL_BSEORD
#endif

#ifdef INCL_DOSMISC
  #define INCL_BSEDOS
#endif

#ifdef INCL_DOSMODULEMGR
  #define INCL_BSEDOS
#endif

#ifdef INCL_DOSNMPIPES
  #define INCL_BSEDOS
#endif

#ifdef INCL_KBD
  #define INCL_BSESUB
#endif

#ifdef INCL_VIO
  #define INCL_BSESUB
#endif

#ifdef INCL_WINWORKPLACE
  #define INCL_PMWIN
#endif

/** if INCL_PM defined then define all the symbols ***********************/
#ifdef INCL_PM
   #define INCL_AVIO
   #define INCL_BITMAPFILEFORMAT
   #define INCL_DEV
   #define INCL_ERRORS
   #define INCL_FONTFILEFORMAT
   #define INCL_GPI
   #define INCL_ORDERS
   #define INCL_PIC
   #define INCL_SPL
   #define INCL_WIN
   #define INCL_WINSTDDLGS
   #define INCL_WINWORKPLACE
#endif /* INCL_PM */

#define INCL_PMSHL

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define VOID void
/* Calling convertion things */

#define EXPENTRY  _System
#define APIENTRY  _System

#define APIENTRY16 _Far16 _Pascal

#define FAR
#define NEAR


#endif
