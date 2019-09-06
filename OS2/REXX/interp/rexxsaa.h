/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __REXXSAA_H_INCLUDED
#define __REXXSAA_H_INCLUDED
/* Remember to add prototypes for Rexx*() functions */

#if defined(WIN32) && (defined(__WATCOMC__) || defined(__BORLANDC__))
# include <windows.h>
#endif

#if defined(__RSXNT__) || defined(__MINGW32__)
# include <windows.h>
#endif

#if defined(_MSC_VER) && !defined(__WINS__)
# if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
# endif
# include <windows.h>
# if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
# endif
# define VOID_TYPEDEFED
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * pack(1) was introduced in Regina 3.3 to become binary compatible in
 * structures with OS/2, Object Rexx, etc.
 */
#ifdef OREXX_BINARY_COMPATIBLE
# pragma pack(1)
#endif

#ifndef CONST
/* Introduced in Regina 2.0, may/should not exist in non-ANSI compilers.
 * Everybody can see which params will change even if CONST is not defined.
 * We don't use CONST at all places to be consistent with other
 * implementations. You can see a "CONST" as a comment before each variable
 * which is/must not be changed.
 */
# define CONST const
#endif

#if defined(__MINGW32__)
# if defined(__REGINA_DLL)
#  define EXTNAME(n) __declspec(dllexport)
# else
#  define EXTNAME(n) __declspec(dllimport)
# endif
#else
#define EXTNAME(n)
#endif

#if !defined(OS2_VAC_C) && !defined(_OS2EMX_H) && !defined(__RSXNT__) && !defined(__MINGW32__)

# if !defined(VOID_TYPEDEFED) && !defined(VOID)
typedef void VOID ;
#  define VOID_TYPEDEFED
# endif

# ifndef PVOID_TYPEDEFED
typedef void* PVOID ;
#  define PVOID_TYPEDEFED
# endif

# ifndef CHAR_TYPEDEFED
typedef char CHAR;
#  define CHAR_TYPEDEFED
# endif
# ifndef PCHAR_TYPEDEFED
typedef CHAR *PCHAR ;
#  define PCHAR_TYPEDEFED
# endif
# ifndef UCHAR_TYPEDEFED
typedef unsigned char UCHAR ;
#  define UCHAR_TYPEDEFED
# endif
# ifndef PUCHAR_TYPEDEFED
typedef UCHAR *PUCHAR;
#  define PUCHAR_TYPEDEFED
# endif

# ifndef SHORT_TYPEDEFED
typedef short SHORT;
#  define SHORT_TYPEDEFED
# endif
# ifndef PSHORT_TYPEDEFED
typedef SHORT *PSHORT ;
#  define PSHORT_TYPEDEFED
# endif
# ifndef USHORT_TYPEDEFED
typedef unsigned short USHORT ;
#  define USHORT_TYPEDEFED
# endif
# ifndef PUSHORT_TYPEDEFED
typedef USHORT *PUSHORT;
#  define PUSHORT_TYPEDEFED
# endif

# ifndef LONG_TYPEDEFED
typedef long LONG;
#  define LONG_TYPEDEFED
# endif
# ifndef PLONG_TYPEDEFED
typedef LONG *PLONG;
#  define PLONG_TYPEDEFED
# endif
# ifndef ULONG_TYPEDEFED
typedef unsigned long ULONG ;
#  define ULONG_TYPEDEFED
# endif

typedef char *PSZ ;
typedef CONST char *PCSZ ;
typedef CHAR *PCH ;

#endif

#ifdef __MINGW32__
typedef CONST char *PCSZ ;
#endif

#ifdef INCL_REXXSAA
# define INCL_RXSUBCOM
# define INCL_RXSHV
# define INCL_RXFUNC
# define INCL_RXSYSEXIT
# define INCL_RXMACRO
# define INCL_RXARI
# define INCL_RXQUEUE
# define INCL_RXMACRO
#endif

