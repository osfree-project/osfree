# Makefile for SOM Compiler Library

PROJ = somc
DLL = 1
DESC = SOM Compiler Library
DLLOPT  = initinstance terminstance
INSTALL_ADD = 1
srcfiles = $(p)somc$(e)

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

SOMIDL_IDL=$(OS2TK)$(SEP)som$(SEP)include
SOMIDL_HEADERS=$(SOMIDL_IDL)
SOM_DIR=$(SRC)
SOM_INCL= -i=$(SOM_INTDIR) &
	  -i=$(SOMIDL_HEADERS)

!ifdef HOSTBLD

DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)lib

!ifeq %OS OS2
ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT 
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST)
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -D_WIN32 -DSTRICT -bc -bm -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST) option nostdcall, IMPLIB=$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST) option nostdcall, IMPLIB=$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
ADD_OPTS = -D_WIN32 -DSTRICT -bc -bm -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS =  -D_PLATFORM_UNIX_ -D_PLATFORM_X11_ -DSTRICT  &
    -D_POSIX_SOURCE -DHAVE_UNISTD_H -DHAVE_SIGNAL_H
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST)
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

ADD_COPT = -bd -dNDEBUG -i=$(defs) -dHAVE_CONFIG_HPP -d_USE_SOME_ &
           $(SOM_INCL) -i=$(BLD)$(RELDIR) $(ADD_OPTS)

!endif

#EXPORTS = 	somtopenEmitFile, &
#		somtopenEmitFileSL, &
#		somtfclose, &
#		somtfcloseSL

UNI2H = 1
C = r

ADD_SCOPT = -I$(SOM_INTDIR) -I$(SOMIDL_HEADERS) -D__PRIVATE__

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif

install_add: $(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib

$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib: $(OS2TK)$(SEP)som$(SEP)lib$(SEP)somc.dll
	$(verbose)$(SAY) LIB      $^. $(LOG)
	$(verbose)$(LIB) -b $^@ +$< $(BLACKHOLE)


.c: $(SRC)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src;$(SOM_DIR)rhbseh2$(SEP)src;$(SOM_DIR)rhbseh$(SEP)src
#.idl: $(SRC)..$(SEP)idl
.xih: $(SOM_INTDIR)
#.xh: $(SRC)

$(OBJS): $(SOMIDL_HEADERS)$(SEP)emitlib.h $(SOMIDL_HEADERS)$(SEP)sm.h $(SOMIDL_HEADERS)$(SEP)sm.xh

$(SOMIDL_HEADERS)$(SEP)sm.h: $(SOM_DIR)$(SEP)sm.h
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)sm.xh: $(SOM_DIR)$(SEP)sm.xh
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)

$(SOMIDL_HEADERS)$(SEP)emitlib.h: $(SOM_DIR)$(SEP)emitlib.h
	$(verbose)$(SAY) CP       $^. $(LOG)
	$(verbose)$(CP) $< $^@ $(BLACKHOLE)
