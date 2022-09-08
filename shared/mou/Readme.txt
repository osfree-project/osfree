Open source replacement of OS/2 Mouse Subsystem
===============================================

Hello. This is part of osFree project. It aims as a open source replacement of OS/2 Mouse Subsystem (MOU).
MOU relatively easy. It consist of Mou* functions which routed to Alternate Mouse Subsystem or Base Mouse
Subsystem.

This subproject includes replacement of original MOU subsystem and some enchancements. osFree OS/2 Mouse subsystem
provides much richer set of MOU APIs comparing to IBM Family API, HX DOS Family API or JdeBP's Family API.

As of Family API v.1.20 it is mainly focused on DOS part of Family API to provide, at first place, dual mode
osFree Command Line Tools.

As side effect osFree Family API provides MOUCALLS.DLL replacement for HX DOS Extender as well as BMSCALLS.DLL
replacement (BMS subproject). Moreover, EMXWRAP, CON3216 and OS2CHAR2 DLLs are provided.

Because of its nature, all MOU API are 16-bit in 1.20 version of package. 

Most probably first release of our Mou subsystem will be as part of osFree Family API 1.20 release.

Files (draft)
-------------

MOU.LIB Static library
MOUCALLS.DLL Dynamic library
MOUCALLS.LIB Import library

Internals
---------

MOUCALLS router is a 16-bit OS/2 API to manage high-level functions of Mouse Subsystem. It routes
Mou* functions to Alternate or Base mouse subsystem. Mouse Router manages its own data for each screen group.

Mouse Router support three Initialization/Deinitialization functions for Shell/Session Manager. On
startup Session Manager calls MouShellInit function to initialize Mouse Router. MouShellInit stores Shell PID
to global variable to prevent calling of MouFree and MouInitReal from non-shell application. It also prepares
global data for each screen group. On shutdown Shell calls MouFree, used to free internal data structures.

On each Mou* call Mouse Router checks is Mouse Router initialized and is mouse presented. If it not initialized, 
then it calls initialization routine similar to MouShellInit. If mouse device driver is present then Mouse Router
check is Alternate Mouse Subsystem function registered and calls or Alternate function or Base function. If Alternate
function executed, depends on return code, Mouse Router executes Base function and returns to application.

Different Alternate Mouse Subsystem can be registered for each Screen Group.

MouSync function used by Mouse Subsystem for syncronization.

References
----------

Putting a mouse in your programs PC Mag 1989-06-16 pp. 309-317
