

x18servicesV0.1.zip
-------------------

Version 0.0, first alpha, 10th June 2002 by Ximon Eighteen.
Version 0.1, added GetServiceNameFromDisplayName(), 5th June 2003 by Ximon Eighteen.


What The Hell Is This?
----------------------

If you've ever had to make an installation program that had to start, stop,
remove or otherwise manipulate a Windows NT Service then you'll find this
useful. It's a collection of functions that can be used from an NSIS script
(Nullsoft Installation System, http://www.nullsoft.com/free/nsis) to manipulate
Windows NT (and hopefully 2000 and XP) Services.

If you don't know what I'm talking about then you don't need this, simple.

Most of the code is based on information found in the Microsoft Knowledge Base
and the MSDN Platform SDK documentation.

I haven't released anything in quite a long time so if I've forgotten something
or have generally fouled up somewhere then for gods sake tell me.

Feel free to send comments/suggestions/bug fixes/hate mail/biscuits to
ximon_eighteen@3b2.com.

As long as you don't violate any nullsoft agreement that might be binding on
an extension DLL for their NSIS product as far as I'm concerned you can do
absolutely anything with this code, claim as your own if you like although
that would be a bit low.

I'll answer any questions that I can, but I'm not responsible if this fouls
up your system, use this at your own risk. If something does go wrong I'll do
what I can to help, but I think this is highly unlikely ;-)

---

Ximon Eighteen aka Sunjammer


Distribution :
--------------

  - exdll.c
  - isadministrator.c
  - logonasservice.c
  - nsisapi.h
  - queryservice.c
  - readme.txt
  - sendservicecommand.c
  - winerror.h
  - winerror.c
  - Visual C++ 6.0 project file (.dsp) [ release & debug configurations ]
  - Services.nsi  [ example ]
  - FindService.nsi [ example ]

  The code is based on the exdll project that shipped with 0.98 of the NSIS
  distro. Building the project should result in a .dll file that you can then
  use with the CallInstDLL NSIS scripting command.


Known Bugs/Issues :
-------------------

  - I don't think the SendServiceCommand should really support a delete
    option, perhaps it should be a separate function?
  - No other known issues.


Function Level Docs : [ extracted from the source ]
---------------------

GetServiceNameFromDisplayName:
  DESC: Find the internal service name of all services whose display name (as
  DESC: seen in the Services Control Panel) contains the given name or name
  DESC: fragment (case insensitive match).
  ARGS: Push the display name or display name fragment onto the stack
  RETN: One or more values on the stack.
  RETN:   The first will be the count of matches found.
  RETN:   The second will be the reason for finding no matches, OR the first match.
  RETN:   The third and so on will be the remaining matches upto the count.
  USGE: See the FindServices.nsi example.

IsProcessUserAdministrator:
  DESC: Find out if the user (the owner of the current process) is a member of
  DESC: the administrators group on the local computer (not on the domain!).
  ARGS: Push the service name onto the stack.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::IsProcessUserAdministrator
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

HasLogonAsAService:
  DESC: Find out if the current user (the owner of the process) has the LSA
  DESC: (Local Security Authority) policy right 'SeServiceLogonRight'.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::HasLogonAsAService '3B2\Ximon_Eighteen'
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

GrantLogonAsAService:
  DESC: Grant the current user (the owner of the process) the LSA
  DESC: (Local Security Authority) policy right 'SeServiceLogonRight'.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Ok' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::GrantLogonAsAService '3B2\Ximon_Eighteen'
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

RemoveLogonAsAService:
  DESC: Deny the current user (the owner of the process) the LSA
  DESC: (Local Security Authority) policy right 'SeServiceLogonRight'.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Ok' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::RemoveLogonAsAService '3B2\Ximon_Eighteen'
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

IsServiceInstalled:
  DESC: Find out if the named service is installed.
  ARGS: Push the service name onto the stack.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::IsServiceInstalled 'MyServiceName'
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

IsServiceRunning
  DESC: Find out if the named service is installed and running.
  ARGS: Push the service name onto the stack.
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Yes' or 'No' if there were no problems, otherwise
  RETN: the reason for the failure will be in the result string.
  USGE: Use it like this :-
  USGE:
  USGE:   services::IsServiceRunning 'MyServiceName'
  USGE:   Pop $0
  USGE:   ; $0 now contains either 'Yes', 'No' or an error description

SendServiceCommand:
  DESC: Send a command (Start, Stop, Pause, Continue or Delete) to the Service
  DESC: Control Manager (SCM) for a given service.
  ARGS: Push service command string onto the stack
  ARGS: Push service name string onto the stack
  RETN: Pop the result string from the stack.
  RETN: The result will be 'Ok' if there were no problems, or the reason for
  RETN: failure otherwise.
  USGE: Use it like this :-
  USGE:
  USGE:   services::SendServiceCommand 'start' 'MyServiceName'
  USGE:   Pop $0
  USGE:   StrCmp $0 'Ok' success 0
  USGE:     MessageBox MB_OK|MB_ICONSTOP 'Failed to send service command: Reason: $0' 0 0
  USGE:     Abort
  USGE:   success:
