/*
 *
 *
 */

#define INCL_DOSMISC
#undef INCL_REXXSAA
#include <os2.h>
#define INCL_REXXSAA

#include "rexxsaa.h"

typedef unsigned short WORD;            // w

typedef WORD FAR *PWORD;                // pw

// To extract offset or selector from any FAR (16:16) pointer
#define OFFSETOF16(p)   (((PWORD)&(p))[0])
#define SEGMENTOF16(p)  (((PWORD)&(p))[1])

// For now, the default operators assume they're working on 16:16 pointers
//#define OFFSETOF        OFFSETOF16
//#define SEGMENTOF       SEGMENTOF16

// To convert a tiled 16:16 address to a 0:32 address
#define MAKEFLATP(fp)   ((PVOID)((SEGMENTOF16(fp)&~7)<<13 | OFFSETOF16(fp)))

typedef char _far16 * PSZ16;

typedef int (APIENTRY16 _PFN16)(char far *);
typedef _PFN16 _Far16 *PFN16;
typedef PSZ16 PCH16;

typedef struct {
   ULONG           strlength;          /*   length of string         */
   PCH16   _Far16  strptr;             /*   far pointer to string    */
   } RXSTRING16;

typedef RXSTRING16 _Far16 *PRXSTRING16;       /* pointer to a RXSTRING      */

typedef struct subcom_node {
    struct subcom_node _far16 *next;      /* pointer to the next block  */
    PSZ16  scbname;                    /* subcom environment name    */
    PSZ16  scbdll_name;                /* subcom module name         */
    PSZ16  scbdll_proc;                /* subcom procedure name      */
    double scbuser;                    /* user area                  */
    PFN16  scbaddr;                    /* subcom environment address */
    USHORT scbmod_handle;              /* dynalink module handle     */
    USHORT scbdrop_auth;               /* Permission to drop         */
    PID    scbpid;                     /* Pid of Registrant          */
    USHORT scbsid;                     /* Session ID.                */
    } SCBLOCK16;

typedef SCBLOCK16 _pascal _far16 *PSCBLOCK16;


//typedef APIRET APIENTRY (*RexxSubcomHandler)(/* CONST */ PRXSTRING, PUSHORT, PRXSTRING);

/*
These functions are undocumented:
RexxLoadSubcom.10,
RexxCallExit.15,
RexxCallSubcom.16,
RexxCallFunction.21,
RexxExecuteMacroFunction.29,
*/

APIRET APIENTRY RexxCreateQueue( PSZ Buffer,
                                 ULONG BuffLen,
                                 PSZ RequestedName,
                                 ULONG* DupFlag);

APIRET APIENTRY RexxDeleteQueue( PSZ QueueName );

APIRET APIENTRY RexxAddQueue( PSZ QueueName,
                              PRXSTRING EntryData,
                              ULONG AddFlag);

APIRET APIENTRY RexxPullQueue( PSZ QueueName,
                               PRXSTRING DataBuf,
                               PDATETIME TimeStamp,
                               ULONG WaitFlag);

APIRET APIENTRY RexxQueryQueue( PSZ QueueName,
                                ULONG* Count);

APIRET APIENTRY RexxRegisterSubcomDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth );

APIRET APIENTRY RexxRegisterSubcomExe(
   PCSZ EnvName,
   RexxSubcomHandler *EntryPoint,
   PUCHAR UserArea );

APIRET APIENTRY RexxQuerySubcom(
   PCSZ Envname,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea );

APIRET APIENTRY RexxDeregisterSubcom(
   PCSZ EnvName,
   PCSZ ModuleName );

APIRET APIENTRY RexxRegisterExitDll(
   PCSZ EnvName,
   PCSZ ModuleName,
   PCSZ ProcedureName,
   PUCHAR UserArea,
   ULONG DropAuth );

APIRET APIENTRY RexxRegisterExitExe(
   PCSZ EnvName,
   RexxExitHandler *EntryPoint,
   PUCHAR UserArea );

APIRET APIENTRY RexxDeregisterExit(
   PCSZ EnvName,
   PCSZ ModuleName );

APIRET APIENTRY RexxQueryExit(
   PCSZ EnvName,
   PCSZ ModuleName,
   PUSHORT Flag,
   PUCHAR UserArea );

APIRET APIENTRY RexxRegisterFunctionDll(
   PCSZ ExternalName,
   PCSZ LibraryName,
   PCSZ InternalName );

APIRET APIENTRY RexxRegisterFunctionExe(
   PCSZ name,
   RexxFunctionHandler *EntryPoint );

APIRET APIENTRY RexxDeregisterFunction(
   PCSZ name );

APIRET APIENTRY RexxQueryFunction(
   PCSZ name );

APIRET APIENTRY RexxAddMacro( PSZ FuncName,
                              PSZ SourceFile,
                              ULONG Position );

