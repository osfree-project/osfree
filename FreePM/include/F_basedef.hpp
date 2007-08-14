/*
 $Id: F_basedef.HPP,v 1.6 2003/06/18 22:36:12 evgen2 Exp $
*/
/* F_basedef.hpp */
/* analog \TOOLKIT\h\basedef.h */
/* ver 0.00 14.08.2002 */

#ifndef FREEPMDEF_BASEDEF
  #define FREEPMDEF_BASEDEF

#ifdef __cplusplus
      extern "C" {
#endif

#ifndef NULL
      #if (defined(__EXTENDED__)  || defined( __cplusplus ))
         #define NULL 0
      #else
         #define NULL ((void *)0)
      #endif
#endif

#ifndef FALSE
   #define FALSE   0
#endif

#ifndef TRUE
   #define TRUE    1
#endif

#define LOCKED    1
#define UNLOCKED  0

#define VOID    void
#define CONST   const
#define VOLATILE volatile

#define far
#define FAR
#define near
#define NEAR

typedef unsigned short   SHANDLE;
typedef unsigned long    LHANDLE;

#define NULLHANDLE    ( (LHANDLE) 0)
#define NULLSHANDLE   ( (SHANDLE) 0)

/*** Common types */

typedef int INT;                        // i
typedef unsigned int UINT;              // u
typedef INT BOOL;                       // f
#define BOOL32    BOOL
typedef unsigned char BYTE;             // b
typedef BYTE  *PBYTE;                  // pb

typedef unsigned char   byte;

typedef char CHAR;                      // ch
typedef char *PCH;                     // pch
typedef CHAR *PCHAR;                    // pch
typedef unsigned char UCHAR;            // uch
typedef short SHORT;                    // s
typedef SHORT *PSHORT;                         // ps
typedef unsigned short USHORT;          // us
typedef int LONG;                       // l
typedef LONG  *PLONG;                   // pl
typedef unsigned int  ULONG;            // ul
typedef ULONG *PULONG;                  // pul

typedef void             *PVOID ,  *LPVOID;
typedef const VOID       *PCVOID,  *LPCVOID;
typedef CHAR             *PSZ   ,  *LPSZ;
typedef const CHAR       *PCSZ  ,  *LPCSZ;

typedef LHANDLE   HAB;         /* hab  */
typedef HAB *PHAB;

typedef LHANDLE HMODULE;        /* hmod */

/*** Common GPI/DEV types */

typedef LHANDLE   HPS;         /* hps  */
typedef HPS *PHPS;

typedef LHANDLE   HDC;         /* hdc  */
typedef HDC *PHDC;

typedef LHANDLE   HRGN;        /* hrgn */
typedef HRGN *PHRGN;

typedef LHANDLE   HBITMAP;     /* hbm  */
typedef HBITMAP *PHBITMAP;

typedef LHANDLE HPOINTER;

typedef LHANDLE   HMF;         /* hmf  */
typedef HMF *PHMF;

typedef LHANDLE   HPAL;        /* hpal */
typedef HPAL *PHPAL;

typedef LONG     COLOR;        /* clr  */
typedef COLOR *PCOLOR;

#ifndef APIENTRY
  #define APIENTRY    _System
#endif
typedef int ( APIENTRY _PFN)  ();
typedef _PFN    *PFN;

typedef struct _POINTS         /* pts */
{
   SHORT x;
   SHORT y;
} POINTS;
typedef POINTS *PPOINTS;

typedef struct _POINTL  {               // ptl
    LONG  x;
    LONG  y;
} POINTL;
typedef POINTL  *PPOINTL;

typedef struct _RECTL {                 // rcl
    LONG  xLeft;
    LONG  yBottom;
    LONG  xRight;
    LONG  yTop;
} RECTL;
typedef RECTL  *PRECTL;

typedef struct _ICONINFO {  /* icninf */
   ULONG   cb;                  /* size of ICONINFO structure */
   ULONG   fFormat;
   PSZ     pszFileName;         /* use when fFormat = ICON_FILE */
   HMODULE hmod;                /* use when fFormat = ICON_RESOURCE */
   ULONG   resid;               /* use when fFormat = ICON_RESOURCE */
   ULONG   cbIconData;          /* use when fFormat = ICON_DATA     */
   PVOID   pIconData;           /* use when fFormat = ICON_DATA     */
} ICONINFO;
typedef ICONINFO FAR *PICONINFO;

#define ICON_FILE     1         /* flags for fFormat */
#define ICON_RESOURCE 2
#define ICON_DATA     3
#define ICON_CLEAR    4

#define LONGfromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)

/*** Common WIN types */

typedef VOID *MPARAM;      /* mp    */
typedef MPARAM *PMPARAM;   /* pmp   */
typedef VOID *MRESULT;     /* mres  */
typedef MRESULT *PMRESULT; /* pmres */

typedef LHANDLE HWND;      /* hwnd */
typedef HWND *PHWND;

typedef LHANDLE HMQ;       /* hmq */
typedef LHANDLE *PHMQ;


/* Combine l & h to form a 32 bit quantity. */
#define MAKEULONG(l, h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKELONG(l, h)   ((LONG)MAKEULONG(l, h))

/* Combine l & h to form a 16 bit quantity. */
#define MAKEUSHORT(l, h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))


#ifdef __cplusplus
      }
#endif
#endif  /*FREEPMDEF_BASEDEF */

