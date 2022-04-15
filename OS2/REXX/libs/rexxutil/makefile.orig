#########################################################################
#
#              Open Watcom 1.7a Main Makefile
#
# Open Watcom 1.7a and newer makefile for rexxutil.dll
#
# Author        : Michael Greene, January 2008
#
#**************************************************************************
#
# Options:
#
#       wmake [debug]
#
#       clean        - remove all build files and directories
#
#**************************************************************************


# do a dummy check - is WATCOM env variable defined?
!ifndef %WATCOM
!error **** Variable WATCOM not define! ****
!endif

# include user selectable option
NAMEDLL    =rexxutil

DLLDEFS      =
MACHINE      = -6s -fp6 -bd

OPTIMIZATION =  -oteanx

CC  = wcc386
CL  = wcl386
LD  = wlink
LB  = wlib

########################################
#
#  ***** Release/Debug options *****
#
!ifeq %DEBUG 1
C_COMMON    =-d3 -of
OPT         =-od
LD_OPTS_BLD =d all op map,symf
!else
C_COMMON    =-d0
OPT         =$(OPTIMIZATION)
LD_OPTS_BLD =op el,map
!endif
#  ***** END Release/Debug options *****

LD_OPTS =$(LD_OPTS_BLD)

INCLUDE   =$(%BASEDIR)\src;$(%watcom)\h;$(%watcom)\h\os2
CFLAGS    =-i=$(INCLUDE) $(DLLDEFS) -wx -bt=os2 -mf $(C_COMMON) $(OPT) -bm
LDFLAGS   =$(LD_OPTS)$(LD_OPTS_DLL)

.c:$(%BASEDIR)\src

.c.obj:
  $(CC) $(CFLAGS) $(MACHINE) $[*.c

OBJS = rexxfuncs.obj sysfile.obj sysmiscfile.obj systextscreen.obj sysversion.obj &
       sysdrive.obj syseautil.obj sysobjects.obj  sysprocess.obj syslang.obj      &
       sysmacro.obj sysstem.obj sysmisc.obj sysmutexsem.obj syseventsem.obj       &
       sysmath.obj helperfuncs.obj

#############################################################
#
#           ***** Project target procedures *****
#
#############################################################

all: .SYMBOLIC
  @%make bldsetup
  @wmake buildproc

# build debug rule
debug: .SYMBOLIC
  @set DEBUG=1
  @wmake -f makefile.wat

#############################################################
#
#           ***** Project clean procedures *****
#
#############################################################

clean: .SYMBOLIC
CLEANEXTS   = obj exe err lst map sym lib dll bak icm
  @%make bldsetup
  @if EXIST .\build  @cd .\build
  @for %a in ($(CLEANEXTS))  do -@rm *.%a
  @cd $(%BASEDIR)
  @if EXIST .\build  -@rd .\build
  @if EXIST $(NAMEDLL).dll rm $(NAMEDLL).dll
  @echo Clean complete.


#############################################################
#
#       ***** Project target build procedures *****
#
#############################################################

buildproc: .PROCEDURE
!ifdef %DEBUG
  @echo DEBUG
!endif
  @echo $(%BASEDIR)
  @echo $(%SRCDIR)
  @if NOT EXIST .\build @mkdir .\build
  @cd .\build
  @%make $(NAMEDLL).dll
  @if EXIST $(NAMEDLL).dll @copy $(NAMEDLL).dll $(%BASEDIR)
  @cd $(%BASEDIR)
!ifneq %DEBUG 1
  -@lxlite $(NAMEDLL).dll
!endif

# build dll for platform
$(NAMEDLL).dll: $(OBJS)   .PROCEDURE
  $(LD) NAME $* @$(%BASEDIR)\rxutil.lnk $(LDFLAGS)  FILE {$(OBJS)}


# gets required directories and sets environmental variables:
#
# BASEDIR  the current directory
# BUILDDIR the project build directory
#
bldsetup: .PROCEDURE
  @set BASEDIR=$(%cdrive):$(%cwd)
  @set SRCDIR=$(BASEDIR)\src



