#
# makefile:
#       makefile for IDL directory.
#       For use with IBM NMAKE, which comes with the IBM compilers,
#       the Developer's Toolkit, and the DDK.
#
#       Called from:    main makefile
#
#       Input:          ./*.IDL
#
#       Output:         ../include/classes/*.h
#                       ../include/classes/*.ih
#                       ../src/classes/*.c
#

!include ..\config.in
!include ..\make\setup.in

# OUTPUTDIR specifies the directory where we will put the
# files created by this makefile.
OUTPUTDIR = $(PROJECT_BASE_DIR)\include\classes

# !undef SMINCLUDE

# Some SOM variables. You won't have to change these.
SOMTEMP = $(TEMP)
!if [set SOMTMP=$(TEMP)] || \
    [set SMTMP=$(SOMTEMP)] || \
    [set SMEMIT=ih;h;c] || \
    [set SMADDSTAR=1] || \
    [set SMNOTC=1] || \
#     [set SMINCLUDE=$(SMINCLUDE);$(TKBASE)\idl;$(TKBASE)\h;$(TKBASE)\SOM\include;.]
#   hacked up SMINCLUDE to make sure idl\wps is before toolkit idl\
#   V0.9.20 (2002-07-25) [umoeller]
    [set SMINCLUDE=$(PROJECT_BASE_DIR)\idl\wps;$(SMINCLUDE);$(TKBASE)\idl;$(TKBASE)\SOM\include;.]
!endif

!if [@echo    TKBASE    is $(TKBASE)]
!endif
!if [@echo    SMINCLUDE is $(SMINCLUDE)]
!endif

# SMTMP=$(SOMTEMP)
# SMEMIT=ih;h;c
# SMADDSTAR=1
# SMNOTC=1

# Specify the main target.

# added all the following from wps\ to avoid problems with warp 4 toolkit
# V0.9.20 (2002-07-31) [umoeller]
all: \
$(OUTPUTDIR)\wps\wpabs.h \
$(OUTPUTDIR)\wps\wpdataf.h \
$(OUTPUTDIR)\wps\wpdesk.h \
$(OUTPUTDIR)\wps\wpdisk.h \
$(OUTPUTDIR)\wps\wpfolder.h \
$(OUTPUTDIR)\wps\wpfsys.h \
$(OUTPUTDIR)\wps\wpimage.h \
$(OUTPUTDIR)\wps\wpkeybd.h \
$(OUTPUTDIR)\wps\wpmouse.h \
$(OUTPUTDIR)\wps\wpobject.h \
$(OUTPUTDIR)\wps\wppgm.h \
$(OUTPUTDIR)\wps\wppgmf.h \
$(OUTPUTDIR)\wps\wpshadow.h \
$(OUTPUTDIR)\wps\wpsound.h \
$(OUTPUTDIR)\wps\wpsystem.h \
$(OUTPUTDIR)\wps\wptrans.h \
$(OUTPUTDIR)\xcenter.h \
$(OUTPUTDIR)\xclslist.h \
$(OUTPUTDIR)\xdebug.h \
$(OUTPUTDIR)\xdebug_folder.h \
$(OUTPUTDIR)\xfdataf.h \
$(OUTPUTDIR)\xfdesk.h \
$(OUTPUTDIR)\xfdisk.h \
$(OUTPUTDIR)\xfldr.h \
$(OUTPUTDIR)\xfobj.h \
$(OUTPUTDIR)\xfont.h \
$(OUTPUTDIR)\xfontfile.h \
$(OUTPUTDIR)\xfontobj.h \
$(OUTPUTDIR)\xwppgmf.h \
$(OUTPUTDIR)\xfstart.h \
$(OUTPUTDIR)\xfshut.h \
$(OUTPUTDIR)\xfsys.h \
$(OUTPUTDIR)\xfwps.h \
$(OUTPUTDIR)\xtrash.h \
$(OUTPUTDIR)\xtrashobj.h \
$(OUTPUTDIR)\xmmcdplay.h \
$(OUTPUTDIR)\xmmvolume.h \
$(OUTPUTDIR)\xwpadmin.h \
$(OUTPUTDIR)\xwpfsys.h \
$(OUTPUTDIR)\xwpimage.h \
$(OUTPUTDIR)\xwpkeybd.h \
$(OUTPUTDIR)\xwplink.h \
$(OUTPUTDIR)\xwpmedia.h \
$(OUTPUTDIR)\xwpmouse.h \
$(OUTPUTDIR)\xwpnetwork.h \
$(OUTPUTDIR)\xwpnetsrv.h \
$(OUTPUTDIR)\xwppgm.h \
$(OUTPUTDIR)\xwpsetup.h \
$(OUTPUTDIR)\xwpscreen.h \
$(OUTPUTDIR)\xwpshadow.h \
$(OUTPUTDIR)\xwpsound.h \
$(OUTPUTDIR)\xwpstring.h \
$(OUTPUTDIR)\xwpvcard.h
    @echo ----- Leaving $(MAKEDIR)

# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.
# The ugly {} brackets are some awkward syntax for specifying
# files in other directories.

RUNSC = sc -p -v -r -maddstar -mnoint -S128000 -C128000

# 1) inference rule for hacked-up IBM WPS classes

# -- emit .IH, .DEF files when .IDL has changed
{wps}.idl{$(OUTPUTDIR)\wps}.h:
# emit H, IH (include\classes):
        $(RUNSC) -d$(OUTPUTDIR)\wps -sh;ih wps\$(@B).idl
