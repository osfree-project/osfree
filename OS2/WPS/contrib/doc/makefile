#
# Makefile for XWorkplace National Language Support.
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

# Say hello to yourself.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

!include ..\config.in
!include ..\make\setup.in

#
# First define the suffixes for files which NMAKE will work on.
# .SUFFIXES is a reserved NMAKE keyword ("pseudotarget") for
# defining file extensions that NMAKE will recognize.
#

.SUFFIXES: .html .ipf .hlp .inf

#
# Now set up a few environment variables.
#

FILESTEM = progref

# This is the language code for the language that the NLS DLL
# will support. Change this to a different language code, and
# the makefile will support your langauge.

# Change this to the full path of the HTML2IPF utility.
HTML2IPF = ..\001\tools\html2ipf.cmd

# Change this to the directory where the output file (INF)
# should be copied to.
COPYTO = ..

#
# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.
#

.html.ipf:
        $(HTML2IPF) $(FILESTEM).html

.ipf.inf:
        ipfc /inf $*.ipf $*.inf

#
# Now define inference rules for the different file types.
#

$(COPYTO)\$(FILESTEM).inf: $(FILESTEM).inf
        $(COPY) $(@B).inf $(COPYTO)
        del $(@B).inf

$(FILESTEM).inf: *.ipf

$(FILESTEM).ipf: *.html

