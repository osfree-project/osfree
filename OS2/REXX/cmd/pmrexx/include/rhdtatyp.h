/*static char *SCCSID = "@(#)rhdtatyp.h	6.4 92/01/15";                       */
/*********************  START OF SPECIFICATIONS  *******************          */
/*                                                                            */
/*  SOURCE FILE NAME:  rhdtatyp.h                                             */
/*  DESCRIPTIVE NAME:  Structure and macro definitions and function           */
/*                     prototypes for pmrexxio.c                              */
/*                                                                            */
/*  COPYRIGHT:         IBM Corporation 1990                                   */
/*  STATUS:            Version 2.00                                           */
/*                                                                            */
/*  DESCRIPTION AND PURPOSE:                                                  */
/*           This module provides the structure and macro                     */
/*      definitions necessary for compilation of the pmrexxio                 */
/*      modules.                                                              */
/*                                                                            */
/*  EXTERNAL DEPENDENCIES:                                                    */
/*          This assumes that OS2.H has already been included to              */
/*      define the types used.                                                */
/*                                                                            */
/***********************  END OF SPECIFICATIONS  *******************          */

#include "pmrexxio.h"

/* This needs to be defined early, for rxhdlong.h and filedlg.h               */
/* Maximum supported filename sz                                              */

#define  FNAME_SIZE     260

/* For Dos call errors, post a message to the client window to let            */
/*  put up a message box with the error information                           */

#define  doserrorbox(x,y)  SysErrorBox(pWinData->client, x, y);

/* This section defines the constants used in the pmrexx program              */

#define  ALLOCSIZE      8192           /* This is the initial allocation size */
                                       /* for the _IMPORT structure           */
#define  MLE_TXT_FMT    MLFIE_CFTEXT   /* This is the format used to import   */
                                       /* text to the MLE                     */
#define  MSGQUEUE_SIZE  2000           /* This is the size for the main       */
                                       /* message queues                      */
#define  SMALL_MQ_SIZE  0              /* And the size for the small queues (0*/
                                       /* = default)                          */
#define  MAXWIDTH       200
#define  ANSLEN         128
#define  PIPE_SIZE      8192           /* Size to use for pipes               */
#define  INHERIT        0x0080         /* Inheritance bit for handles         */
#define  TITLE_SIZE     55             /* Maximum title length                */
#define  BORDER_MARGIN  3              /* Border area around MLE              */
#define  SAVE_WIDTH     225            /* Size of save pulldown               */
#define  TIMER_DELAY    100            /* Length of timer delay               */
#define  ONE_K          1024
#define  RXHD_FONT      260            /* Identified for Font Dialog.         */
#define  ENDLINE        '\r'
#define  END_STR        '\0'
#define  KEY_PRESSED    0x8000         /* For WinGetKeyState                  */
#define  RESOURCE_FLAG  0xffff0000
#define  STD_IN         0              /* Index for standard input            */
#define  TRC_IN         1              /* Index for trace input.              */
#define  STDIN          0              /* Handle for standard input           */
#define  STDOUT         1              /* Handle for standard output          */
#define  STDERR         2              /* Handle for standard error           */
#define  QUERYTHREAD    2              /* For DosGetPrty 2 means thread       */
#define  STACK_SIZE     8196           /* thread stack size                   */
#define  IMPORTSIZE     8196           /* size of import buffer               */
#define  ADD_STDOUTWIN  WM_USER+999
#define  INPUT_LINE     WM_USER+998
#define  RXIOB_MSGBOX   WM_USER+997
#define  START_TIMER    WM_USER+996
#define  PMREXXCLASSNAME "PMREXXIO"
#define  IO_EXIT         "PMREXXIO"
#define  MODULE_NAME     "PMREXXIO"

/* Next, define the structures and type definitions.                          */
/* These define a simple set of list structures used for dispersing           */
/*   input lines to the appropriate targets.                                  */

typedef struct _LIST_ENTRY {           /* This structure defines a list       */
                                       /* element                             */
    struct _LIST_ENTRY *next;          /* Link to next entry                  */

    void *ptr;                         /* Pointer to data structure           */
    ULONG len;                         /* Length of data structure            */
} LIST_ENTRY;

typedef LIST_ENTRY *PLIST_ENTRY;

typedef struct _LIST {                 /* This structure defines a list to be */
                                       /* a pointer to elements and two       */
                                       /* semaphores for access control       */
    HMTX  q_sem;                       /* Queue manipulation semaphore        */
    HEV   q_data;                      /* Queue data semaphore                */
    PLIST_ENTRY q_1st;                 /* List element pointer                */
} LIST;

typedef LIST *PLIST;

/* The _IMPORT structure is used to collect multiple lines together           */
/*   for import into the Multi-Line Edit Control window all at one            */
/*   time. This is necessary to keep the performance reasonable if            */
/*   significant amounts of output are being generated all at once.           */

typedef struct _IMPORT {
    ULONG total_size;                  /* Total size of the data block        */
    ULONG inuse;                       /* Number of bytes currently in use,   */
                                       /* including the header                */
    HMTX sem;                          /* Semaphore for access control        */
    UCHAR data[1];                     /* Start of the data area              */
} IMPORT;

typedef IMPORT *PIMPORT;

#define  HDRSIZE        (sizeof(ULONG)*2+sizeof(HMTX))/* Number of header    */
                                       /* bytes in the _IMPORT structure      */

