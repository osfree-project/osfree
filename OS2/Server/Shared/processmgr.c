/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
*/
#define INCL_ERRORS
#define INCL_DOS
#define INCL_BSEDOS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#include <token.h>
#include <cfgparser.h>
#include <processmgr.h>
#include <modlx.h>
#include <execlx.h>
#include <modmgr.h>
#include <ixfmgr.h>
#include <io.h>


/*ULONG     pib_ulpid;      Process identifier.
  ULONG     pib_ulppid;     Parent process identifier.
  ULONG     pib_hmte;       Module handle of executable program.
  PCHAR     pib_pchcmd;     Command line pointer.
  PCHAR     pib_pchenv;     Environment pointer.
  ULONG     pib_flstatus;   Process' status bits.
  ULONG     pib_ultype;     Process' type code. */

/* Creates a process for an LX-module. */
struct t_os2process * PrcCreate(IXFModule ixfModule)
{

    struct t_os2process * c = (struct t_os2process *) malloc(sizeof(struct t_os2process));
    c->lx_pib   = (PPIB) malloc(sizeof(PIB));
    c->main_tid = (PTIB) malloc(sizeof(TIB));

    if (c != NULL) {
        c->pid = 1;
        c->code_mmap = 0;
        c->stack_mmap = 0;
        c->lx_mod = (struct LX_module *)(ixfModule.FormatStruct);

        c->lx_pib->pib_ulpid = 1;
        c->lx_pib->pib_ulppid = 0;
        c->lx_pib->pib_hmte = (ULONG) (struct LX_module *)(ixfModule.FormatStruct);
        c->lx_pib->pib_pchcmd = "";
        c->lx_pib->pib_pchenv = "";

        init_memmgr(&c->root_mem_area); /* Initialize the memory registry. */
        /* Registrate base invalid area. */
        alloc_mem_area(&c->root_mem_area, (void*) 1, 0xfffd);
        /* Make sure the the lower 64 kb address space is marked as used. */
    }
    return(c);
}

void PrcDestroy(struct t_os2process * proc) {
        /* Avmappar filen. */
        struct o32_obj * kod_obj = (struct o32_obj *) get_code(proc->lx_mod);
        int ret_munmap = munmap((void*) kod_obj->o32_base,
                        kod_obj->o32_size);


        if(ret_munmap != 0)
                io_printf("Error at unmapping of fh: %p\n", proc->lx_mod->fh);
        else
                io_printf("\nUnmapping fh: %p\n\n", proc->lx_mod->fh);

        free(proc->lx_pib);
        free(proc->main_tid);
        free(proc);
}

unsigned long find_path(char * name, char * full_path_name);