/*
 * For Innotek gcc, force use of _System calling convention
 */
#if defined( __EMX__ ) && defined( __INNOTEK_LIBC__ )
# undef APIENTRY
# define APIENTRY _System
#endif

#if !defined(APIENTRY)
# define APIENTRY
#endif

#if !defined(APIRET)
# define APIRET ULONG
#endif

#if !defined(_OS2EMX_H) && !defined( PFN_TYPEDEFED )
typedef APIRET (APIENTRY *PFN)();
#endif


typedef struct RXSTRING_type
{
   ULONG strlength ;
   char* strptr ;
} RXSTRING ;
typedef RXSTRING *PRXSTRING ;

#if !defined(_OS2EMX_H)
typedef struct REXXDATETIME_type
{
   USHORT hours;               /* hour of the day (24-hour)    */
   USHORT minutes;             /* minute of the hour           */
   USHORT seconds;             /* second of the minute         */
   USHORT hundredths;          /* hundredths of a second       */
   USHORT day;                 /* day of the month             */
   USHORT month;               /* month of the year            */
   USHORT year;                /* current year                 */
   USHORT weekday;             /* day of the week              */
   ULONG  microseconds;        /* microseconds                 */
   ULONG  yearday;             /* day number within the year   */
   USHORT valid;               /* valid time stamp marker      */
} REXXDATETIME;
#define DATETIME REXXDATETIME
typedef REXXDATETIME *PDATETIME;
#endif

#define RXAUTOBUFLEN  256L

typedef struct {
   char *sysexit_name ;
   LONG  sysexit_code ;
} RXSYSEXIT ;
typedef RXSYSEXIT *PRXSYSEXIT ;

typedef PUCHAR PEXIT ;

/*
 * typedefs for Rexx handler types
 */
typedef APIRET APIENTRY RexxFunctionHandler(PCSZ name, ULONG argc, /* CONST */ PRXSTRING argv, PCSZ queuename, PRXSTRING returnstring) ;
typedef LONG APIENTRY RexxExitHandler( LONG, LONG, /* CONST */ PEXIT ) ;
typedef APIRET APIENTRY RexxSubcomHandler(/* CONST */ PRXSTRING, PUSHORT, PRXSTRING);

/*
 * -------------------------------------------------------------------
 * System Exit Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXSYSEXIT

/*
 * The following are definition for the function codes and subcodes for
 * System Exit Handlers. All RXxxx macros are function codes, while the
 * RXxxxyyy macros are function subcodes for function code RXxxx.
 */
#define RXFNC        2
# define RXFNCCAL    1

#define RXCMD        3
# define RXCMDHST    1

#define RXMSQ        4 /* Net yet implemented */
# define RXMSQPLL    1 /* Net yet implemented */
# define RXMSQPSH    2 /* Net yet implemented */
# define RXMSQSIZ    3 /* Net yet implemented */
# define RXMSQNAM   20 /* Net yet implemented */

#define RXSIO        5
# define RXSIOSAY    1
# define RXSIOTRC    2
# define RXSIOTRD    3
# define RXSIODTR    4
# define RXSIOTLL    5 /* Net yet implemented */

#define RXHLT        7
# define RXHLTCLR    1
# define RXHLTTST    2

#define RXTRC        8
# define RXTRCTST    1

#define RXINI        9
# define RXINIEXT    1

#define RXTER       10
# define RXTEREXT    1

#define RXDBG       11
# define RXDBGTST    1

#define RXENV       12  /* System Environment interface */
# define RXENVGET    1  /* Get System Environment Variable */
# define RXENVSET    2  /* Set System Environment Variable */
# define RXCWDGET    3  /* Get Current Working Directory */
# define RXCWDSET    4  /* Set Current Working Directory */

#define RXENDLST     0
#define RXNOOFEXITS 13  /* MUST be 1 more than last exit number */

