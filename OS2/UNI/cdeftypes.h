#ifndef __CDEFTYPES_H__
#define __CDEFTYPES_H__
#define CDEFTYPES_INCLUDED

/* Default includes (default modifier of uses) */

#define INCL_OS2
#define INCL_OS2DEF
#define INCL_BSE
#define INCL_BSEDOS
#define INCL_CDEFTYPES2
#define INCL_PM
/* Original OS/2 default definitions */

#ifdef INCL_BASE
    #define INCL_DOS
    #define INCL_SUB
    #define INCL_DOSERRORS
#endif


/* Aliases (alias modifier of uses) */


#ifdef INCL_SUB
  #define INCL_BSESUB
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



/* Base types */
#define Byte unsigned char
#define BYTE Byte
#define DWord unsigned long
#define DWORD DWord
#define UINT unsigned int
#define Word unsigned short
#define Char char
#define LongInt long
#define pointer void *

#ifndef NULL
#define NULL  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Calling convertion things */

#define EXPENTRY  _System
#define APIENTRY  _System

#define APIENTRY16 _Far16 _Pascal

#define FAR
#define NEAR


#endif
