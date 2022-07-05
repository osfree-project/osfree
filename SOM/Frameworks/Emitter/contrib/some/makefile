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
!else
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

SOM_DIR=$(SRC)
SOM_INCL= -i=$(SOM_DIR) &
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
ADD_LINKOPT = lib clbrdll
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll
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

ADD_SCOPT = -I$(SOM_DIR) -I$(OS2TK)$(SEP)som$(SEP)include -D__PRIVATE__

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif



.c: $(SRC)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src;$(SOM_DIR)rhbseh2$(SEP)src;$(SOM_DIR)rhbseh$(SEP)src
.idl: $(SRC)..$(SEP)idl
.xih: $(SRC)
#.xh: $(SRC)

PRIVATE_HEADERS=$(SRC)$(SEP)scentry.xh &
			$(SRC)$(SEP)scentry.xih &
			$(SRC)$(SEP)scmeta.xh &
			$(SRC)$(SEP)scmeta.xih &
			$(SRC)$(SEP)sctdef.xh &
			$(SRC)$(SEP)sctdef.xih &
			$(SRC)$(SEP)scstruct.xh &
			$(SRC)$(SEP)scstruct.xih &
			$(SRC)$(SEP)scenum.xh &
			$(SRC)$(SEP)scenum.xih &
			$(SRC)$(SEP)scunion.xh &
			$(SRC)$(SEP)scunion.xih &
			$(SRC)$(SEP)scenumnm.xh &
			$(SRC)$(SEP)scenumnm.xih &
			$(SRC)$(SEP)scclass.xh &
			$(SRC)$(SEP)scclass.xih &
			$(SRC)$(SEP)scconst.xh &
			$(SRC)$(SEP)scconst.xih &
			$(SRC)$(SEP)scbase.xh &
			$(SRC)$(SEP)scbase.xih &
			$(SRC)$(SEP)scattrib.xh &
			$(SRC)$(SEP)scattrib.xih &
			$(SRC)$(SEP)sccommon.xh &
			$(SRC)$(SEP)sccommon.xih &
			$(SRC)$(SEP)scpass.xh &
			$(SRC)$(SEP)scpass.xih &
			$(SRC)$(SEP)scdata.xh &
			$(SRC)$(SEP)scdata.xih &
			$(SRC)$(SEP)scmodule.xh &
			$(SRC)$(SEP)scmodule.xih &
			$(SRC)$(SEP)scemit.xh &
			$(SRC)$(SEP)scemit.xih &
			$(SRC)$(SEP)somstrt.xh &
			$(SRC)$(SEP)somstrt.xih &
			$(SRC)$(SEP)sctmplt.xh &
			$(SRC)$(SEP)sctmplt.xih &
			$(SRC)$(SEP)scmethod.xh &
			$(SRC)$(SEP)scmethod.xih &
			$(SRC)$(SEP)scstring.xh &
			$(SRC)$(SEP)scstring.xih &
			$(SRC)$(SEP)scseqnce.xh &
			$(SRC)$(SEP)scseqnce.xih &
			$(SRC)$(SEP)scparm.xh &
			$(SRC)$(SEP)scparm.xih &
			$(SRC)$(SEP)scusrtyp.xh &
			$(SRC)$(SEP)scusrtyp.xih

$(OBJS): $(PRIVATE_HEADERS)
#  echo $(ADD_CPPOPT)