/* Symbolic return codes for System Exit Handlers */
#define RXEXIT_HANDLED       0
#define RXEXIT_NOT_HANDLED   1
#define RXEXIT_RAISE_ERROR (-1)

#define RXEXIT_DROPPABLE     0x00
#define RXEXIT_NONDROP       0x01

/* Return Codes from RXEXIT interface */

#define RXEXIT_ISREG         0x01
#define RXEXIT_ERROR         0x01
#define RXEXIT_FAILURE       0x02
#define RXEXIT_BADENTRY      1001
#define RXEXIT_NOEMEM        1002
#define RXEXIT_BADTYPE       1003
#define RXEXIT_NOTINIT       1004
#define RXEXIT_OK               0
#define RXEXIT_DUP             10
#define RXEXIT_MAXREG          20
#define RXEXIT_NOTREG          30
#define RXEXIT_NOCANDROP       40
#define RXEXIT_LOADERR         50
#define RXEXIT_NOPROC         127

typedef struct {
   struct {
               unsigned  rxfferr:1 ;
               unsigned  rxffnfnd:1 ;
               unsigned  rxffsub:1 ;
   }                     rxfnc_flags ;
   /* CONST */ PUCHAR    rxfnc_name ;
               USHORT    rxfnc_namel ;
   /* CONST */ PUCHAR    rxfnc_que ;
               USHORT    rxfnc_quel ;
               USHORT    rxfnc_argc ;
   /* CONST */ PRXSTRING rxfnc_argv ;
               RXSTRING  rxfnc_retc ;
} RXFNCCAL_PARM ;

typedef struct {
   struct {
               unsigned rxfcfail:1 ;
               unsigned rxfcerr:1 ;
   }                    rxcmd_flags ;
   /* CONST */ PUCHAR   rxcmd_address ;
               USHORT   rxcmd_addressl ;
   /* CONST */ PUCHAR   rxcmd_dll ;
               USHORT   rxcmd_dll_len ;
   /* CONST */ RXSTRING rxcmd_command ;
               RXSTRING  rxcmd_retc ;
} RXCMDHST_PARM ;

typedef struct {
   RXSTRING rxmsq_retc ;
} RXMSQPLL_PARM ;

typedef struct {
   struct {
               unsigned rxfmlifo:1 ;
   }                    rxmsq_flags ;
   /* CONST */ RXSTRING rxmsq_value ;
} RXMSQPSH_PARM ;

typedef struct {
   ULONG rxmsq_size ;
} RXMSQSIZ_PARM ;

typedef struct {
   RXSTRING rxmsq_name ;
} RXMSQNAM_PARM ;

typedef struct {
   /* CONST */ RXSTRING rxsio_string ;
} RXSIOSAY_PARM ;

typedef struct {
   /* CONST */ RXSTRING rxsio_string ;
} RXSIOTRC_PARM ;

typedef struct {
   RXSTRING rxsiotrd_retc ;
} RXSIOTRD_PARM ;

typedef struct {
   RXSTRING rxsiodtr_retc ;
} RXSIODTR_PARM ;

typedef struct {
   struct {
      unsigned rxfhhalt:1 ;
   }           rxhlt_flags ;
} RXHLTTST_PARM ;

typedef struct {
   struct {
      unsigned rxftrace:1 ;
   }           rxtrx_flags ;
} RXTRCTST_PARM ;

typedef struct {
   RXSTRING rxenv_name ;
   RXSTRING rxenv_value ;
} RXENVGET_PARM ;

typedef struct {
   RXSTRING rxenv_name ;
   RXSTRING rxenv_value ;
} RXENVSET_PARM ;

typedef struct {
   RXSTRING rxcwd_value ;
} RXCWDGET_PARM ;

typedef struct {
   RXSTRING rxcwd_value ;
} RXCWDSET_PARM ;

