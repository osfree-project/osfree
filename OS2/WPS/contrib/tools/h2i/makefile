#
# makefile:
#       makefile for TOOLS\xlvm directory.
#       For use with IBM NMAKE, which comes with the IBM compilers,
#       the Developer's Toolkit, and the DDK.
#
#       All the makefiles have been restructured with V0.9.0.
#
#       Called from:    main makefile
#
#       Input:          files in this directory
#
#       Output:         xlvm.exe, copied to parent directory (TOOLS\).
#
#       Edit "setup.in" to set up the make process.
#

# include setup (compiler options etc.)
!include ..\..\config.in
!include ..\..\make\setup.in

# FIXED MACROS
# ------------
#
# You probably need not change the following.
#

# OUTPUTDIR specifies the directory where all the output .OBJ
# files will be created in.
OUTPUTDIR = $(XWP_OUTPUT_ROOT)\exe_mt

COPYTODIR = ..

!ifndef XWP_DEBUG
COPYTODIR = $(HELPERS_BASE_DIR)
!endif

!if [@md $(XWP_OUTPUT_ROOT) 2> NUL]
!endif
!if [@md $(OUTPUTDIR) 2> NUL]
!endif

APPNAME = h2i

# The OBJS macro contains all the .OBJ files which need to be
# created from the files in this directory.
# These will be put into BIN\.
OBJS = $(OUTPUTDIR)\$(APPNAME).obj $(OUTPUTDIR)\helpers.lib

# The main target:
# If we're called from the main makefile, MAINMAKERUNNING is defined,
# and we'll set $(OBJS) as our targets (which will go on).
# Otherwise, we call the main makefile, which will again call ourselves later.
all: helpers \
    $(MODULESDIR)\$(APPNAME).exe
    @echo ----- Leaving $(MAKEDIR)

SUBMAKE_PASS_STRING = "PROJECT_BASE_DIR=$(PROJECT_BASE_DIR)" "PROJECT_INCLUDE=$(PROJECT_INCLUDE)"

helpers:
    @cd $(HELPERS_BASE_DIR)\src\helpers
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING) \
"HELPERS_OUTPUT_DIR=$(XWP_OUTPUT_ROOT)\exe_mt" "CC_HELPERS=$(CC_HELPERS_EXE_MT)"
    @cd $(MAKEDIR)

# Now define inference rules for what to do with certain file
# types, based on their file extension.
# The syntax we need here is ".fromext.toext".
# So whenever NMAKE encounters a .toext file, it
# executes what we specify here.
# The ugly {} brackets are some awkward syntax for specifying
# files in other directories.

# Special macros used here: $(@B) is the current target w/out ext.

# -- compile C files to .OBJ files, using the CC macro above.
#    The output will be placed in the directory specified by
#    the OUTPUTDIR variable (set above).

.cpp.{$(OUTPUTDIR)}.obj:
        @echo $(MAKEDIR)\makefile: Compiling $(@B).cpp
!ifndef PRECH
# precompiled headers disabled:
        $(CC_EXE_MT) /w1 /Fo$(OUTPUTDIR)\$(@B).obj $(@B).cpp
!else
        $(CC_EXE_MT) /w1 /fi"$(PRECH)\$(@B).pch" /si"$(PRECH)\$(@B).pch" /Fo$(OUTPUTDIR)\$(@B).obj $(@B).cpp
!endif

# main include path
INC = ..\..\include
# base include path
BSINC = $(INC)\base

# Now define dependencies for all our .OBJ files by telling NMAKE
# which .OBJ file depends on what. For each .OBJ we define what files
# will enforce a recompilation when they've been changed.
# This uses the inference rules defined above.

# Note that include\setup.h is specified with _all_ files, so
# you can enforce a complete recompile of XWorkplace if you update
# that file (just save it once).

$(OUTPUTDIR)\$(APPNAME).obj: \
    $(INC)\setup.h $(INC)\bldlevel.h \
    $(APPNAME).h

# build targets
$(APPNAME).def: ..\..\include\bldlevel.h makefile
          $(RUN_BLDLEVEL) $*.def ..\..\include\bldlevel.h "HTML-to-IPF converter"

$(MODULESDIR)\$(APPNAME).exe: $(OBJS) $(@B).def makefile
          $(LINK) /out:$(MODULESDIR)\$(@B).exe $(OBJS) $(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
          cd $(MODULESDIR)
          mapsym /n $(@B).map > NUL
!ifdef COPYTODIR
          $(COPY) $(@B).sym $(COPYTODIR)
          $(COPY) $(@B).exe $(COPYTODIR)
!endif
!ifdef COMMONBINDIR
          $(COPY) $(@B).sym $(COMMONBINDIR)
          $(COPY) $(@B).exe $(COMMONBINDIR)
!endif
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
          cd $(MAKEDIR)