APIRET APIENTRY RexxDropMacro( PSZ FuncName);

APIRET APIENTRY RexxClearMacroSpace( VOID );

APIRET APIENTRY RexxLoadMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile);

APIRET APIENTRY RexxQueryMacro( PSZ FuncName,
                                PUSHORT Position );

APIRET APIENTRY RexxReorderMacro( PSZ FuncName,
                                  ULONG Position );

APIRET APIENTRY RexxSaveMacroSpace( ULONG FuncCount,
                                    PSZ * FuncNames,
                                    PSZ MacroLibFile);

USHORT APIENTRY16 RXSUBCOMREGISTER(PSCBLOCK16 PSCB);

USHORT APIENTRY16 RXMACROLOAD (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16 _Far16 *argv,                   /* List of funct names to save */
         PSZ16 file );

USHORT APIENTRY16 RXMACROSAVE (
         USHORT argc,                      /* Argument count (0==save all)*/
         PSZ16 _Far16 *argv,                   /* List of funct names to save */
         PSZ16 file );                       /* File to save functions in   */

USHORT APIENTRY16 RXSUBCOMLOAD(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll);                         /* DLL Module Name            */

USHORT APIENTRY16 RXMACRODROP (
         PSZ16 fn );                        /* Name of function to remove */

USHORT APIENTRY16 RXEXITQUERY (
         PSZ16 fn,                          /* Exit name                  */
         PSZ16 dll,                          /* DLL Module name.           */
         PUSHORT _Far16 flag,                      /* Existance flag.            */
         double _Far16 *data ) ;              /* User data.                 */

USHORT APIENTRY16 RXMACROERASE(
         VOID );                      /* No Arguments.               */

USHORT APIENTRY16 RXSUBCOMDROP(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll);                         /* DLL Module Name            */

USHORT APIENTRY16 RXMACROQUERY (
         PSZ16 fn,                         /* Function to search for      */
         PUSHORT _Far16 flag);             /* Ptr for position flag return*/

USHORT APIENTRY16 RXSUBCOMDROP(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll);                         /* DLL Module Name            */

USHORT APIENTRY16 RXMACROCHANGE (
         PSZ16 fn,                         /* Function to add/change      */
         PSZ16 file,                         /* Name of file to get function*/
         USHORT flag);                    /* Flag indicating search pos  */

USHORT APIENTRY16 RXSUBCOMQUERY(
         PSZ16 env,                          /* Name of the Environment    */
         PSZ16 dll,                          /* DLL Module Name            */
         PUSHORT _Far16 codestor,                      /* Stor for existance code    */
         double _Far16 *userstor);                /* Stor for user word         */

USHORT APIENTRY16 RXFUNCTIONCALL (
        PSZ16 fn,                           /* Name of function to call   */
        USHORT argc,                        /* Number of arguments        */
        PRXSTRING16 argv,                     /* Array of argument strings  */
        PUSHORT _Far16 rc,                       /* RC from function called    */
        PRXSTRING16 stor,                     /* Storage for returned data  */
        PSZ16 data);                         /* Name of active data queue  */

USHORT APIENTRY16 RXEXITREGISTER (
         PSCBLOCK16 scb ) ;                  /* Ptr to SCBLOCK to register */

USHORT APIENTRY16 RXMACROREORDER(
         PSZ16 fn,                         /* Name of funct change order  */
         USHORT pos);                    /* New position for function   */

USHORT APIENTRY16 RXSUBCOMEXECUTE(
         PSZ16 env,                          /* Name of Subcommand Environ */
         PSZ16 dll,                          /* Module name of its' DLL    */
         PRXSTRING16 cmd,                    /* Command string to be passed*/
         PUSHORT _Far16 flag,                      /* Stor for error flag notice */
         PUSHORT _Far16 rc,                      /* Stor for rc from handler   */
         PRXSTRING16 stor );                  /* Stor for returned string   */

USHORT APIENTRY16 RXFUNCTIONQUERY( PSZ16 fn);

USHORT APIENTRY16 RXFUNCTIONQUERY(
        PSZ16 fn);                         /* Name of function to find   */

USHORT APIENTRY16 RXFUNCTIONDEREGISTER (
        PSZ16 fn );                         /* Name of function to remove */

USHORT APIENTRY16 RXEXITDROP (
         PSZ16 fn,                          /* Exit name                  */
         PSZ16 dll ) ;                       /* DLL module name            */

USHORT APIENTRY16 RXFUNCTIONREGISTER(
        PSZ16 fn,                           /* Name of function to add    */
        PSZ16 dll,                           /* Dll file name (if in dll)  */
        PSZ16 entry,                           /* Entry in dll OR mem address*/
        USHORT flag);                       /* RX_DYNALINK || RX_CALLENTRY*/