/*!

Lets a program run another program as a child process.

#define INCL_DOSPROCESS
#include <os2.h>

PCHAR           pObjname;     Address of the buffer in which the name of the object that
                                   contributed to the failure of DosExecPgm is returned.
LONG            cbObjname;    Length, in bytes, of the buffer described by pObjname.
ULONG           execFlag;     Flag indicating how the program runs in relation to the
                                 requester, and whether execution is under conditions for
                                 debugging.
PSZ             pArg;         Address of the ASCIIZ argument strings passed to the program.
PSZ             pEnv;         Address of the ASCIIZ environment strings passed to the program.
PRESULTCODES    pRes;         Pointer to the RESULTCODES structure where the process ID, or
                                 the termination code and the result code indicating the
                                 reason for ending the child process is returned.
PSZ             pName;        Address of the name of the file that contains the program to
                                 be executed.
APIRET          ulrc;         Return Code.

ulrc = DosExecPgm(pObjname, cbObjname, execFlag,
         pArg, pEnv, pRes, pName);


pObjname (PCHAR) - output
Address of the buffer in which the name of the object that contributed to the failure
of DosExecPgm is returned.

cbObjname (LONG) - input
Length, in bytes, of the buffer described by pObjname.

execFlag (ULONG) - input
Flag indicating how the program runs in relation to the requester, and whether execution
is under conditions for debugging.

The values of this field are shown in the following list:

0 EXEC_SYNC
Execution is synchronous to the parent process. The termination code and result code are
stored in the RESULTCODES structure pointed to by pRes.

1 EXEC_ASYNC
Execution is asynchronous to the parent process. When the child process ends, its
result code is discarded. The process ID is stored in the codeTerminate field of the
RESULTCODES structure pointed to by pRes.

2 EXEC_ASYNCRESULT
Execution is asynchronous to the parent process. When the child process ends, its
result code is saved for examination by a DosWaitChild request. The process ID is
stored in the codeTerminate field of the RESULTCODES structure pointed to by pRes.

3 EXEC_TRACE
Execution is the same as if EXEC_ASYNCRESULT were specified for execFlag. Debugging
conditions are present for the child process.

4 EXEC_BACKGROUND
Execution is asynchronous to and detached from the parent-process session. When the
detached process starts, it is not affected by the ending of the parent process. The
detached process is treated as an orphan of the parent process.

A program executed with this option runs in the background, and should not require
any input from the keyboard or output to the screen other than VioPopups. It should
not issue any console I/O calls (VIO, KBD, or MOU functions).

5 EXEC_LOAD
The program is loaded into storage and made ready to execute, but is not executed
until the session manager dispatches the threads belonging to the process.

6 EXEC_ASYNCRESULTDB
Execution is the same as if EXEC_ASYNCRESULT were specified for execFlag, with the
addition of debugging conditions being present for the child process and any of its
descendants. In this way, it is possible to debug even detached and synchronous processes.

Some memory is consumed for uncollected result codes. Issue DosWaitChild to release
this memory. If result codes are not collected, then EXEC_SYNC or EXEC_ASYNC should
be used for execFlag.

pArg (PSZ) - input
Address of the ASCIIZ argument strings passed to the program.

These strings represent command parameters, which are copied to the environment segment
of the new process.

The convention used by CMD.EXE is that the first of these strings is the program name
(as entered from the command prompt or found in a batch file), and the second string
consists of the parameters for the program. The second ASCIIZ string is followed by
an additional byte of zeros. A value of zero for the address of pArg means that no
arguments are to be passed to the program.

pEnv (PSZ) - input
Address of the ASCIIZ environment strings passed to the program.

These strings represent environment variables and their current values. An environment
string has the following form:


variable=value


The last ASCIIZ environment string must be followed by an additional byte of zeros.

A value of 0 for the address of pEnv results in the new process' inheriting the
environment of its parent process.

When the new process is given control, it receives:

 A pointer to its environment segment
 The fully qualified file specification of the executable file
 A copy of the argument strings.

A coded example of this follows:


eo:   ASCIIZ string 1  ; environment string 1
      ASCIIZ string 2  ; environment string 2
.
.
.
      ASCIIZ string n  ; environment string n
      Byte of 0
.
.
.
po:   ASCIIZ           ; string of file name
                       ; of program to run.
.
.
.
ao:   ASCIIZ           ; argument string 1
                       ; (name of program being started
                       ; for the case of CMD.EXE)
      ASCIIZ           ; argument string 2
                       ; (program parameters following
                       ; program name for the case of
                       ; CMD.EXE)
      Byte of 0


The beginning of the environment segment is "eo", and "ao" is the offset of the first
argument string in that segment. The offset to the command line, "ao", is passed to
the program on the stack at SS:[ESP+16].

The environment strings typically have the form: parameter = value

A value of zero for pEnv causes the newly created process to inherit the parent's
environment unchanged.

pRes (PRESULTCODES) - output
Pointer to the RESULTCODES structure where the process ID, or the termination code
and the result code indicating the reason for ending the child process is returned.

This structure also is used by a DosWaitChild request, which waits for an asynchronous
child process to end.

pName (PSZ) - input
Address of the name of the file that contains the program to be executed.

When the environment is passed to the target program, this name is copied into "po"
in the environment description shown above.

If the string appears to be a fully qualified file specification (that is, it contains
a ":" or a "\" in the second position), then the file name must include the extension,
and the program is loaded from the indicated drive:directory.

If the string is not a fully qualified path, the current directory is searched. If the
file name is not found in the current directory, each drive:directory specification in
the PATH defined in the current-process environment is searched for this file. Note that
any extension (.XXX) is acceptable for the executable file being loaded.

ulrc (APIRET) - returns
Return Code.

DosExecPgm returns one of the following values:

0 NO_ERROR
1 ERROR_INVALID_FUNCTION
2 ERROR_FILE_NOT_FOUND
3 ERROR_PATH_NOT_FOUND
4 ERROR_TOO_MANY_OPEN_FILES
5 ERROR_ACCESS_DENIED
8 ERROR_NOT_ENOUGH_MEMORY
10 ERROR_BAD_ENVIRONMENT
11 ERROR_BAD_FORMAT
13 ERROR_INVALID_DATA
26 ERROR_NOT_DOS_DISK
31 ERROR_GEN_FAILURE
32 ERROR_SHARING_VIOLATION
33 ERROR_LOCK_VIOLATION
36 ERROR_SHARING_BUFFER_EXCEEDED
89 ERROR_NO_PROC_SLOTS
95 ERROR_INTERRUPT
108 ERROR_DRIVE_LOCKED
127 ERROR_PROC_NOT_FOUND
182 ERROR_INVALID_ORDINAL
190 ERROR_INVALID_MODULETYPE
191 ERROR_INVALID_EXE_SIGNATURE
192 ERROR_EXE_MARKED_INVALID
195 ERROR_INVALID_MINALLOCSIZE
196 ERROR_DYNLINK_FROM_INVALID_RING

For a full list of error codes, see Errors.


DosExecPgm allows a program to request that another program execute as a child process.

The target program is located and loaded into storage (if necessary), a process is
created for it and placed into execution. The execution of a child process can be
synchronous or asynchronous to the execution of its parent process. If synchronous
execution is indicated, the requesting thread waits for completion of the child process.
Other threads in the requesting process may continue to run.

If asynchronous execution is indicated, DosExecPgm places the process ID of the started
child process into the first doubleword of the pRes structure. If EXEC_ASYNCRESULT is
specified for execFlag, the parent process can issue DosWaitChild (after DosExecPgm)
to examine the result code returned when the child process ends. If the value of execFlag,
is EXEC_ASYNC, the result code of the asynchronous child process is not returned to
the parent process.

If synchronous execution is indicated, DosExecPgm places the termination code and
result code into the pRes structure.

The new process is created with an address space separate and distinct from its parent;
that is, a new linear address space is built for the process.

The new process inherits all file handles and pipes of its parent, although not
necessarily with the same access rights:

 Files are inherited except for those opened with no inheritance indicated.
 Pipes are inherited.

A child process inherits file handles obtained by its parent process with DosOpen
calls that indicated inheritance. The child process also inherits handles to pipes
created by the parent process with DosCreatePipe. This means that the parent process
has control over the meanings of standard input, output, and error. For example, the
parent could write a series of records to a file, open the file as standard input,
open a listing file as standard output, and then execute a sort program that takes
its input from standard input and writes to standard output.

Because a child process can inherit handles, and a parent process controls the meanings
of handles for standard I/O, the parent can duplicate inherited handles as handles for
standard I/O. This permits the parent process and the child process to coordinate I/O
to a pipe or file. For example, a parent process can create two pipes with DosCreatePipe
requests. It can issue DosDupHandle to redefine the read handle of one pipe as
standard input (0x0000), and the write handle of the other pipe as standard output (0x0001).
The child process uses the standard I/O handles, and the parent process uses the remaining
read and write pipe handles. Thus, the child process reads what the parent process writes
to one pipe, and the parent process reads what the child process writes to the other pipe.

When an inherited file handle is duplicated, the position of the file pointer is always
the same for both handles, regardless of which handle repositions the file pointer.

An asynchronous process that was started because the value of execFlag was EXEC_TRACE or
EXEC_ASYNCRESULTDB is provided a trace flag facility. This facility and the trace buffers
provided by DosDebug enable a debugger to perform breakpoint debugging. DosStartSession
provides additional debugging capabilities that allow a debugger to trace all processes
associated with an application running in a child session, regardless of whether the
process is started with DosExecPgm or DosStartSession.

A detached process is treated as an orphan of the parent process and runs in the background.
Thus, it cannot make any VIO, KBD, or MOU calls, except from within a video pop-up
requested by VioPopUp. To test whether a program is running detached, use the following
method. Issue a video call, (for example, VioGetAnsi). If the call is not issued within
a video pop-up and the process is detached, the video call returns error code
ERROR_VIO_DETACHED.

You may use DosExecPgm to start a process that is of the same type as the starting process.
Process types include Presentation Manager, text-windowed, and full-screen. You may not
use DosExecPgm to start a process that is of a different type than the starting process.

You must use DosStartSession to start a new process from a process that is of a different
type. For example, use DosStopSession to start a Presentation Manager process from a
non-Presentation Manager process.

The following are the register conventions for 32-bit programs:

Register Definition
EIP Starting program entry address
ESP Top of stack address
CS Code selector for the base of the linear address space
DS, ES, SS Data selector for the base of the linear address space
FS Data selector for the thread information block
GS 0
EAX, EBX 0
ECX, EDX 0
ESI, EDI 0
EBP 0
[ESP+0] Return address to the routine that calls DosExit
[ESP+4] Module handle for the program module
[ESP+8] 0
[ESP+12] Address of the environment data object
[ESP+16] Offset to the command line in the environment data object.


*/

