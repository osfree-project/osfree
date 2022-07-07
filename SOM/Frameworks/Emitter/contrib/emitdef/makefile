
PROJ = emitdef
TRGT = $(PROJ).dll
DLL = 1
DESC = SOM DEF Emitter
DLLOPT  = initinstance terminstance
srcfiles = $(p)emitdef$(e) &
		$(p)defemit$(e)


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
		  -i=$(OS2TK)$(SEP)som$(SEP)include

!ifdef HOSTBLD

DEST    = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)som$(SEP)lib

!ifeq %OS OS2
ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT 
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST)
defs = $(%WATCOM)$(SEP)h$(SEP)os2
!endif
!ifeq %OS WIN32
ADD_OPTS = -D_WIN32 -DSTRICT -bc -bm -br -dWIN32_LEAN_AND_MEAN
ADD_LINKOPT = lib clbrdll, somtk libpath $(DEST)
defs = $(%WATCOM)$(SEP)h$(SEP)nt
!endif
!ifeq %OS WIN64
ADD_LINKOPT = lib clbrdll, somtk libpath $(DEST)
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
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST)
defs = $(%WATCOM)$(SEP)lh
!endif
OPTIONS = manyautodata

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_HPP -d_USE_SOME_ &
           $(SOM_INCL) -i=$(BLD)host$(SEP)$(%HOST)$(SEP)$(RELDIR) $(ADD_OPTS)

!else

DEST    = os2tk45$(SEP)som$(SEP)lib

ADD_OPTS = -D_PLATFORM_OS2_ -DSTRICT 
ADD_LINKOPT = lib clib3r, somtk libpath $(DEST)
defs = $(%WATCOM)$(SEP)h$(SEP)os2
OPTIONS = manyautodata, caseexact, heapsize=40960, start=dll_initterm

ADD_COPT = -dNDEBUG -i=$(defs) -dHAVE_CONFIG_HPP -d_USE_SOME_ &
           $(SOM_INCL) -i=$(BLD)$(RELDIR) $(ADD_OPTS)

!endif

UNI2H = 1
C = r

ADD_SCOPT = -I$(SOM_INTDIR) -I$(OS2TK)$(SEP)som$(SEP)include -D__PRIVATE__

!ifdef HOSTBLD
!include $(%ROOT)/mk/tools.mk
!else
!include $(%ROOT)/mk/appsos2.mk
!endif



.c: $(SRC)contrib$(SEP)som$(SEP)src;$(SOM_DIR)somid$(SEP)src
.idl: $(SRC)..$(SEP)idl
.xih: $(SOM_INTDIR)
#.xh: $(SRC)

PRIVATE_HEADERS=$(SOM_INTDIR)$(SEP)defemit.xh &
			$(SOM_INTDIR)$(SEP)defemit.xih

$(OBJS): $(PRIVATE_HEADERS)
