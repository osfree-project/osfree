# Makefile

PROJ = som
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM Kernel
DLLOPT  = initinstance terminstance
srcfiles = $(p)somkern$(e) &
		$(p)rhbsomid$(e) &
		$(p)somalloc$(e) &
		$(p)somobjva$(e) &
		$(p)somapi$(e) &
		$(p)sommutex$(e) &
		$(p)somkpath$(e) &
		$(p)rhbseh2$(e) &
!ifeq %OS LINUX
!ifdef HOSTBLD
		$(p)rhbseh$(e) &
!endif
!endif
		$(p)somshlb$(e)

!if 0
EXPORTS =     somLPrintf=_somLPrintf, &
              somPrintf=_somPrintf, &
              somva_SOMObject_somClassDispatch=_somva_SOMObject_somClassDispatch, &
              somva_SOMObject_somDispatch=_somva_SOMObject_somDispatch, &
              somva_SOMObject_somDispatchA=_somva_SOMObject_somDispatchA, &
              somva_SOMObject_somDispatchD=_somva_SOMObject_somDispatchD, &
              somva_SOMObject_somDispatchL=_somva_SOMObject_somDispatchL, &
              somva_SOMObject_somDispatchV=_somva_SOMObject_somDispatchV
!endif

#INSTALL_ADD = 1

!ifdef HOSTBLD
SRC = $(MYDIR)..$(SEP)..$(SEP)
OS2TK = $(%OS2TK)
!else
SRC = $(MYDIR)..$(SEP)
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45
!endif

SOM_DIR=$(SRC)contrib$(SEP)
SOM_INCL= -i=$(SOM_DIR)include &
         -i=$(SOM_DIR)som$(SEP)include &
		 -i=$(SOM_DIR)som$(SEP)src &
		 -i=$(SOM_DIR)somcdr$(SEP)include &
		 -i=$(SOM_DIR)rhbmtut$(SEP)include &
		 -i=$(SOM_DIR)somcdr$(SEP)src 	&
		 -i=$(OS2TK)$(SEP)som$(SEP)include &
		 -i=$(SOM_DIR)somkpub$(SEP)include &
		 -i=$(SOM_DIR)somtk$(SEP)include

!ifdef HOSTBLD

DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)common$(SEP)dll

!ifeq %OS OS2
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_OS2_ -DSTRICT -DBUILD_RHBSEH2 -DSOMKERN_DLL_TERM
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_WIN32 -DSTRICT -DBUILD_RHBSEH2 -bc -bm -br -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clbrdll option nostdcall, IMPLIB=$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll option nostdcall, IMPLIB=$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_WIN32 -DSTRICT -DBUILD_RHBSEH2 -bc -bm -br -dWIN32_LEAN_AND_MEAN
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS DOS
ADD_OPTS = 
defs = $(%WATCOM)$(SEP)h$(SEP)dos
!endif
!ifeq %OS LINUX
ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_UNIX_ -D_PLATFORM_X11_ -DSTRICT -DBUILD_RHBSEH2 &
    -D_POSIX_SOURCE -DHAVE_UNISTD_H -DHAVE_SIGNAL_H
ADD_LINKOPT = lib clib3r
defs = $(%WATCOM)$(SEP)lh
!endif
OPTIONS = manyautodata

ADD_COPT = -bd -dNDEBUG -i=$(defs) -dHAVE_CONFIG_H &
           $(SOM_INCL) -DBUILD_SOM -i=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR) $(ADD_OPTS)

!else

DEST    = os2tk45$(SEP)som$(SEP)common$(SEP)dll

ADD_OPTS = -i=$(SOM_DIR)som$(SEP)win32 -D_PLATFORM_OS2_ -DSTRICT -DBUILD_RHBSEH2 -DSOMKERN_DLL_TERM
ADD_LINKOPT = lib clib3r 
defs = $(%WATCOM)$(SEP)h$(SEP)os2
OPTIONS = manyautodata, caseexact, heapsize=40960, start=dll_initterm, IMPLIB=$(OS2TK)$(SEP)som$(SEP)lib$(SEP)somtk.lib

ADD_COPT = -bd -dNDEBUG -i=$(defs) -dHAVE_CONFIG_H &
           $(SOM_INCL) -DBUILD_SOM -i=$(BLD)$(RELDIR) $(ADD_OPTS)

!endif

#STACKSIZE = 0x20000
#UNI2H = 1
C = r

!ifdef HOSTBLD
!include $(%ROOT)tools/mk/tools.mk
!else
!include $(%ROOT)tools/mk/appsos2.mk
!endif


.c: $(SRC)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src;$(SOM_DIR)rhbseh2$(SEP)src;$(SOM_DIR)rhbseh$(SEP)src

.h: $(SRC)contrib$(SEP)include

