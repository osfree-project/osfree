# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

verbose=1
PROJ = some
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Emitter Framework
DLLOPT  = initinstance terminstance
srcfiles = $(p)emitlib$(e) &
		$(p)scentry$(e) &
		$(p)scmeta$(e) &
		$(p)sctdef$(e) &
		$(p)scstruct$(e) &
		$(p)scenum$(e) &
		$(p)scunion$(e) &
		$(p)scenumnm$(e) &
		$(p)scclass$(e) &
		$(p)scconst$(e) &
		$(p)scbase$(e) &
		$(p)scattrib$(e) &
		$(p)sccommon$(e) &
		$(p)scpass$(e) &
		$(p)scdata$(e) &
		$(p)scmodule$(e) &
		$(p)scemit$(e) &
		$(p)somstrt$(e) &
		$(p)sctmplt$(e) &
		$(p)scmethod$(e) &
		$(p)scstring$(e) &
		$(p)scseqnce$(e) &
		$(p)scparm$(e) &
		$(p)scusrtyp$(e)


PLATFORM = host$(SEP)$(%HOST)$(SEP)
HOSTBLD=1
!ifdef HOSTBLD
SRC = $(MYDIR)
OS2TK = $(%OS2TK)
SOM_INTDIR=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR)
!else
SRC = $(MYDIR)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
SOM_INTDIR=$(BLD)$(RELDIR)
!endif

SOM_DIR=$(SRC)
SOM_INCL= -i=$(SOM_INTDIR) &
          -i=$(SOM_DIR)$(SEP)..$(SEP)somipc &
		  -i=$(OS2TK)$(SEP)som$(SEP)include

!ifdef HOSTBLD

DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)common$(SEP)dll

!ifeq %OS OS2
ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT 
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -D_WIN32 -DSTRICT -bc -bm -br -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clbrdll, somtk
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll, somtk
ADD_OPTS = -D_WIN32 -DSTRICT -bc -bm -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS =  -D_PLATFORM_UNIX_ -D_PLATFORM_X11_ -DSTRICT  &
    -D_POSIX_SOURCE -DHAVE_UNISTD_H -DHAVE_SIGNAL_H
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)lh
!endif
OPTIONS = manyautodata

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_HPP -d_USE_SOME_ &
           $(SOM_INCL) -i=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR) $(ADD_OPTS)

!else

DEST    = os2tk45$(SEP)som$(SEP)common$(SEP)dll

ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT 
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2
OPTIONS = manyautodata, caseexact, heapsize=40960, start=dll_initterm

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_HPP -d_USE_SOME_ &
           $(SOM_INCL) -i=$(BLD)$(RELDIR) $(ADD_OPTS)

!endif

UNI2H = 1
C = r

ADD_SCOPT = -I$(SOM_INTDIR) -I$(OS2TK)$(SEP)som$(SEP)include -p

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif

PRIVATE_HEADERS=$(SOM_INTDIR)$(SEP)scentry.xh &
			$(SOM_INTDIR)$(SEP)scentry.xih &
			$(SOM_INTDIR)$(SEP)scmeta.xh &
			$(SOM_INTDIR)$(SEP)scmeta.xih &
			$(SOM_INTDIR)$(SEP)sctdef.xh &
			$(SOM_INTDIR)$(SEP)sctdef.xih &
			$(SOM_INTDIR)$(SEP)scstruct.xh &
			$(SOM_INTDIR)$(SEP)scstruct.xih &
			$(SOM_INTDIR)$(SEP)scenum.xh &
			$(SOM_INTDIR)$(SEP)scenum.xih &
			$(SOM_INTDIR)$(SEP)scunion.xh &
			$(SOM_INTDIR)$(SEP)scunion.xih &
			$(SOM_INTDIR)$(SEP)scenumnm.xh &
			$(SOM_INTDIR)$(SEP)scenumnm.xih &
			$(SOM_INTDIR)$(SEP)scclass.xh &
			$(SOM_INTDIR)$(SEP)scclass.xih &
			$(SOM_INTDIR)$(SEP)scconst.xh &
			$(SOM_INTDIR)$(SEP)scconst.xih &
			$(SOM_INTDIR)$(SEP)scbase.xh &
			$(SOM_INTDIR)$(SEP)scbase.xih &
			$(SOM_INTDIR)$(SEP)scattrib.xh &
			$(SOM_INTDIR)$(SEP)scattrib.xih &
			$(SOM_INTDIR)$(SEP)sccommon.xh &
			$(SOM_INTDIR)$(SEP)sccommon.xih &
			$(SOM_INTDIR)$(SEP)scpass.xh &
			$(SOM_INTDIR)$(SEP)scpass.xih &
			$(SOM_INTDIR)$(SEP)scdata.xh &
			$(SOM_INTDIR)$(SEP)scdata.xih &
			$(SOM_INTDIR)$(SEP)scmodule.xh &
			$(SOM_INTDIR)$(SEP)scmodule.xih &
			$(SOM_INTDIR)$(SEP)scemit.xh &
			$(SOM_INTDIR)$(SEP)scemit.xih &
			$(SOM_INTDIR)$(SEP)somstrt.xh &
			$(SOM_INTDIR)$(SEP)somstrt.xih &
			$(SOM_INTDIR)$(SEP)sctmplt.xh &
			$(SOM_INTDIR)$(SEP)sctmplt.xih &
			$(SOM_INTDIR)$(SEP)scmethod.xh &
			$(SOM_INTDIR)$(SEP)scmethod.xih &
			$(SOM_INTDIR)$(SEP)scstring.xh &
			$(SOM_INTDIR)$(SEP)scstring.xih &
			$(SOM_INTDIR)$(SEP)scseqnce.xh &
			$(SOM_INTDIR)$(SEP)scseqnce.xih &
			$(SOM_INTDIR)$(SEP)scparm.xh &
			$(SOM_INTDIR)$(SEP)scparm.xih &
			$(SOM_INTDIR)$(SEP)scusrtyp.xh &
			$(SOM_INTDIR)$(SEP)scusrtyp.xih

$(OBJS): $(PRIVATE_HEADERS)