APIRET APIENTRY RexxRegisterExitExe(
                PCSZ             EnvName,
#ifdef RX_WEAKTYPING
                PFN              EntryPoint,
#else
                RexxExitHandler *EntryPoint,
#endif
    /* CONST */ PUCHAR           UserArea )
EXTNAME("RexxRegisterExitExe");
#define REXXREGISTEREXITEXE RexxRegisterExitExe

APIRET APIENTRY RexxRegisterExitDll(
                PCSZ   EnvName,
                PCSZ   ModuleName,
                PCSZ   ProcedureName,
    /* CONST */ PUCHAR UserArea,
                ULONG  DropAuth )
EXTNAME("RexxRegisterExitDll");
#define REXXREGISTEREXITDLL RexxRegisterExitDll

APIRET APIENTRY RexxDeregisterExit(
                PCSZ EnvName,
                PCSZ ModuleName )
EXTNAME("RexxDeregisterExit");
#define REXXDEREGISTEREXIT RexxDeregisterExit

APIRET APIENTRY RexxQueryExit (
                PCSZ    ExitName,
                PCSZ    ModuleName,
                PUSHORT Flag,
                PUCHAR  UserArea)
EXTNAME("RexxQueryExit");
#define REXXQUERYEXIT RexxQueryExit

#endif /* INCL_RXSYSEXIT */


#define MAXENVNAMELEN 32767   /* at least, there is no limit */

#define MAKERXSTRING(x,c,l)   ((x).strptr=(c),(x).strlength=(l))
#define RXNULLSTRING(x)       (!(x).strptr)
#define RXSTRLEN(x)           ((x).strptr ? (x).strlength : 0UL)
#define RXSTRPTR(x)           ((x).strptr)
#define RXVALIDSTRING(x)      ((x).strptr && (x).strlength)
#define RXZEROLENSTRING(x)    ((x).strptr && !(x).strlength)

#define RXCOMMAND         0
#define RXSUBROUTINE      1
#define RXFUNCTION        2
/*
 * RXRESTRICTED is OR'ed with one of the above values to run Regina
 * in "restricted" mode
 */
#define RXRESTRICTED    256


#ifdef INCL_RXSHV

/*
 * Definitions and declarations for the Rexx variable pool interface.
 */

# define RXSHV_OK       0x00    /* Everything OK */
# define RXSHV_NEWV     0x01    /* Var not previously set */
# define RXSHV_LVAR     0x02    /* Last var in a NEXTV sequence */
# define RXSHV_TRUNC    0x04    /* Name or value has been truncated */
# define RXSHV_BADN     0x08    /* Bad/invalid name */
# define RXSHV_MEMFL    0x10    /* Memory problem, e.g. out of memory */
# define RXSHV_BADF     0x80    /* Invalid function code */

# define RXSHV_NOAVL    0x90    /* Interface is not available */

/*
 * Note: Some documentation claims that RXSHV_BADF should have the
 *       value 0x80, but that can't be correct, can it?
 */

# define RXSHV_SET      0x00    /* Set variable */
# define RXSHV_FETCH    0x01    /* Get value of variable */
# define RXSHV_DROPV    0x02    /* Drop variable */
# define RXSHV_SYSET    0x03    /* Set symbolic variable */
# define RXSHV_SYFET    0x04    /* Get value of symbolic variable */
# define RXSHV_SYDRO    0x05    /* Drop symbolic variable */
# define RXSHV_NEXTV    0x06    /* Get next var in a NEXTV sequence */
# define RXSHV_PRIV     0x07    /* Get private information */
# define RXSHV_EXIT     0x08    /* Set function exit value */

/*
 * Note: A symbolic variable is a symbol that will be 'expanded' before
 *       use, while a (normal) variable is used directly, without
 *       expanding it first.
 */

