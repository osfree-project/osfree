#
# Makefile for XWorkplace National Language Support.
#
# A makefile contains information about how several source files
# relate to each other, especially which code modules must be updated
# when certain source files change.
#
# For use with IBM NMAKE, which is part of IBM C-Set/2 and VisualAge C++.
# This file will probably not work with other MAKE utilities, such as
# GNU make or DMAKE.
#
# Copyright (C) 1997-2006 Ulrich M”ller.
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, in version 2 as it comes in the COPYING
# file of the XWorkplace main distribution.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# Required changes for translating the NLS DLL: three lines below.
#

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# This is the language code for the language that the NLS DLL
# will support. Change this to a different language code, and
# the makefile will support your language. The language code
# MUST have three digits exactly.
LANGUAGE = 049

#
# DO NOT CHANGE THE FOLLOWING
#

#
# Set up a few environment variables.
#

# OUTPUTDIR specifies the directory where all the output .OBJ
# files will be created in.
OUTPUTDIR = $(XWP_OUTPUT_ROOT)

# create output directory
!if [@md $(OUTPUTDIR) 2> NUL]
!endif

OBJS = $(OUTPUTDIR)\xfldr$(LANGUAGE).obj

XFLDHELP = $(XWPRUNNING)\help

all: $(XWPRUNNING)\bin\xfldr$(LANGUAGE).dll
    @echo ----- Leaving $(MAKEDIR)

# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.

.c{$(OUTPUTDIR)}.obj:
        $(CC_DLL_MT) -I$(INCLUDE) -c /Fo$(OUTPUTDIR)\$(@B).obj $<

#
# Now define inference rules for the different file types.
#

# V1.00: the following has been removed
# xfldr$(LANGUAGE).msg: $$(@B).txt
#          mkmsgf $*.txt $*.msg

$(OUTPUTDIR)\xfldr$(LANGUAGE).obj: $(@B).c

$(OUTPUTDIR)\xfldr$(LANGUAGE).res: *.rc *.dlg *.ico ..\..\include\dlgids.h
         $(RC) -r -x2 $(@B).rc $(OUTPUTDIR)\$(@B).res

$(MODULESDIR)\xfldr$(LANGUAGE).dll: $(@B).def $(OBJS) $(OUTPUTDIR)\$(@B).res ..\..\include\dlgids.h
         $(LINK_BASE) $(@B).def $(OBJS) /OUT:$(MODULESDIR)\$(@B).dll
         $(RC) -p -x2 $(OUTPUTDIR)\$(@B).res $(MODULESDIR)\$(@B).dll

# main target
$(XWPRUNNING)\bin\xfldr$(LANGUAGE).dll: $(MODULESDIR)\xfldr$(LANGUAGE).dll
!ifdef XWP_UNLOCK_MODULES
         $(RUN_UNLOCK) $(XWPRUNNING)\bin\$(@B).dll
!endif
         $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\bin