APIRET APIENTRY PrcExecuteModule(char * pObjname,
                                 long cbObjname,
                                 unsigned long execFlag,
                                 char * pArg,
                                 char * pEnv,
                                 struct _RESULTCODES *pRes, /*PRESULTCODES */
                                 char * pName)
{
  int rc=NO_ERROR;
  void * addr;
  unsigned long size;
  IXFModule ixfModule;
  struct t_os2process * tiny_process;
  #define buf_size 4096
  char buf[buf_size+1];
  char *p_buf = (char *) &buf;

  // Try to load file (consider fully-qualified name specified)
  rc=io_load_file(pName, &addr, &size);
  if (rc) // if error then
  {
    // Searches for module name and returns the full path in the buffer p_buf.
    rc=find_path(pName, p_buf);
    if (!rc) rc=io_load_file(p_buf, &addr, &size);
  }
  if (rc) return rc;

  rc=IXFIdentifyModule(addr, size, &ixfModule);
  if (rc)
  {
    return rc;
  }

  // Load module
  rc=IXFLoadModule(addr, size, &ixfModule);
  if (rc)
  {
    return rc;
  }

  /* Creates an simple process (keeps info about it, does not start to execute). */
  tiny_process = PrcCreate(ixfModule);

  /* Register the exe with the module table. With the complete path. */
  /* @todo Is we really need to register executable??? Don't see any reason */
  // @todo extract filename only because can be fullname with path
  register_module(pName, &ixfModule);

  // Fixup module
  rc=IXFFixupModule(&ixfModule);
  if (rc)
  {
    return rc;
  }

  /* Print info about used memory loaded modules. */
  //print_used_mem(&tiny_process->root_mem_area);
  if(rc == NO_ERROR) {
    /* Starts to execute the process. */
    io_printf("Executing exe...\n");
    #if defined(L4API_l4v2)
    l4_exec_lx((struct LX_module *)(ixfModule.FormatStruct), tiny_process);
    #endif
    exec_lx((struct LX_module *)(ixfModule.FormatStruct), tiny_process);
    io_printf("Done executing exe.\n");
  }

  PrcDestroy(tiny_process); /* Removes the process.
             Maybe use garbage collection here? Based on reference counter?
                And when the counter reeches zero, release process. */

  return rc; /*NO_ERROR;*/
}

/*! @brief This is function searches files in PATH environment variables which
           pointed by CONFIG.SYS SET PATH statament. It is uses only OS/2 style
           paths.

    @param name             Filename to find
    @param full_path_name   Fully qualified filename

    @return
      NO_ERROR                  Server finished successfully
      ERROR_FILE_NOT_FOUND      Searched file not found

      See also other error codes

*/

unsigned long find_path(char * name, char * full_path_name)
{
  FILE *f;
  char *p;
  #define buf_size 4096
  char buf[buf_size+1];
  char *path = (char *) &buf;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  char *sep="/";
  char *psep=";";

  cfg_getenv("PATH", &path);

  p = path - 1;

  StrTokSave(&st);
  if((p = StrTokenize((char*)path, psep)) != 0) do if(*p)
  {
    p_buf = full_path_name;
    p_buf[0] = 0;
    strcat(p_buf, p);
    strcat(p_buf, sep);
    strcat(p_buf, name);
    f = fopen(p_buf, "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    if(f)
    {
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
}
