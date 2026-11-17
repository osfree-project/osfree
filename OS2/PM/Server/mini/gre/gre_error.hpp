WinSetErrorInfo posts an error message, which can be retrieved by an application that calls WinGetLastError.

Syntax
WinSetErrorInfo(idError, fsOptions, arg1...argN)
Parameters
idError (ERRORID) - input
An ERRORID structure
fsOptions (ULONG) - input
Option flags:
SEI_NOBEEP   Do not call DosBeep.
SEI_NOPROMPT Do not prompt the user.
SEI_DOSERROR The next parameter is a base OS/2 error code.
SEI_ARGCOUNT  fsOptions must be set to SEI_ARGCOUNT to use the variable arguments arg1...argN. arg1 must contain the number of arguments that follow, not including arg1 itself. Note that fsOptions cannot equal 000CH; that is, SEI_DOSERROR and argcount are mutually exclusive.
arg1...argN (ULONG) - input
Variable number of optional arguments.
These parameters have no significance except when the presentation driver is reporting a base OS/2 error. The two examples that follow show how WinSetErrorInfo is used to post a presentation driver error and a base OS/2 error:
Example #1. To post a presentation driver error:

   WinSetErrorInfo (ERRORID (SEVERITY_ERROR, PMERR_INV_COORDINATE), NOBEEP);
Example #2. To post a base OS/2 error:

   WinSetErrorInfo (ERRORID (SEVERITY_WARNING, ErrorCode),
   (DOSERROR+NOBEEP+NOPROMPT), DosErrorCode);
There is no return value for this function.

Returns
There is no return value for this function.

Remarks
The syntax of the call to WinSetErrorInfo allows a variable number of parameters. The minimum requirement is the appropriate ERRORID structure and a NULL value for fsOptions.

Note
Presentation drivers do not need to import this function through the module definition file.


#define SEI_BREAKPOINT   0x8000
#define SEI_NOBEEP       0x4000
#define SEI_NOPROMPT     0x2000
#define SEI_DBGRSRVD     0x1000
#define SEI_DEBUGONLY    (SEI_BREAKPOINT | SEI_NOBEEP | SEI_NOPROMPT | SEI_RESERVED)

#define SEI_STACKTRACE   0x0001
#define SEI_REGISTERS    0x0002
#define SEI_ARGCOUNT     0x0004
#define SEI_DOSERROR     0x0008
#define SEI_RESERVED     0x0FE0

ERRORID APIENTRY WinSetErrorInfo(ERRORID,ULONG,...);
