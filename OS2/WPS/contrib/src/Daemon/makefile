#
# Makefile for XWPDAEMN.EXE.
#
# See make\readme.txt about an introduction to the make system introduced
# in the CVS trunk on 2003-01-28.
#
#       Copyright (C) 1998-2006 Ulrich M”ller.
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

# ***************************************************************************
# *
# *     1) include generic setup definitions (compiler options etc.)
# *
# ***************************************************************************

!include ..\..\config.in
!include ..\..\make\setup.in

# ***************************************************************************
# *
# *     2) include mode-specific compiler inference rules
# *
# ***************************************************************************

!include $(MAKE_INCLUDE_DIR)\compile_exe_mt.in
#       this defines the proper OUTPUTDIR

# ***************************************************************************
# *
# *     3) list objects to be built
# *
# ***************************************************************************

MODULESTEM = xwpdaemn

# The PGRDAEMNOBJS macro contains all the XPager .OBJ files,
# which go into XWPDAEMN.EXE.
PGRDAEMNOBJS = \
$(OUTPUTDIR)\pg_control.obj \
$(OUTPUTDIR)\pg_move.obj \
$(OUTPUTDIR)\pg_winlist.obj

# The DMNOBJS macro contains all the .OBJ files for XWPDAEMN.EXE;
# this includes PGRDAEMNOBJS.
OBJS = \
$(OUTPUTDIR)\xwpdaemn.obj \
$(OUTPUTDIR)\drivemonitor.obj \
$(OUTPUTDIR)\watchdog.obj \
$(PGRDAEMNOBJS)

HLPOBJS = \
$(OUTPUTDIR)\debug.obj \
$(OUTPUTDIR)\gpih.obj \
$(OUTPUTDIR)\linklist.obj \
$(OUTPUTDIR)\except.obj \
$(OUTPUTDIR)\dosh.obj \
$(OUTPUTDIR)\dosh2.obj \
$(OUTPUTDIR)\level.obj \
$(OUTPUTDIR)\procstat.obj \
$(OUTPUTDIR)\regexp.obj \
$(OUTPUTDIR)\shapewin.obj \
$(OUTPUTDIR)\stringh.obj \
$(OUTPUTDIR)\threads.obj \
$(OUTPUTDIR)\nls.obj \
$(OUTPUTDIR)\prfh.obj \
!ifdef XWP_DEBUG
$(OUTPUTDIR)\memdebug.obj \
$(OUTPUTDIR)\tree.obj \
!endif
$(OUTPUTDIR)\xstring.obj \
$(XWP_OUTPUT_ROOT)\xwphook.lib

# extra LINKOBJS macro because we want to link $(OBJS) and $(HLPOBJS),
# but never compile $(HLPOBJS) from here
LINKOBJS = $(OBJS) $(HLPOBJS)

# ***************************************************************************
# *
# *     4) define specific stuff for linker include
# *
# ***************************************************************************

BLDLEVELNAME = "$(XWPNAME) PM daemon"

COPYTODIR = $(XWPRUNNING)\bin

LINKMODULE = $(MODULESTEM).exe

RESOURCE_SOURCES = $(MODULESTEM).rc *.bmp $(MODULESTEM).ico *.ptr

# ***************************************************************************
# *
# *     5) link executable
# *
# ***************************************************************************

# do not allow unlocking the daemon!
!undef XWP_UNLOCK_MODULES
!include $(MAKE_INCLUDE_DIR)\link_exe.in
#   this defines ALLTARGET for the next include

# ***************************************************************************
# *
# *     6) define main target
# *
# ***************************************************************************

!include $(MAKE_INCLUDE_DIR)\targets.in

# create import library from XWPHOOK.DLL
$(XWP_OUTPUT_ROOT)\xwphook.lib: $(MODULESDIR)\$(@B).dll
    implib /nologo $(XWP_OUTPUT_ROOT)\$(@B).lib $(MODULESDIR)\$(@B).dll