typedef struct shvnode
{
   struct shvnode *shvnext ;
   RXSTRING        shvname ;
   RXSTRING        shvvalue ;
   ULONG           shvnamelen ;
   ULONG           shvvaluelen ;
   UCHAR           shvcode ;
   UCHAR           shvret ;
} SHVBLOCK ;

typedef SHVBLOCK *PSHVBLOCK ;

APIRET APIENTRY RexxVariablePool(
                PSHVBLOCK  RequestBlockList )
EXTNAME("RexxVariablePool");
#define REXXVARIABLEPOOL RexxVariablePool

#endif /* INCL_RXSHV */


APIRET APIENTRY RexxFreeMemory(
                PVOID      MemoryBlock )
EXTNAME("RexxFreeMemory");
#define REXXFREEMEMORY RexxFreeMemory

PVOID APIENTRY  RexxAllocateMemory(
                ULONG      size )
EXTNAME("RexxAllocateMemory");
#define REXXALLOCATEMEMORY RexxAllocateMemory


APIRET APIENTRY RexxStart(
                LONG       ArgCount,
    /* CONST */ PRXSTRING  ArgList,
                PCSZ       ProgramName,
    /* CONST */ PRXSTRING  Instore,
                PCSZ       EnvName,
                LONG       CallType,
    /* CONST */ PRXSYSEXIT Exits,
                PSHORT     ReturnCode,
                PRXSTRING  Result )
EXTNAME("RexxStart");
#define REXXSTART RexxStart

/*
 * Return codes .... haha there doesn't seem to be a standard, except
 * zero is OK, negative is an Rexx error, the positive error codes below
 * are my own 'invention'
 */
#define RX_START_OK         0
#define RX_START_BADP       1  /* Bad parameters */
#define RX_START_UNIMPL     2  /* Not yet implemented :-) */
#define RX_START_TOOMANYP   3  /* Too many parameters */
#define RX_DIDNT_START      4  /* Unable to start interpreter */

APIRET APIENTRY RexxCallBack(
                PCSZ       ProcedureName,
                LONG       ArgCount,
                PRXSTRING  ArgList,
                PSHORT     ReturnCode,
                PRXSTRING  Result )
EXTNAME("RexxCallBack");
#define REXXCALLBACK RexxCallBack

/*
 * Return codes for RexxCallBack
 */
#define RX_CB_OK         0
#define RX_CB_BADP       1  /* Bad parameters */
#define RX_CB_NOTSTARTED 2  /* Interface not running */
#define RX_CB_TOOMANYP   3  /* Too many parameters */
#define RX_CB_BADN       8  /* Procedure not found */
/*
 * -------------------------------------------------------------------
 * Sub-command Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXSUBCOM

APIRET APIENTRY RexxRegisterSubcomExe(
                PCSZ               EnvName,
#ifdef RX_WEAKTYPING
                PFN                EntryPoint,
#else
                RexxSubcomHandler *EntryPoint,
#endif
    /* CONST */ PUCHAR             UserArea )
EXTNAME("RexxRegisterSubcomExe");
#define REXXREGISTERSUBCOMEXE RexxRegisterSubcomExe

APIRET APIENTRY RexxDeregisterSubcom(
                PCSZ EnvName,
                PCSZ ModuleName )
EXTNAME("RexxDeregisterSubcom");
#define REXXDEREGISTERSUBCOM RexxDeregisterSubcom

APIRET APIENTRY RexxRegisterSubcomDll(
                PCSZ   EnvName,
                PCSZ   ModuleName,
                PCSZ   ProcedureName,
    /* CONST */ PUCHAR UserArea,
                ULONG  DropAuth )
EXTNAME("RexxRegisterSubcomDll");
#define REXXREGISTERSUBCOMDLL RexxRegisterSubcomDll

APIRET APIENTRY RexxQuerySubcom(
                PCSZ    Envname,
                PCSZ    ModuleName,
                PUSHORT Flag,       /* Documentation diverges ... */
                PUCHAR  UserArea )
