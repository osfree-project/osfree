/*********************************************************************\
*
* Module Name: REXXSAA.H
*
* REXXSAA Common Definitions file
*
* @Copyright  1989 IBM Corporation
* Licensed Materials - Property of IBM
*
\*********************************************************************/
 
#ifndef REXXSAA_INCLUDED
#define REXXSAA_INCLUDED
 
#ifndef OS2_INCLUDED
#include <os21x\os2.h>
#endif
 
#ifdef INCL_REXXSAA
#define INCL_RXSUBCOM
#define INCL_RXSHV
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXMACRO
#define INCL_RXARI
#endif /* INCL_REXXSAA */
 
/* System exits definitions require subcommand definitions */
 
#ifdef  INCL_RXSYSEXIT
#define INCL_RXSUBCOM
#endif
 
/***    Structure for external interface string (RXSTRING) */
 
 
#define RXAUTOBUFLEN         256L
 
#define RXNULLSTRING(r)      (!(r).strptr)
#define RXZEROLENSTRING(r)   ((r).strptr && !(r).strlength)
#define RXVALIDSTRING(r)     ((r).strptr && (r).strlength)
#define RXSTRLEN(r)          (RXNULLSTRING(r)?0L:(r).strlength)
#define RXSTRPTR(r)          (r).strptr
#define MAKERXSTRING(r,p,l)  (r).strptr=(PCH)p;(r).strlength=(ULONG)l
 
 
typedef struct {
   ULONG           strlength;          /*   length of string         */
   PCH             strptr;             /*   far pointer to string    */
   } RXSTRING;
 
typedef RXSTRING FAR *PRXSTRING;       /* pointer to a RXSTRING      */
 
 
 
 
 
/***    Structure for system exit block (RXSYSEXIT) */
 
typedef struct {
   PSZ   sysexit_name;                 /* subcom enviro for sysexit  */
   SHORT sysexit_code;                 /* sysexit function code      */
   } RXSYSEXIT;
 
typedef RXSYSEXIT FAR *PRXSYSEXIT;     /* pointer to a RXSYSEXIT     */
 
 
/***    Main Entry Point to the REXXSAA Interpreter */
#define RXCOMMAND       0              /* Program called as Command  */
#define RXSUBROUTINE    1              /* Program called as Subroutin*/
#define RXFUNCTION      2              /* Program called as Function */
 
 
SHORT APIENTRY REXXSAA (
         SHORT,                        /* Num of args passed to rexx */
         PRXSTRING,                    /* Array of args passed to rex*/
         PSZ,                          /* [d:][path] filename[.ext]  */
         PRXSTRING,                    /* Loc of rexx proc in memory */
         PSZ,                          /* ASCIIZ initial environment.*/
         SHORT,                        /* type (command,subrtn,funct)*/
         PRXSYSEXIT,                   /* SysExit env. names &  codes*/
         PSHORT,                       /* Ret code from if numeric   */
         PRXSTRING );                  /* Retvalue from the rexx proc*/
 
 
 
/***    Subcommand Interface */
#ifdef INCL_RXSUBCOM
 
 
/***    Drop Authority for RXSUBCOM interface */
 
#define RXSUBCOM_DROPPABLE   0x00     /* handler to be dropped by all*/
#define RXSUBCOM_NONDROP     0x01     /* process with same PID as the*/
                                      /* registrant may drop environ */
 
/***    Return Codes from RXSUBCOM interface */
 
#define RXSUBCOM_ISREG       0x01     /* Subcommand is registered    */
#define RXSUBCOM_ERROR       0x01     /* Subcommand Ended in Error   */
#define RXSUBCOM_FAILURE     0x02     /* Subcommand Ended in Failure */
#define RXSUBCOM_BADENTRY    1001     /* Invalid Entry Conditions    */
#define RXSUBCOM_NOEMEM      1002     /* Insuff stor to complete req */
#define RXSUBCOM_BADTYPE     1003     /* Bad registration type.      */
#define RXSUBCOM_NOTINIT     1004     /* API system not initialized. */
#define RXSUBCOM_OK           0       /* Function Complete           */
#define RXSUBCOM_DUP         10       /* Duplicate Environment Name- */
                                      /* but Registration Completed  */
