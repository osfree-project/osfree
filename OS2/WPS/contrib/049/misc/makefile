
#
# makefile:
#       makefile for 001\misc directory.
#       For use with IBM NMAKE, which comes with the IBM compilers,
#       the Developer's Toolkit, and the DDK.
#
#       All the makefiles have been restructured with V0.9.0.
#
#       Called from:    001
#
#       This copies the files in the current directory to
#       XWPRUNNING subdirectories, if necessary.

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# This is the language code for the language that the NLS DLL
# will support. Change this to a different language code, and
# the makefile will support your langauge.
LANGUAGE = 049

#
# DO NOT CHANGE THE FOLLOWING
#

FILES_TO_COPY = \
    $(XWPRUNNING)\help\xfldr$(LANGUAGE).tmf \
    $(XWPRUNNING)\install\crobj$(LANGUAGE).cmd \
    $(XWPRUNNING)\install\instl$(LANGUAGE).cmd \
!ifndef XWPLITE
    $(XWPRUNNING)\install\sound$(LANGUAGE).cmd \
    $(XWPRUNNING)\help\drvrs$(LANGUAGE).txt \
    $(XWPRUNNING)\help\xfcls$(LANGUAGE).txt
!endif

all: $(FILES_TO_COPY)
    @echo ----- Leaving $(MAKEDIR)

# copy tmf file to modulesdir first
$(XWPRUNNING)\help\xfldr$(LANGUAGE).tmf: $(MODULESDIR)\$(@B).tmf
    $(COPY) $(MODULESDIR)\$(@B).tmf $@

# compose tmf file in modulesdir from several source tmf files
!ifdef XWPLITE
$(MODULESDIR)\xfldr$(LANGUAGE).tmf: xfldr$(LANGUAGE).tmf xwplite$(LANGUAGE).tmf
    $(COPY) /b $(@B).tmf+xwplite$(LANGUAGE).tmf $@
!else
$(MODULESDIR)\xfldr$(LANGUAGE).tmf: xfldr$(LANGUAGE).tmf xwpfull$(LANGUAGE).tmf
    $(COPY) /b $(@B).tmf+xwpfull$(LANGUAGE).tmf $@
!endif

# crobjXXX.cmd

!ifdef XWPLITE
CROBJ_SOURCE = crobj$(LANGUAGE)_lite.cmd
!else
CROBJ_SOURCE = crobj$(LANGUAGE).cmd
!endif

$(XWPRUNNING)\install\crobj$(LANGUAGE).cmd: $(CROBJ_SOURCE)
    $(COPY) $(CROBJ_SOURCE) $(XWPRUNNING)\install\crobj$(LANGUAGE).cmd

# instlXXX.cmd

!ifdef XWPLITE
INSTL_SOURCE = instl$(LANGUAGE)_lite.cmd
!else
INSTL_SOURCE = instl$(LANGUAGE).cmd
!endif

$(XWPRUNNING)\install\instl$(LANGUAGE).cmd: $(INSTL_SOURCE)
    $(COPY) $(INSTL_SOURCE) $(XWPRUNNING)\install\instl$(LANGUAGE).cmd

$(XWPRUNNING)\install\sound$(LANGUAGE).cmd: $(@B).cmd
    $(COPY) $(@B).cmd $(XWPRUNNING)\install

# *.txt files

$(XWPRUNNING)\help\drvrs$(LANGUAGE).txt: $(@B).txt
    $(COPY) $(@B).txt $(XWPRUNNING)\help

$(XWPRUNNING)\help\xfcls$(LANGUAGE).txt: $(@B).txt
    $(COPY) $(@B).txt $(XWPRUNNING)\help