EXTNAME("RexxQuerySubcom");
#define REXXQUERYSUBCOM RexxQuerySubcom

#define RXSUBCOM_OK          0
#define RXSUBCOM_DUP        10
#define RXSUBCOM_MAXREG     20
#define RXSUBCOM_NOTREG     30
#define RXSUBCOM_NOCANDROP  40
#define RXSUBCOM_LOADERR    50
#define RXSUBCOM_NOPROC    127

#define RXSUBCOM_BADENTRY 1001
#define RXSUBCOM_NOEMEM   1002
#define RXSUBCOM_BADTYPE  1003
#define RXSUBCOM_NOTINIT  1004

/* I don't know the 'real' values of these */
#define RXSUBCOM_ERROR      0x01
#define RXSUBCOM_ISREG      0x01
#define RXSUBCOM_FAILURE    0x02

#define RXSUBCOM_DROPPABLE  0x00
#define RXSUBCOM_NONDROP    0x01

#endif /* INCL_RXSUBCOM */


/*
 * -------------------------------------------------------------------
 * External Function Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXFUNC

#define RXFUNC_OK             0
#define RXFUNC_DEFINED       10
#define RXFUNC_NOMEM         20
#define RXFUNC_NOTREG        30
#define RXFUNC_MODNOTFND     40
#define RXFUNC_ENTNOTFND     50
#define RXFUNC_NOTINIT       60
#define RXFUNC_BADTYPE       70
#define RXFUNC_NOEMEM      1002

APIRET APIENTRY RexxRegisterFunctionExe(
                PCSZ                 name,
#ifdef RX_WEAKTYPING
                PFN                  EntryPoint )
#else
                RexxFunctionHandler *EntryPoint )
#endif
EXTNAME("RexxRegisterFunctionExe");
#define REXXREGISTERFUNCTIONEXE RexxRegisterFunctionExe

APIRET APIENTRY RexxRegisterFunctionDll(
                PCSZ ExternalName,
                PCSZ LibraryName,
                PCSZ InternalName )
EXTNAME("RexxRegisterFunctionDll");
#define REXXREGISTERFUNCTIONDLL RexxRegisterFunctionDll

APIRET APIENTRY RexxDeregisterFunction(
                PCSZ name )
EXTNAME("RexxDeregisterFunction");
#define REXXDEREGISTERFUNCTION RexxDeregisterFunction

APIRET APIENTRY RexxQueryFunction(
                PCSZ name )
EXTNAME("RexxQueryFunction");
#define REXXQUERYFUNCTION RexxQueryFunction

#endif /* INCL_RXFUNC */

/*
 * -------------------------------------------------------------------
 * Asynchronous Request Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXARI

/***    Return Codes from Asynchronous Request interface */

#define RXARI_OK                   0
#define RXARI_NOT_FOUND            1
#define RXARI_PROCESSING_ERROR     2

APIRET APIENTRY RexxSetHalt(
                LONG pid,
                LONG tid)
EXTNAME("RexxSetHalt");
#define REXXSETHALT RexxSetHalt

APIRET APIENTRY RexxSetTrace(
                LONG pid,
                LONG tid)
EXTNAME("RexxSetTrace");
#define REXXSETTRACE RexxSetTrace

APIRET APIENTRY RexxResetTrace(
                LONG pid,
                LONG tid)
EXTNAME("RexxResetTrace");
#define REXXRESETTRACE RexxResetTrace

#endif /* INCL_RXARI */