#define RXSUBCOM_MAXREG      20       /* Cannot register more blocks */
#define RXSUBCOM_NOTREG      30       /* Name Not Registered         */
#define RXSUBCOM_NOCANDROP   40       /* Name not droppable          */
#define RXSUBCOM_LOADERR     50       /* Could not load function     */
#define RXSUBCOM_NOPROC     127       /* RXSUBCOM routine - not found*/
 
 
/***    Structure of Rexx Subcomand Block (SCBLOCK) */
 
typedef struct subcom_node {
    struct subcom_node far *next;      /* pointer to the next block  */
    PSZ    scbname;                    /* subcom environment name    */
    PSZ    scbdll_name;                /* subcom module name         */
    PSZ    scbdll_proc;                /* subcom procedure name      */
    double scbuser;                    /* user area                  */
    PFN    scbaddr;                    /* subcom environment address */
    USHORT scbmod_handle;              /* dynalink module handle     */
    USHORT scbdrop_auth;               /* Permission to drop         */
    PID    scbpid;                     /* Pid of Registrant          */
    USHORT scbsid;                     /* Session ID.                */
    } SCBLOCK;
 
typedef SCBLOCK FAR *PSCBLOCK;
 
 
/***   RxSubcomRegister - Register environment as Subcommand handler */
 
USHORT APIENTRY RxSubcomRegister(
         PSCBLOCK );                   /* Pointer to SCBLOCK         */
 
 
/***    RxSubcomQuery - Query an environment for Existance */
 
USHORT APIENTRY RxSubcomQuery(
         PSZ,                          /* Name of the Environment    */
         PSZ,                          /* DLL Module Name            */
         PUSHORT,                      /* Stor for existance code    */
         double far *);                /* Stor for user word         */
 
 
/***    RxSubcomLoad - Load a Subcommand environment */
 
USHORT APIENTRY RxSubcomLoad(
         PSZ,                          /* Name of the Environment    */
         PSZ);                         /* DLL Module Name            */
 
 
/***    RxSubcomDrop - Drop an Subcommand environment */
 
USHORT APIENTRY RxSubcomDrop(
         PSZ,                          /* Name of the Environment    */
         PSZ);                         /* DLL Module Name            */
 
 
/***    RxSubcomExecute - Execute a command in an environment */
 
USHORT APIENTRY RxSubcomExecute(
         PSZ,                          /* Name of Subcommand Environ */
         PSZ,                          /* Module name of its' DLL    */
         PRXSTRING,                    /* Command string to be passed*/
         PUSHORT,                      /* Stor for error flag notice */
         PUSHORT,                      /* Stor for rc from handler   */
         PRXSTRING );                  /* Stor for returned string   */
 
#endif /* INCL_RXSUBCOM */
 
 
 
/***    Shared Variable Pool Interface */
#ifdef INCL_RXSHV
 
/***    Function Codes for Variable Pool Interface (shvcode) */
 
#define RXSHV_SET          0x00       /* Set var from given value    */
#define RXSHV_FETCH        0x01       /* Copy value of var to buffer */
#define RXSHV_DROPV        0x02       /* Drop variable               */
#define RXSHV_SYSET        0x03       /* Symbolic name Set variable  */
#define RXSHV_SYFET        0x04       /* Symbolic name Fetch variable*/
#define RXSHV_SYDRO        0x05       /* Symbolic name Drop variable */
#define RXSHV_NEXTV        0x06       /* Fetch "next" variable       */
#define RXSHV_PRIV         0x07       /* Fetch private information   */
#define RXSHV_EXIT         0x08       /* Set function exit value     */
 
 
/***    Return Code Flags for Variable Pool Interface (shvret) */
 
