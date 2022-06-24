#
# Makefile for xwpsec32.sys.
#
# For use with IBM NMAKE, which is part of IBM C-Set/2 and VisualAge C++.
# This file will probably not work with other MAKE utilities, such as
# GNU make or DMAKE.
#
#       Copyright (C) 1998-2008 Ulrich M”ller.
#       This file is part of the XWorkplace source package.
#       XWorkplace is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published
#       by the Free Software Foundation, in version 2 as it comes in the
#       "COPYING" file of the XWorkplace main distribution.
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

!ifndef DDKBASE
!error DDKBASE is not defined.
!endif

!ifndef SESBASE
!error SESBASE is not defined.
!endif

MYDIR = $(MAKEDIR)

# OUTPUTDIR specifies the directory where all the output .OBJ
# files will be created in. $(XWP_OUTPUT_ROOT) is set by
# setup.in to point to \bin from the XWorkplace sources root,
# but this can be modified by setting an external environment
# variable. This approach has the advantage that
# 1) all build files are in a common dir tree and the entire
#    tree can be removed completely;
# 2) the build tree can be on a different physical drive for
#    speed.
MAIN_OUTPUTDIR = $(XWP_OUTPUT_ROOT)
OUTPUTDIR=$(MAIN_OUTPUTDIR)\xwpsec32.sys
MODULESDIR=$(MAIN_OUTPUTDIR)\modules

# create output directory
!if [@md $(OUTPUTDIR) 2> NUL]
!endif

# FIXED MACROS
# ------------

MYDIR = $(MAKEDIR)

INC = ..\..\include

MYINC = $(INC)\xwpsec32.sys

CC        = icc -q
CFLAGS    = -Ge- -Gs- -Gr+ -Rn -Ss+ -O -G5 -w2 /tn+

LD        = icc

AS        = alp
ASFLAGS   = -Mb -Li +Sv:M510

CDEFINES   =
# -DDYNAMIC_DEVHELP -DMWDD32_VER=$(MWDD32_VERSION)
ADEFINES   =
#-D:DYNAMIC_DEVHELP
CINCLUDES  = /I$(PROJECT_INCLUDE)

DRIVERNAME=xwpsec32

AINCLUDES  = -Fdi:$(DDKBASE)/inc -Fdi:../../include/xwpsec32.sys

AOBJS = \
    $(OUTPUTDIR)\sec32_pre_init_base.obj $(OUTPUTDIR)\sec32_devhlp32.obj

COBJS = \
    $(OUTPUTDIR)\callb_close.obj \
    $(OUTPUTDIR)\callb_delete.obj \
    $(OUTPUTDIR)\callb_dirs.obj \
    $(OUTPUTDIR)\callb_exec.obj \
    $(OUTPUTDIR)\callb_find.obj \
    $(OUTPUTDIR)\callb_move.obj \
    $(OUTPUTDIR)\callb_open.obj \
    $(OUTPUTDIR)\callb_trustedpath.obj \
    $(OUTPUTDIR)\sec32_strategy.obj \
    $(OUTPUTDIR)\sec32_data.obj \
    $(OUTPUTDIR)\sec32_callbacks.obj \
    $(OUTPUTDIR)\sec32_contexts.obj \
    $(OUTPUTDIR)\strat_init_base.obj \
    $(OUTPUTDIR)\strat_init_complete.obj \
    $(OUTPUTDIR)\strat_ioctl.obj \
    $(OUTPUTDIR)\strat_shutdown.obj \
    $(OUTPUTDIR)\util_misc.obj \
    $(OUTPUTDIR)\util_string.obj \
    $(OUTPUTDIR)\tree.obj

# update DEF file
!if [@$(RUN_BLDLEVEL) $(DRIVERNAME).def ..\..\include\bldlevel.h "$(XWPNAME) Security device driver"]
!endif

# TARGETS
# -------

all: $(MODULESDIR)\$(DRIVERNAME).sys
    @echo ----- Leaving $(MAKEDIR)

# The "dep" target: run fastdep on the sources.
# "nmake dep" gets called from src\makefile if nmake dep
# is running on the main makefile.
dep:
    $(RUN_FASTDEP) *.c
    @echo ----- Leaving $(MAKEDIR)

.c{$(OUTPUTDIR)}.obj:
    @echo Compiling $<
    @echo CINCLUDES: $(CINCLUDES)
    $(CC) -c $(CFLAGS) $(CDEFINES) -Fa$(@:.obj=.s) -Fo$@ $(CINCLUDES) $<

.asm{$(OUTPUTDIR)}.obj:
    @echo Assembling $<
    $(AS) $(ASFLAGS) $(ADEFINES) $(AINCLUDES) $< -Fo:$@ -Fl:$(@:.obj=.lst)

# linking
$(MODULESDIR)\$(DRIVERNAME).sys: \
$(AOBJS) \
$(COBJS) \
$(OUTPUTDIR)\sec32_start.obj \
$(OUTPUTDIR)\sec32_end.obj \
$(DRIVERNAME).def \
makefile
    @echo Linking $@:
    ilink /nologo @<<
/OUT:$(MODULESDIR)\$(@B).sys
/map:$(MODULESDIR)\$(@B).map
/EXEPACK:2 /nod /ALIGNMENT:4 /linenumbers
$(OUTPUTDIR)\sec32_start.obj
$(AOBJS)
$(COBJS)
$(DDKBASE)\lib\os2386p.lib
$(DDKBASE)\lib\vdh.lib
$(OUTPUTDIR)\sec32_end.obj
$(DRIVERNAME).def
<<
# /NOD
!ifdef XWP_OUTPUT_ROOT_DRIVE
    @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
    @cd $(MODULESDIR)
    mapsym $(DRIVERNAME).map
!ifdef CVS_WORK_ROOT_DRIVE
    @$(CVS_WORK_ROOT_DRIVE)
!endif
    @cd $(MYDIR)
    -$(COPY) $@             E:\os2
    -$(COPY) $(@:.sys=.sym) E:\os2
    -$(COPY) $@             F:\os2\boot
    -$(COPY) $(@:.sys=.sym) F:\os2\boot
    -$(COPY) $@             I:\os2\boot
    -$(COPY) $(@:.sys=.sym) I:\os2\boot
!ifdef PMDFDIR
    -$(COPY) $(@:.sys=.sym) $(PMDFDIR)
!endif


# The .OBJ-from-sources dependencies are now automatically
# created by "nmake dep" into the .depend include file.
# V0.9.12 (2001-05-22) [umoeller]

!ifndef NOINCLUDEDEPEND
!include .depend
!endif



