
src\shared has code which is shared by the various
XWorkplace classes. The code in this directory has
been separated from the actual class code because
it might be interesting for other developers too.

This has the XWorkplace "Kernel" (kernel.c) which
handles Desktop startup and thread synchronization,
common.c which deals with global settings, NLS, and
such, notebook.c which has useful code for managing
notebook settings pages, and more.

All this code goes into XFLDR.DLL.

Also, this holds the DEF files to link XFLDR.DLL
together as well as the resources which go into
XFLDR.DLL (and are thus independent of NLS).

Ulrich M”ller (2000-10-29)