/*
 * -------------------------------------------------------------------
 * External Queue Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXQUEUE

ULONG  APIENTRY RexxCreateQueue (
                PSZ,
                ULONG,
                PSZ,
                ULONG* )
EXTNAME("RexxCreateQueue");
#define REXXCREATEQUEUE RexxCreateQueue

ULONG  APIENTRY RexxDeleteQueue (
                PSZ )
EXTNAME("RexxDeleteQueue");
#define REXXDELETEQUEUE RexxDeleteQueue

ULONG  APIENTRY RexxQueryQueue (
                PSZ,
                ULONG* )
EXTNAME("RexxQueryQueue");
#define REXXQUERYQUEUE RexxQueryQueue

ULONG  APIENTRY RexxAddQueue (
                PSZ,
                PRXSTRING,
                ULONG )
EXTNAME("RexxAddQueue");
#define REXXADDQUEUE RexxAddQueue

ULONG  APIENTRY RexxPullQueue (
                PSZ,
                PRXSTRING,
                PDATETIME,
                ULONG )
EXTNAME("RexxPullQueue");
#define REXXPULLQUEUE RexxPullQueue

/* Request flags for External Data Queue access --------------------- */
#define RXQUEUE_FIFO          0    /* Access queue first-in-first-out */
#define RXQUEUE_LIFO          1    /* Access queue last-in-first-out  */

#define RXQUEUE_NOWAIT        0    /* Wait for data if queue empty    */
#define RXQUEUE_WAIT          1    /* Don't wait on an empty queue    */

/* Return Codes from RxQueue interface ------------------------------ */
#define RXQUEUE_OK            0        /* Successful return           */
#define RXQUEUE_NOTINIT       1000     /* Queues not initialized      */

#define RXQUEUE_STORAGE       1        /* Ret info buf not big enough */
#define RXQUEUE_SIZE          2        /* Data size > 64K-64          */
#define RXQUEUE_DUP           3        /* Attempt-duplicate queue name*/
#define RXQUEUE_NOEMEM        4        /* Not enough available memory */
#define RXQUEUE_BADQNAME      5        /* Not a valid queue name      */
#define RXQUEUE_PRIORITY      6        /* Not accessed as LIFO|FIFO   */
#define RXQUEUE_BADWAITFLAG   7        /* Not accessed as WAIT|NOWAIT */
#define RXQUEUE_EMPTY         8        /* No data in queue            */
#define RXQUEUE_NOTREG        9        /* Queue does not exist        */
#define RXQUEUE_ACCESS       10        /* Queue busy and wait active  */
#define RXQUEUE_MAXREG       11        /* No memory to create a queue */
#define RXQUEUE_MEMFAIL      12        /* Failure in memory management*/
#define RXQUEUE_NETERROR    100        /* Network error               */

#endif /* INCL_RXQUEUE */

/*
 * -------------------------------------------------------------------
 * Macrospace Interface
 *-------------------------------------------------------------------
 */
#ifdef INCL_RXMACRO

APIRET APIENTRY RexxAddMacro (
                PSZ,                         /* Function to add or change    */
                PSZ,                         /* Name of file to get function */
                ULONG  )                     /* Flag indicating search pos   */
EXTNAME("RexxAddMacro");
#define REXXADDMACRO  RexxAddMacro

APIRET APIENTRY RexxDropMacro (
                PSZ )                         /* Name of function to remove  */
EXTNAME("RexxDropMacro");
#define REXXDROPMACRO  RexxDropMacro

APIRET APIENTRY RexxSaveMacroSpace (
                ULONG ,                      /* Argument count (0==save all) */
                PSZ *,                       /* List of funct names to save  */
                PSZ)                         /* File to save functions in    */
EXTNAME("RexxSaveMacroSpace");
#define REXXSAVEMACROSPACE  RexxSaveMacroSpace

APIRET APIENTRY RexxLoadMacroSpace (
                ULONG ,                      /* Argument count (0==load all) */
                PSZ *,                       /* List of funct names to load  */
                PSZ)                        /* File to load functions from  */
EXTNAME("RexxLoadMacroSpace");
#define REXXLOADMACROSPACE  RexxLoadMacroSpace

APIRET APIENTRY RexxQueryMacro (
                PSZ,                         /* Function to search for       */
                PUSHORT )                   /* Ptr for position flag return */
EXTNAME("RexxQueryMacro");
#define REXXQUERYMACRO  RexxQueryMacro

APIRET APIENTRY RexxReorderMacro(
                PSZ,                         /* Name of funct change order   */
                ULONG  )                    /* New position for function    */
EXTNAME("RexxReorderMacro");
#define REXXREORDERMACRO  RexxReorderMacro

APIRET APIENTRY RexxClearMacroSpace(
                )
EXTNAME("RexxClearMacroSpace");
#define REXXCLEARMACROSPACE  RexxClearMacroSpace

/* Registration Search Order Flags ---------------------------------- */
#define RXMACRO_SEARCH_BEFORE       1  /* Beginning of search order   */
#define RXMACRO_SEARCH_AFTER        2  /* End of search order         */

/* Return Codes from RxMacroSpace interface ------------------------- */
#define RXMACRO_OK                 0  /* Macro interface completed    */
#define RXMACRO_NO_STORAGE         1  /* Not Enough Storage Available */
#define RXMACRO_NOT_FOUND          2  /* Requested function not found */
#define RXMACRO_EXTENSION_REQUIRED 3  /* File ext required for save   */
#define RXMACRO_ALREADY_EXISTS     4  /* Macro functions exist        */
#define RXMACRO_FILE_ERROR         5  /* File I/O error in save/load  */
#define RXMACRO_SIGNATURE_ERROR    6  /* Incorrect format for load    */
#define RXMACRO_SOURCE_NOT_FOUND   7  /* Requested cannot be found    */
#define RXMACRO_INVALID_POSITION   8  /* Invalid search order pos     */
#define RXMACRO_NOT_INIT           9  /* API not initialized          */

#endif /* INCL_RXMACRO */


/* REGINA EXTENSIONS *********************************************************/
/* The following functions are an extension to the standard. Never try to
 * address the function directly if you want to be compatible. Use the dynamic
 * linking machanism of your operating system instead.
 *
 *
 * ReginaVersion returns informations about the version of the library.
 * This function was introduced in version 2.0.
 * Returns: ULONG, in lower byte the two-digit fraction part of the version.
 *          The higher bytes will hold the integer part of the version.
 *          Examples: 0x10A codes the Version "1.10".
 * VersionString will be filled if VersionString is non-NULL.
 * If VersionString is non-NULL then there are two possibilities:
 * a) VersionString->strlength == 0: VersionString is filled with the
 *                                   appropriate values. VersionString->strptr
 *                                   is always created.
 * b) VersionString->strlength != 0: VersionString->strptr is filled up to
 *                                   this value. VersionString->strlength will
 *                                   hold the copied bytes.
 * Note: A terminating ASCII-zero is appended if there is enough space
 *       although it is never counted in VersionString.strlength.
 *       RexxAllocateMemory is used if needed.
 */
APIRET APIENTRY ReginaVersion(
                PRXSTRING VersionString )
EXTNAME("ReginaVersion");
#define REGINAVERSION ReginaVersion

/*
 * ReginaCleanup performs a graceful cleanup. This is done automatically
 * on many systems but you can't be sure in all cases. The cleanup operations
 * destroys all informations that are collected so far by one thread.
 * It works for a single process or thread too. Although all memory is
 * freed, some open handles may remain open. It is necessary to perform
 * the appropriate "RexxDeregister<whatever>" calls before this routine
 * is called.
 * This function was introduced in version 3.3.
 * Returns: ULONG, 0 if this call hasn't done anything.
 *          1 if at least something could be freed.
 * It is allowed to reuse every API function after this call but this routine
 * must not be used when some parts of the Regina core are in use.
 */
APIRET APIENTRY ReginaCleanup( )
EXTNAME("ReginaCleanup");
#define REGINACLEANUP ReginaCleanup

#ifdef OREXX_BINARY_COMPATIBLE
# pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif /* __REXXSAA_H_INCLUDED */
