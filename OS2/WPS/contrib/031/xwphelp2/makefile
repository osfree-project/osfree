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

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# OUTPUTDIR specifies the directory where all the output .OBJ
# files will be created in.
OUTPUTDIR = $(XWP_OUTPUT_ROOT)\modules
!if [@echo       OUTPUTDIR is $(OUTPUTDIR)]
!endif

# create output directory
!if [@md $(OUTPUTDIR) 2> NUL]
!endif

#
# Now set up a few environment variables.
#

# This is the language code for the language that the NLS DLL
# will support. Change this to a different language code, and
# the makefile will support your langauge.
LANGUAGE = 031

FEATURES_STRING =
!ifdef XWPLITE
FEATURES_STRING = -i..\..\include\_features_lite.h
!endif

# Change this to the full path of the HTML2IPF utility.
# THIS HAS CHANGED WITH V0.84.
# now using h2i V0.9.13 (2001-06-23) [umoeller]
HTML2IPF = $(RUN_H2I) -s -i$(PROJECT_BASE_DIR)\include\shared\helppanels.h -i..\$(ENTITYDEFS) $(FEATURES_STRING)

# Change this to the directory where the output file (HLP)
# should be copied to.
!ifndef XWPRUNNING
!error The environment variable XWPRUNNING is not defined. Terminating.
!endif
COPYTO = $(XWPRUNNING)\help

# DO NOT CHANGE THE FOLLOWING

# Define the suffixes for files which NMAKE will work on.
# .SUFFIXES is a reserved NMAKE keyword ("pseudotarget") for
# defining file extensions that NMAKE will recognize.
.SUFFIXES: .html .ipf .hlp .inf .gif

#
# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.
#
{img}.gif{img}.bmp:
    -gbmsize $< $@,1.1

# The BMPS macro contains all the .BMP files which need to be
# created from the files in this directory.
# If you add a new .GIF, add the corresponding .BMP file here.
BMPS = \
    img\cascade.bmp \
    img\fonthint.bmp \
    img\folderview.bmp \
    img\nocascade.bmp \
    img\notrunc.bmp \
    img\properties.bmp \
    img\string.bmp \
    img\titleedit.bmp \
    img\trunc.bmp \
    img\xcenter_view.bmp \
    img\xc_drag.bmp \
    img\xc_mem.bmp \
    img\xc_power1.bmp \
    img\xc_power2.bmp \
    img\xc_pulse.bmp \
    img\xc_sentinel1.bmp \
    img\xc_sentinel2.bmp \
    img\xc_sizbar.bmp \
    img\xc_tray.bmp \
    img\xc_winlist.bmp \
    img\xfsys.bmp \
    img\xfwps.bmp \
    img\xwpmedia.bmp \
    img\xwpscreen.bmp \
    img\xwsetup.bmp \
    img\trashcan.bmp \
    img\os2infofdr.bmp \
    img\setupfdr.bmp \
    img\setupfdr_mini.bmp \
    img\os2system.bmp \
    img\os2system_mini.bmp \
    img\os2cmdpfdr.bmp \
    img\os2cmdpfdr_mini.bmp \
    img\os2drives.bmp \
    img\os2drives_mini.bmp \
    img\disk_diskette.bmp \
    img\disk_harddisk.bmp \
    img\disk_network.bmp \
    img\disk_cdrom.bmp \
    img\os2_dosfull.bmp \
    img\os2_doswin.bmp \
    img\os2_os2full.bmp \
    img\os2_os2win.bmp \
    img\os2_win16.bmp \
    img\foldericon.bmp \
    img\pager.bmp \
    img\os2wpstartup.bmp \
    img\os2minwinv.bmp \
    img\os2temps.bmp \
    img\os2temps_mini.bmp \
    img\template.bmp \
    img\templ_pgm.bmp \
    img\os2games.bmp \
    img\os2warpcenter.bmp \
    img\os2clipview.bmp \
    img\notebook.bmp \
    img\os2iconedit.bmp \
    img\xfwps_mini.bmp \
    img\os2eexe.bmp \
    img\desktop.bmp \
    img\desktop_mini.bmp \
    img\selection.bmp \
    img\fontsfdr.bmp \
    img\os2pgmfdr.bmp \
    img\os2internet.bmp \
    img\os2wpnetwork.bmp \
    img\os2wpnetgrp.bmp \
    img\os2wpnetgrp_mini.bmp \
    img\os2wpserver.bmp \
    img\os2wpserver_mini.bmp \
    img\setup_look.bmp \
    img\setup_look_mini.bmp \
    img\estyler.bmp \
    img\os2country.bmp \
    img\drag.bmp \
    img\net_clientserver.bmp \
    img\net_peer.bmp \
    img\os2wpprinter.bmp \
    img\os2wpprinter_mini.bmp \
    img\os2instrem.bmp

# main target
all: $(BMPS) $(COPYTO)\xfldr$(LANGUAGE).hlp
    @echo ----- Leaving $(MAKEDIR)


SRCHTML = *.html \
fsys\*.html \
glossary\*.html \
hook\*.html \
kernel\*.html \
media\*.html \
menus\*.html \
objects\*.html \
setup\*.html \
tasks\*.html \
wps\*.html \
xcenter\*.html \
xshutdown\*.html

#
# Now define inference rules for the different file types.
#

IPFFILE = $(OUTPUTDIR)\hlp$(LANGUAGE).ipf

$(COPYTO)\xfldr$(LANGUAGE).hlp: $(OUTPUTDIR)\xfldr$(LANGUAGE).hlp
# added unlock help V0.9.20 (2002-07-12) [umoeller]
    $(CVS_WORK_ROOT)\$(XWPDIR)\tools\unlockhelp.cmd
    $(COPY) $(OUTPUTDIR)\$(@B).hlp $(COPYTO)

$(IPFFILE): $(SRCHTML) makefile ..\$(ENTITYDEFS)
    $(HTML2IPF) xfldr$(LANGUAGE).html
    $(COPY) xfldr$(LANGUAGE).ipf $(IPFFILE)

$(OUTPUTDIR)\xfldr$(LANGUAGE).hlp: $(IPFFILE) img\*.bmp
    ipfc $(IPFFILE) $(OUTPUTDIR)\xfldr$(LANGUAGE).hlp