#define RXSHV_OK           0x00       /* Execution was OK            */
#define RXSHV_NEWV         0x01       /* Variable did not exist      */
#define RXSHV_LVAR         0x02       /* Last var trans via SHVNEXTV */
#define RXSHV_TRUNC        0x04       /* Truncation occurred-Fetch   */
#define RXSHV_BADN         0x08       /* Invalid variable name       */
#define RXSHV_MEMFL        0x10       /* Out of memory failure       */
#define RXSHV_BADF         0x80       /* Invalid funct code (shvcode)*/
#define RXSHV_NOAVL        0x90       /* Interface not available     */
 
 
/***    Structure of Shared Variable Request Block (SHVBLOCK) */
 
typedef struct shvnode {
    struct shvnode FAR *shvnext;      /* pointer to the next block   */
    RXSTRING           shvname;       /* Pointer to the name buffer  */
    RXSTRING           shvvalue;      /* Pointer to the value buffer */
    ULONG              shvnamelen;    /* Length of the name value    */
    ULONG              shvvaluelen;   /* Length of the fetch value   */
    UCHAR              shvcode;       /* Function code for this block*/
    UCHAR              shvret;        /* Individual Return Code Flags*/
    } SHVBLOCK;
 
typedef SHVBLOCK FAR *PSHVBLOCK;
 
 
/***    RxVar - Request Variable Pool Service */
 
USHORT APIENTRY RxVar(
         PSHVBLOCK);                  /* Pointer to list of SHVBLOCKs*/
 
#endif /* INCL_RXSHV */
 
 
/***    External Function Interface */
#ifdef INCL_RXFUNC
 
/***    Registration Type Identifiers for Available Function Table */
 
#define RXFUNC_DYNALINK       1        /* Function Available in DLL  */
#define RXFUNC_CALLENTRY      2        /* Registered as mem entry pt.*/
 
 
/***    Return Codes from RxFunction interface */
 
 
#define RXFUNC_OK             0        /* REXX-API Call Successful   */
#define RXFUNC_DEFINED       10        /* Function Defined in AFT    */
#define RXFUNC_NOMEM         20        /* Not Enough Mem to Add      */
#define RXFUNC_NOTREG        30        /* Funct Not Registered in AFT*/
#define RXFUNC_MODNOTFND     40        /* Funct Dll Module Not Found */
#define RXFUNC_ENTNOTFND     50        /* Funct Entry Point Not Found*/
#define RXFUNC_NOTINIT       60        /* API not initialized        */
#define RXFUNC_BADTYPE       70        /* Bad function type          */
 
 
/***    RxFunctionRegister - Register a function in the AFT */
 
USHORT APIENTRY RxFunctionRegister (
        PSZ,                           /* Name of function to add    */
        PSZ,                           /* Dll file name (if in dll)  */
        PSZ,                           /* Entry in dll OR mem address*/
        USHORT);                       /* RX_DYNALINK || RX_CALLENTRY*/
 
 
/***    RxFunctionDeregister - Delete a function from the AFT */
 
USHORT APIENTRY RxFunctionDeregister (
        PSZ );                         /* Name of function to remove */
 
 
/***    RxFunctionQuery - Scan the AFT for a function */
 
USHORT APIENTRY RxFunctionQuery (
        PSZ );                         /* Name of function to find   */
 
 
/***    RxFunctionCall - Call a function in the AFT */
 
USHORT APIENTRY RxFunctionCall (
        PSZ,                           /* Name of function to call   */
        USHORT,                        /* Number of arguments        */
        PRXSTRING,                     /* Array of argument strings  */
        PUSHORT,                       /* RC from function called    */
        PRXSTRING,                     /* Storage for returned data  */
        PSZ );                         /* Name of active data queue  */
 
 
