PROJ = emitdef
TRGT = $(PROJ).dll
srcfiles = $(p)emitdef$(e)

DEST = os2tk45$(SEP)som$(SEP)common$(SEP)dll

DLL=1

EXPORTS = emit, emitSL

IMPORTS = somtopenEmitFile SOMC, somtgetatt SOMC


!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif

#ADD_LINKOPT += lib c:\som21nt\lib\somtk.lib
ADD_LINKOPT += lib c:\som21nt\lib\som_ow.lib option NOD

ADD_COPT = -br -bd -6s $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include -i=$(PORT_BASE)somc$(SEP)include

TRAGETS: $(FILESDIR)os2$(SEP)dll$(SEP)emitdef.dll

.c: $(%ROOT)SOM$(SEP)emitdef


