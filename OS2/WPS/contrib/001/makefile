
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

# Say hello to yourself.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

# include setup (compiler options etc.)
!include ..\config.in
!include ..\make\setup.in

# This is the language code for the language that the NLS DLL
# will support. Change this to a different language code, and
# the makefile will support your language. The language code
# MUST have three digits exactly.
LANGUAGE = 001

#
# DO NOT CHANGE THE FOLLOWING
#

all:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(LANGUAGE)\dll
    @cd dll
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(LANGUAGE)\inf.$(LANGUAGE)
    @cd inf.$(LANGUAGE)
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(LANGUAGE)\misc
    @cd misc
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(LANGUAGE)\xwphelp2
    @cd xwphelp2
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..\..
    @echo ----- Leaving $(MAKEDIR)

running:
    $(COPY) readme $(XWPRUNNING)

release:
    $(COPY) readme $(XWPRELEASE_NLSDOC)

release_nls:
    $(COPY) readme $(XWPRELEASE_NLSDOC)