#endif /* INCL_RXFUNC */
 
 
/***   System Exits */
#ifdef INCL_RXSYSEXIT
 
 
/* System Exit function and sub-function definitions */
 
#define RXENDLST    0                 /* End of exit list.           */
#define RXFNC    2                    /* Process external functions. */
#define    RXFNCCAL 1                 /* subcode value.              */
#define RXCMD    3                    /* Process host commands.      */
#define    RXCMDHST 1                 /* subcode value.              */
#define RXMSQ    4                    /* Manipulate queue.           */
#define    RXMSQPLL 1                 /* Pull a line from queue      */
#define    RXMSQPSH 2                 /* Place a line on queue       */
#define    RXMSQSIZ 3                 /* Return num of lines on queue*/
#define    RXMSQNAM 20                /* Set active queue name       */
#define RXSIO    5                    /* Session I/O.                */
#define    RXSIOSAY 1                 /* SAY a line to STDOUT        */
#define    RXSIOTRC 2                 /* Trace output                */
#define    RXSIOTRD 3                 /* Read from char stream       */
#define    RXSIODTR 4                 /* DEBUG read from char stream */
#define    RXSIOTLL 5                 /* Return linelength(N/A OS/2) */
#define RXHLT    7                    /* Halt processing.            */
#define    RXHLTCLR 1                 /* Clear HALT indicator        */
#define    RXHLTTST 2                 /* Test HALT indicator         */
#define RXTRC    8                    /* Test ext trace indicator.   */
#define    RXTRCTST 1                 /* subcode value.              */
#define RXINI    9                    /* Initialization processing.  */
#define    RXINIEXT 1                 /* subcode value.              */
#define RXTER   10                    /* Termination processing.     */
#define    RXTEREXT 1                 /* subcode value.              */
#define RXNOOFEXITS 11                /* 1 + largest exit number.    */
 
typedef PUCHAR PEXIT;                 /* ptr to exit parameter block */
 
/***    Subfunction RXFNCCAL - External Function Calls */
 
typedef struct {
   struct {
      unsigned rxfferr  : 1;           /* Invalid call to routine.   */
      unsigned rxffnfnd : 1;           /* Function not found.        */
      unsigned rxffsub  : 1;           /* Called as a subroutine if  */
                                       /* set.  Return values are    */
                                       /* optional for subroutines,  */
                                       /* required for functions.    */
      } rxfnc_flags ;
   PUCHAR            rxfnc_name;       /* Pointer to function name.  */
   USHORT            rxfnc_namel;      /* Length of function name.   */
   PUCHAR            rxfnc_que;        /* Current queue name.        */
   USHORT            rxfnc_quel;       /* Length of queue name.      */
   USHORT            rxfnc_argc;       /* Number of args in list.    */
   PRXSTRING         rxfnc_argv;       /* Pointer to argument list.  */
                                       /* List mimics argv list in   */
                                       /* REXXSAA -- array of        */
                                       /* RXSTRINGs.                 */
   RXSTRING          rxfnc_retc;       /* Return value.              */
   } RXFNCCAL_PARM;
 
 
/***    Subfunction RXCMDHST -- Process Host Commands     */
 
typedef struct {
   struct {                            /* Condition flags            */
      unsigned rxfcfail : 1;           /* Command failed.  Trap with */
                                       /* CALL or SIGNAL on FAILURE. */
      unsigned rxfcerr  : 1;           /* Command ERROR occurred.    */
                                       /* Trap with CALL or SIGNAL on*/
                                       /* ERROR.                     */
      } rxcmd_flags;
   PUCHAR            rxcmd_address;    /* Pointer to address name.   */
   USHORT            rxcmd_addressl;   /* Length of address name.    */
   PUCHAR            rxcmd_dll;        /* dll name for command.      */
   USHORT            rxcmd_dll_len;    /* Length of dll name.  0 ==> */
                                       /* .EXE file.                 */
   RXSTRING          rxcmd_command;    /* The command string.        */
   RXSTRING          rxcmd_retc;       /* Pointer to return code     */
                                       /* buffer.  User allocated.   */
   } RXCMDHST_PARM;
 
 
