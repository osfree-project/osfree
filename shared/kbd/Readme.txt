Open source replacement of OS/2 Keyboard Subsystem
==================================================

Hello. This is part of osFree project. It aims as a open source replacement of OS/2 Keyboard Subsystem (KBD).
KBD relatively easy. It consist of Kbd* functions which routed to Alternate Keyboard Subsystem or Base Keyboard
Subsystem.

This subproject includes replacement of original KBD subsystem and some enchancements. osFree OS/2 Keyboard subsystem
provides much richer set of KBD APIs comparing to IBM Family API, HX DOS Family API or JdeBP's Family API.

As of Family API v.1.20 it is mainly focused on DOS part of Family API to provide, at first place, dual mode
osFree Command Line Tools.

As side effect osFree Family API provides KBDCALLS.DLL replacement for HX DOS Extender as well as BKSCALLS.DLL
replacement (BKS subproject). Moreover, EMXWRAP, CON3216 and OS2CHAR2 DLLs are provided.

Because of its nature, all KBD API are 16-bit in 1.20 version of package. 

Most probably first release of our Kbd subsystem will be as part of osFree Family API 1.20 release.

Files
-----

KBD.LIB Static library.
KBDCALLS.DLL Dynamic library.
KBDCALLS.LIB Import library.