/* Search dialogs parameters structure  */
typedef struct _SEARCHINFO {
   USHORT size, dummy;
   BOOL caseSensitive;
   CHAR szSearchString[100];
} SEARCHINFO;

typedef SEARCHINFO *PSEARCHINFO;
   
/* The _RHWINDATA structure defines an area allocated for each                */
/*   main window to hold parametric data about that window. The               */
/*   structure is allocated at window initialization and a pointer            */
/*   to it is stored with the frame window.                                   */

typedef struct _RHWINDATA {
    HAB     hab;                       /* anchor block                        */
    HWND    frame;                     /* Window handle for Frame             */
    HWND    client;                    /* Window handle for Client area       */
    HWND    outmle;                    /* Window handle for MLE               */
    HWND    insle;                     /* Window handle for input field       */
    HWND    HelpInst;                  /* window handle for help instance     */
    HMODULE hResource;                 /* module handle                       */
    HFILE   stdin_r;
    HFILE   stdin_w;
    HFILE   stdout_r;
    HFILE   stdout_w;
    HFILE   stderr_w;
    LIST    pipe_in;                   /* Initialize structure for input queue*/
    LIST    trace_in;                  /* Initialize structure for trace input*/
                                       /* queue                               */
    ULONG   in_type;                   /* Specifies where to get input from   */
    ULONG   in_hgt;                    /* Height for input entry field        */
    LONG    wtitle;                    /* Width of input area title           */
    BOOL    visible;                   /* True if PMREXX is visible           */
    BOOL    trace_inp;                 /* True if input is being traced       */
    BOOL    tracebit;                  /* Indicates if tracing is enabled     */
    BOOL    RxHalt;                    /* True if we are to halt the procedure*/
    BOOL    RxResult;                  /* True if we are to halt the procedure*/
    BOOL    imp_queued;                /* True if we are to get input from the*/
                                       /* queue                               */
    BOOL    infocus;                   /* Input window has focus flag         */
    BOOL    stopoutpipe;               /* shutdown output pipe                */
    PIMPORT imp_ptr;                   /* Pointer to buffer for data waiting  */
                                       /* to be placed into MLE               */
    PUCHAR  pszInputText;              /* Input field title                   */
    PID     proc_id;                   /* process id of caller                */
    TID     in_tid;                    /* input queue thread id               */
    TID     out_tid;                   /* output queue thread id              */
    TID     rexx_tid;                  /* rexx thread id                      */
    COUNTRYINFO  CountryInfo;          /* county information                  */
    COUNTRYCODE  CountryCode;          /* code page info                      */
    FONTDLG MleFontAttrs;              /* MleFontAttributes                   */
    USHORT  timer;                     /* Time-out timer for import           */
    UCHAR   ImportBuffer[ALLOCSIZE];   /* import buffer                       */
    UCHAR   MleFamilyName[MAXWIDTH];   /* font family name                    */
} RHWINDATA;

typedef RHWINDATA *PRHWINDATA;

typedef struct _SAVEDWINDOWPOS {
    ULONG  flags;
    SHORT cy;
    SHORT cx;
    SHORT y;
    SHORT x;
    SHORT restorecy;
    SHORT restorecx;
    SHORT restorey;
    SHORT restorex;
} SAVEDWINDOWPOS;


/* Macros                                                                     */

#define  addline(w,l)                                                       \
                         {const char * pcht=(l);                                   \
                        while (!WinPostMsg((w),ADD_STDOUTWIN \
                               ,MPFROMLONG((pcht)) \
                                                                              \
                         ,MPFROMSHORT(strlen((pcht))+1)))DosSleep((ULONG)TIMER_DELAY);}
#define  max(a,b) (((a) > (b)) ? (a) : (b))

/*                Function prototype section                                  */

MRESULT EXPENTRY MainWndProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
MRESULT EXPENTRY EntryDlgProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
RexxExitHandler io_routine;
void    SetOptions(HWND, ULONG ,BOOL);
void    SetCheckMark(HWND, ULONG ,BOOL);

PLIST_ENTRY Remove_Q_Element(PLIST);
void    Add_Q_Element(PLIST, PLIST_ENTRY);

void    makepipe(PHFILE, HFILE, PHFILE, HFILE, ULONG );
void    setinherit(HFILE, BOOL);
void    stdinpipe(PRHWINDATA);
#pragma linkage(stdinpipe, system)
void    stdoutpipe(PRHWINDATA);
#pragma linkage(stdoutpipe, system)
LONG    initialize(PRHWINDATA);
void    SizeWindow(HWND, PRHWINDATA);
void    ClipBoard(HWND, USHORT, ULONG, PRHWINDATA);
ULONG   RHWinErrorBox(HAB, HWND, HMODULE, ULONG, ULONG );
void    Import(PRHWINDATA);
MRESULT RXIOCmds(HWND, MPARAM, PRHWINDATA);
void    getfixedfont(HWND, PFATTRS);
void    SysErrorBox(HWND, ULONG, PUCHAR);
ULONG   SysErrorBoxM(HWND,ULONG, PSZ, ULONG );
PUCHAR  getstring(HAB, HMODULE, ULONG);
void    SaveWindow(PRHWINDATA);
void    RestoreWindow(HWND, PRHWINDATA);
ULONG   SelectFont(PRHWINDATA);