/***     Subfunction RXMSQPLL -- Pull Entry from Queue */
 
typedef struct {
   RXSTRING          rxmsq_retc;       /* Pointer to dequeued entry  */
                                       /* buffer.  User allocated.   */
   } RXMSQPLL_PARM;
 
 
/***    Subfunction RXMSQPSH -- Push Entry on Queue */
 
typedef struct {
   struct {                            /* Operation flag             */
      unsigned rxfmlifo : 1;           /* Stack entry LIFO if set,   */
                                       /* FIFO if reset.             */
      } rxmsq_flags;
   RXSTRING          rxmsq_value;      /* The entry to be pushed.    */
   } RXMSQPSH_PARM;
 
 
/***    Subfunction RXMSQSIZ -- Return the Current Queue Size */
 
typedef struct {
   ULONG             rxmsq_size;       /* Number of Lines in Queue   */
   } RXMSQSIZ_PARM;
 
 
/***    Subfunction RXMXQNAM -- Set Current Queue Name */
 
typedef struct {
   SEL               rxmsq_name;       /* Selector containing ASCIIZ */
                                       /* queue name.  Change length */
                                       /* with DosReallocSeg if      */
                                       /* required.                  */
   } RXMSQNAM_PARM;
 
 
/***    Subfunction RXSIOSAY -- Perform SAY Clause */
 
typedef struct {
   RXSTRING          rxsio_string;     /* String to display.         */
   } RXSIOSAY_PARM;
 
 
/***    Subfunction RXSIOTRC -- Write Trace Output */
 
typedef struct {
  RXSTRING           rxsio_string;     /* Trace line to display.     */
  } RXSIOTRC_PARM;
 
 
/***    Subfunction RXSIOTRD -- Read Input from the Terminal */
 
typedef struct {
   RXSTRING          rxsiotrd_retc;    /* RXSTRING for output.  Note:*/
                                       /* user allocates output      */
                                       /* buffer with DosAllocSeg()  */
                                       /* or DosAllocHuge().         */
   } RXSIOTRD_PARM;
 
 
/***    Subfunction RXSIODTR -- Read Debug Input from the Terminal */
 
typedef struct {
   RXSTRING          rxsiodtr_retc;    /* RXSTRING for output.  Note:*/
                                       /* user allocates output      */
                                       /* buffer with DosAllocSeg()  */
                                       /* or DosAllocHuge().         */
   } RXSIODTR_PARM;
 
 
/***    Subfunction RXHSTTST -- Test for HALT Condition */
 
typedef struct {
   struct {                            /* Halt flag                  */
      unsigned rxfhhalt : 1;           /* Set if HALT occurred.      */
      } rxhlt_flags;
   } RXHLTTST_PARM;
 
 
/***    Subfunction RXTRCTST -- Test for TRACE Condition */
 
typedef struct {
   struct {                            /* Trace flags                */
      unsigned rxftrace : 1;           /* Set to run external trace. */
      } rxtrc_flags;
   } RXTRCTST_PARM;
 
 
/***      RxExitRegister - Register a system exit. */
 
USHORT APIENTRY RxExitRegister (
         PSCBLOCK ) ;                  /* Ptr to SCBLOCK to register */
 
 
/***    RxExitDrop - Drop a system exit. */
 
USHORT APIENTRY RxExitDrop (
         PSZ,                          /* Exit name                  */
         PSZ ) ;                       /* DLL module name            */
 
 
/***    RxExitQuery - Query an exit for existance. */
 
USHORT APIENTRY RxExitQuery (
         PSZ,                          /* Exit name                  */
         PSZ,                          /* DLL Module name.           */
         PUSHORT,                      /* Existance flag.            */
         double far * ) ;              /* User data.                 */
 