# cleanup IH: this removes the ugly #pragma's for VAC++ 3.0
        $(RUN_STRRPL) $(OUTPUTDIR)\wps\$(@B).ih "#pragma checkout(suspend)" " "
        $(RUN_STRRPL) $(OUTPUTDIR)\wps\$(@B).ih "#pragma checkout(resume)" " "

# 2) inference rule for XWP classes

# -- emit .IH, .DEF files when .IDL has changed
.idl{$(OUTPUTDIR)}.h:
# emit H, IH (include\classes):
        $(RUNSC) -d$(OUTPUTDIR) -sh;ih $(@B).idl
# emit C (src\classes):
        $(RUNSC) -d$(PROJECT_BASE_DIR)\src\classes -sc $(@B).idl
# emit DEF (this dir):
        $(RUNSC) -s def $(@B).idl
# cleanup IH: this removes the ugly #pragma's for VAC++ 3.0
        $(RUN_STRRPL) $(OUTPUTDIR)\$(@B).ih "#pragma checkout(suspend)" " "
        $(RUN_STRRPL) $(OUTPUTDIR)\$(@B).ih "#pragma checkout(resume)" " "

# Now define dependencies to recreate the .H, .IH etc. files
# using the SOM inference rule above.

# Special macros used here: $(@B) is the current target w/out ext.

$(OUTPUTDIR)\wps\wpobject.h:     wps\$(@B).idl

$(OUTPUTDIR)\xfobj.h:            $(@B).idl $(OUTPUTDIR)\wps\wpobject.h

# WPAbstract and subclasses

$(OUTPUTDIR)\wps\wpabs.h:        wps\$(@B).idl $(OUTPUTDIR)\xfobj.h

$(OUTPUTDIR)\xcenter.h:          $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xclslist.h:         $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xfdisk.h:           $(@B).idl  wps\wpdisk.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xfsys.h:            $(@B).idl  wps\wpsystem.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xfwps.h:            $(@B).idl  wps\wpsystem.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xmmcdplay.h:        $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xmmvolume.h:        $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpadmin.h:         $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpkeybd.h:         $(@B).idl wps\wpkeybd.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpmedia.h:         $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpmouse.h:         $(@B).idl wps\wpmouse.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwppgm.h:           $(@B).idl wps\wppgm.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpscreen.h:        $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpsetup.h:         $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpshadow.h:        $(@B).idl wps\wpshadow.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpsound.h:         $(@B).idl wps\wpsound.idl $(OUTPUTDIR)\wps\wpabs.h

$(OUTPUTDIR)\xwpstring.h:        $(@B).idl $(OUTPUTDIR)\wps\wpabs.h

# WPTransient and subclasses

$(OUTPUTDIR)\wps\wptrans.h:      wps\$(@B).idl $(OUTPUTDIR)\xfobj.h

$(OUTPUTDIR)\xfontobj.h:         $(@B).idl $(OUTPUTDIR)\wps\wptrans.h

$(OUTPUTDIR)\xtrashobj.h:        $(@B).idl $(OUTPUTDIR)\wps\wptrans.h

# WPFileSystem and subclasses

$(OUTPUTDIR)\wps\wpfsys.h:       wps\$(@B).idl $(OUTPUTDIR)\xfobj.h

$(OUTPUTDIR)\xwpfsys.h:          $(@B).idl $(OUTPUTDIR)\wps\wpfsys.h

# WPFolder and subclasses

$(OUTPUTDIR)\wps\wpfolder.h:     wps\$(@B).idl $(OUTPUTDIR)\xwpfsys.h

$(OUTPUTDIR)\xfldr.h:            $(@B).idl $(OUTPUTDIR)\wps\wpfolder.h

$(OUTPUTDIR)\xfdesk.h:           $(@B).idl $(OUTPUTDIR)\xfldr.h

$(OUTPUTDIR)\xfont.h:            $(@B).idl $(OUTPUTDIR)\xfldr.h

$(OUTPUTDIR)\xfstart.h:          $(@B).idl $(OUTPUTDIR)\xfldr.h

$(OUTPUTDIR)\xfshut.h:           $(@B).idl $(OUTPUTDIR)\xfldr.h

$(OUTPUTDIR)\xtrash.h:           $(@B).idl $(OUTPUTDIR)\xfldr.h

$(OUTPUTDIR)\xwpnetwork.h:       $(@B).idl

$(OUTPUTDIR)\xwpnetsrv.h:        $(@B).idl

# WPDataFile and subclasses

$(OUTPUTDIR)\wps\wpdataf.h:      wps\$(@B).idl $(OUTPUTDIR)\xwpfsys.h

$(OUTPUTDIR)\xfdataf.h:          $(@B).idl $(OUTPUTDIR)\wps\wpdataf.h

$(OUTPUTDIR)\wps\wpimage.h:      wps\$(@B).idl $(OUTPUTDIR)\xfdataf.h

$(OUTPUTDIR)\xwpimage.h:         $(@B).idl $(OUTPUTDIR)\wps\wpimage.h

$(OUTPUTDIR)\xfontfile.h:        $(@B).idl $(OUTPUTDIR)\xfdataf.h

$(OUTPUTDIR)\xwplink.h:          $(@B).idl $(OUTPUTDIR)\xfdataf.h

$(OUTPUTDIR)\xwppgmf.h:          $(@B).idl $(OUTPUTDIR)\xfdataf.h

$(OUTPUTDIR)\xwpvcard.h:         $(@B).idl $(OUTPUTDIR)\xfdataf.h



$(OUTPUTDIR)\xdebug.h:           $(@B).idl

$(OUTPUTDIR)\xdebug_folder.h:    $(@B).idl