#endif /* INCL_RXSYSEXIT */
 
 
/***    Asynchronous Request Interface */
#ifdef INCL_RXARI
 
/***    Return Codes from Asynchronous Request interface */
 
#define RXARI_OK                   0  /* Interface completed         */
#define RXARI_NOT_FOUND            1  /* Target program not found    */
#define RXARI_PROCESSING_ERROR     2  /* Error processing request    */
 
/***    RxHaltSet - Request Program Halt */
 
USHORT APIENTRY RxHaltSet(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */
 
/***    RxTraceSet - Request Program Trace */
 
USHORT APIENTRY RxTraceSet(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */
 
/***    RxTraceReset - Turn Off Program Trace */
 
USHORT APIENTRY RxTraceReset(
         PID,                         /* Process Id                  */
         TID);                        /* Thread Id                   */
 
 
#endif /* INCL_RXARI */
 
 
/***    Macro Space Interface */
#ifdef INCL_RXMACRO
 
/***    Registration Search Order Flags */
 
#define RXMACRO_SEARCH_BEFORE       1  /* Beginning of search order  */
#define RXMACRO_SEARCH_AFTER        2  /* End of search order        */
 
 
/***    Return Codes from RxMacroSpace interface */
 
#define RXMACRO_OK                 0  /* Macro interface completed   */
#define RXMACRO_NO_STORAGE         1  /* Not Enough Storage Available*/
#define RXMACRO_NOT_FOUND          2  /* Requested function not found*/
#define RXMACRO_EXTENSION_REQUIRED 3  /* File ext required for save  */
#define RXMACRO_ALREADY_EXISTS     4  /* Macro functions exist       */
#define RXMACRO_FILE_ERROR         5  /* File I/O error in save/load */
#define RXMACRO_SIGNATURE_ERROR    6  /* Incorrect format for load   */
#define RXMACRO_SOURCE_NOT_FOUND   7  /* Requested cannot be found   */
#define RXMACRO_INVALID_POSITION   8  /* Invalid search order pos    */
#define RXMACRO_NOT_INIT           9  /* API not initialized         */
 
 
 
/***    RxMacroChange - Register a function in the Macro Space */
 
USHORT APIENTRY RxMacroChange (
         PSZ,                         /* Function to add/change      */
         PSZ,                         /* Name of file to get function*/
         USHORT );                    /* Flag indicating search pos  */
 
 
/***    RxMacroDrop - Remove a function from the Macro Space */
 
USHORT APIENTRY RxMacroDrop (
         PSZ );                        /* Name of function to remove */
 
 
/***    RxMacroSave - Save Macro Space functions to a file */
 
USHORT APIENTRY RxMacroSave (
         USHORT,                      /* Argument count (0==save all)*/
         PSZ FAR *,                   /* List of funct names to save */
         PSZ );                       /* File to save functions in   */
 
 
/***    RxMacroLoad - Load Macro Space functions from a file */
 
USHORT APIENTRY RxMacroLoad (
         USHORT,                      /* Argument count (0==load all)*/
         PSZ FAR *,                   /* List of funct names to load */
         PSZ );                       /* File to load functions from */
 
 
/***    RxMacroQuery - Find a function's search-order position */
 
USHORT APIENTRY RxMacroQuery (
         PSZ,                         /* Function to search for      */
         PUSHORT );                   /* Ptr for position flag return*/
 
 
/***    RxMacroReOrder - Change a function's search-order position */
 
USHORT APIENTRY RxMacroReOrder(
         PSZ,                         /* Name of funct change order  */
         USHORT );                    /* New position for function   */
 
 
/***    RxMacroErase - Remove all functions from the Macro Space */
 
USHORT APIENTRY RxMacroErase(
         VOID );                      /* No Arguments.               */
 
#endif /* INCL_RXMACRO */
 
#endif /* REXXSAA_INCLUDED */
