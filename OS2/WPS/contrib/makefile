
#
# makefile:
#       This is XWorkplace's main makefile.
#       For use with IBM NMAKE, which comes with the IBM compilers,
#       the Developer's Toolkit, and the DDK.
#
#       All the makefiles have been restructured with V0.9.0.
#
#       Called from:    nowhere, this is the main makefile.
#                       This recurses into the subdirectories.
#
#       Input:          specify the target(s) to be made, which can be:
#
#                       --  "dep": create dependencies for all makefiles.
#                           Run this first before the regular "all"
#                           target.
#
#                       --  "all" (default): build XFLDR.DLL, XWPHOOK.DLL,
#                           XWPDAEMN.EXE, plus the default plugin DLLs and
#                           copy to XWorkplace install directory
#                           specified by XWPRUNNING.
#
#                       --  "really_all": build "all" plus external EXEs
#                           (Treesize, Netscape DDE, et al) plus NLS
#                           specified by the XWP_LANG_CODE variable,
#                           which defaults to "001" (setup.in) and
#       	            copy to XWorkplace install directory
#                           specified by XWPRUNNING.
#
#                       The following subtargets exist (which get called
#                       by the "all" or "really_all" targets):
#
#                       --  nls: compile $(XWP_LANG_CODE)\ directory
#                       --  tools: compile TOOLS\ directory
#                       --  idl: update SOM headers (include\classes\*)
#                       --  compile_all: compile main *.c files for DLLs, no link
#                       --  compile_really_all: compile all *.c files for DLLs, no link
#                       --  link: link bin\*.obj to DLLs and copy to XWorkplace
#                                 install directory specified by XWPRUNNING
#
#                       Use "nmake -a [<targets>] to _re_build the targets,
#                       even if they are up to date.
#
#                       Other special targets not used by "all" or "really_all"
#                       (these must be specified explicitly on the nmake
#                       command line):
#
#                       --  dlgedit: invoke dialog editor on NLS DLL
#                       --  release: create/update release tree in directory
#                           specified by XWPRELEASE.  Invokes "really_all"
#                           before files are copied.
#
#       Environment:    You MUST set a number of environment variables before
#                       compiling. See PROGREF.INF.
#
#       Output:         All XWorkplace Files code files. This calls the other
#                       makefiles in the subdirectories.
#
#                       Output files are first created in bin\ (which is
#                       created if it doesn't exist), then copied to
#                       $(XWPRUNNING), which must be defined externally or
#                       thru "setup.in".
#
#       Edit "setup.in" to set up the make process (compilation flags etc.).
#

# Say hello to yourself.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

# include setup (compiler options etc.)
!include config.in
!include make\setup.in

# VARIABLES
# ---------

OUTPUTDIR = $(XWP_OUTPUT_ROOT)\dll_mt

# objects.in defines the .OBJ files to be compiled.
!include make\objects.in

# The OBJS macro contains all the .OBJ files which have been
# created from the files in MAIN\. The definitions below are
# from the objects.in include file.
OBJS = \
$(XFLDR_OBJS_CLASSES) \
$(XFLDR_OBJS_SHARED) \
$(XFLDR_OBJS_CONFIG) \
$(XFLDR_OBJS_FILESYS) \
$(XFLDR_OBJS_MEDIA) \
$(XFLDR_OBJS_STARTSHUT) \
$(XFLDR_OBJS_XCENTER)

# The HLPOBJS macro contains all the .OBJ files which have been
# created from the files in HELPERS\. You probably won't have to change this.
HLPOBJS = $(XWP_OUTPUT_ROOT)\helpers.lib

# The following macros contains the .OBJ files for the XCenter plugins.
DISKFREEOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\w_diskfree.obj $(PMPRINTF_LIB)
WINLISTOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\w_winlist.obj $(PMPRINTF_LIB)
IPMONOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\w_ipmon.obj $(PMPRINTF_LIB) libs\tcp32dll.lib libs\so32dll.lib
MONITOROBJS = $(XWP_OUTPUT_ROOT)\dll_subs\w_monitors.obj $(PMPRINTF_LIB)
SENTINELOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\w_sentinel.obj $(PMPRINTF_LIB) libs\win32k.lib
HEALTHOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\xwHealth.obj $(PMPRINTF_LIB)
SAMPLEOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\____sample.obj $(PMPRINTF_LIB)

D_CDFSOBJS = $(XWP_OUTPUT_ROOT)\dll_subs\d_cdfs.obj $(PMPRINTF_LIB)

# objects for XDEBUG.DLL (debugging only)
DEBUG_OBJS = $(XWP_OUTPUT_ROOT)\xdebug.obj $(XWP_OUTPUT_ROOT)\xdebug_folder.obj

# Define the suffixes for files which NMAKE will work on.
# .SUFFIXES is a reserved NMAKE keyword ("pseudotarget") for
# defining file extensions that NMAKE will recognize.
.SUFFIXES: .obj .dll .exe .h .rc .res

# The LIBS macro contains all the .LIB files, either from the compiler or
# others, which are needed for this project:
#   pmprintf    is for debugging
# The other OS/2 libraries are used by default.
LIBS = $(PMPRINTF_LIB)

# some variable strings to pass to sub-nmakes
SUBMAKE_PASS_STRING = "PROJECT_BASE_DIR=$(PROJECT_BASE_DIR)" "PROJECT_INCLUDE=$(PROJECT_INCLUDE)"

# store current directory so we can change back later
CURRENT_DIR = $(MAKEDIR)

# setup running directories
!if [@md $(XWPRUNNING) 2> NUL]
!endif
!if [@md $(XWPRUNNING)\bin 2> NUL]
!endif
!if [@md $(XWPRUNNING)\plugins 2> NUL]
!endif
!if [@md $(XWPRUNNING)\plugins\xcenter 2> NUL]
!endif
!if [@md $(XWPRUNNING)\plugins\drvdlgs 2> NUL]
!endif
!if [@md $(XWPRUNNING)\help 2> NUL]
!endif
!if [@md $(XWPRUNNING)\install 2> NUL]
!endif


# REGULAR MAIN TARGETS
# --------------------

# "all": default, does a regular compile, but leaves
# out the not-so-important stuff such as netscdee and treesize.
# Basically, this is for updating XFLDR.DLL and the hook/daemon
# for speed.
all:            compile_all link
    @echo ----- Leaving $(MAKEDIR)
    @echo Yo, done!

# "really_all" references "all" and compiles really everything.
# This must be used for the release version.
really_all:     compile_really_all tools link nls doc
    @echo ----- Leaving $(MAKEDIR)
    @echo Yo, done!

# "full": shortcut to really_all V1.0.0 (2002-09-13) [umoeller]
full: really_all

# "dep": create dependencies.
dep:
    @echo $(MAKEDIR)\makefile [$@]: Going for src\helpers (DLL version)
    @cd $(HELPERS_BASE_DIR)\src\helpers
    @$(MAKE) -nologo dep "NOINCLUDEDEPEND=1" $(SUBMAKE_PASS_STRING)
    @cd $(CURRENT_DIR)
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir src (dep)
    @cd src
    $(MAKE) -nologo dep "SUBTARGET=dep" "RUNDEPONLY=1" "REALLYALL=1"
    @cd $(CURRENT_DIR)
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir tools (dep)
    @cd tools
    $(MAKE) -nologo dep "SUBTARGET=dep" "RUNDEPONLY=1" "REALLYALL=1"
    @cd $(CURRENT_DIR)
    @echo ----- Leaving $(MAKEDIR)
    @echo Yo, done!

# "running": update running directories
running:
    $(COPY) BUGS $(XWPRUNNING)
    $(COPY) cvs.txt $(XWPRUNNING)
    $(COPY) FEATURES $(XWPRUNNING)
!ifndef XWPLITE
    @cd $(XWP_LANG_CODE)
    $(MAKE) -nologo running "MAINMAKERUNNING=YES"
    @cd ..
#   $(COPY) $(XWP_LANG_CODE)\readme $(XWPRUNNING)
    $(COPY) release\* $(XWPRUNNING)
    $(COPY) release\bin\alwssort.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\bm-lvm.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\bootmgr.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\defdetls.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\deficon.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\deftree.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\iconorm.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\icosmall.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\newobj.cmd $(XWPRUNNING)\bin
!endif
    $(COPY) release\bin\packtree.cmd $(XWPRUNNING)\bin
!ifndef XWPLITE
    $(COPY) release\bin\showall.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\xhelp.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\xshutdwn.cmd $(XWPRUNNING)\bin
    $(COPY) release\bin\xshutdwn.ico $(XWPRUNNING)\bin
!endif
!if [@md $(XWPRUNNING)\bootlogo 2> NUL]
!endif
    $(COPY) release\bootlogo\* $(XWPRUNNING)\bootlogo
!if [@md $(XWPRUNNING)\cdplay 2> NUL]
!endif
    $(COPY) release\cdplay\* $(XWPRUNNING)\cdplay
#!if [@md $(XWPRUNNING)\icons 2> NUL]
#!endif
#    $(COPY) release\icons\* $(XWPRUNNING)\icons
    $(COPY) release\install\deinst.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\delobjs.cmd $(XWPRELEASE_MAIN)\install
!ifdef XWPLITE
    $(COPY) release\install\freshini_lite.cmd $(XWPRELEASE_MAIN)\install\freshini.cmd
!else
    $(COPY) release\install\freshini.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\od.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\soundoff.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\xwp.ico $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\xwp_o.ico $(XWPRELEASE_MAIN)\install
!endif
!if [@md $(XWPRUNNING)\themes 2> NUL]
!endif
!if [@md $(XWPRUNNING)\themes\warp3 2> NUL]
!endif
    $(COPY) release\themes\warp3\* $(XWPRUNNING)\themes\warp3
!if [@md $(XWPRUNNING)\themes\warp4 2> NUL]
!endif
    $(COPY) release\themes\warp4\* $(XWPRUNNING)\themes\warp4
!if [@md $(XWPRUNNING)\themes\xwp 2> NUL]
!endif
    $(COPY) release\themes\xwp\* $(XWPRUNNING)\themes\xwp
!if [@md $(XWPRUNNING)\toolkit 2> NUL]
!endif
    $(COPY) src\widgets\miniwdgt.c $(XWPRUNNING)\toolkit
!if [@md $(XWPRUNNING)\toolkit\shared 2> NUL]
!endif
    $(COPY) include\shared\center.h $(XWPRUNNING)\toolkit\shared
    $(COPY) include\shared\helppanels.h $(XWPRUNNING)\toolkit\shared\helpids.h
!ifndef XWPLITE
!if [@md $(XWPRUNNING)\toolkit\config 2> NUL]
!endif
    $(COPY) include\config\drivdlgs.h $(XWPRUNNING)\toolkit\config
!endif
!if [@md $(XWPRUNNING)\wav 2> NUL]
!endif
    $(COPY) release\wav\* $(XWPRUNNING)\wav

# "COMPILE" PSEUDOTARGETS
# -----------------------

# The following are invoked via "all" or "really_all"
# to call src\makefile with SUBTARGET=all, which will
# in turn invoke nmake all to the src\ subdirectories.

# idl always gets invoked first to check whether
# src\classes\*.c needs to be refreshed from idl\*.idl.
idl:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir idl
    @cd idl
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES"
    @cd ..

helpers:
# helpers:
# this branches over to the xwphelpers source tree,
# which is prepared for this. The helpers.lib file
# is created in $(XWP_OUTPUT_ROOT) then.
    @echo $(MAKEDIR)\makefile [$@]: Going for src\helpers (DLL version)
    @cd $(HELPERS_BASE_DIR)\src\helpers
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING) \
"OUTPUTDIR_HELPERS=$(XWP_OUTPUT_ROOT)" "CC_HELPERS=$(CC_HELPERS_DLL)"
# according to VAC++ user guide, we need to use /ge+ for libs
# even if the lib will be linked to a DLL
    @cd $(CURRENT_DIR)

helpers_exe_mt:
# helpers_exe_mt:
# same as the above, but this builds a multithread lib for EXEs
# in $(XWP_OUTPUT_ROOT)\exe_mt\ instead.
    @echo $(MAKEDIR)\makefile [$@]: Going for src\helpers (EXE MT version)
    @cd $(HELPERS_BASE_DIR)\src\helpers
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING) \
"OUTPUTDIR_HELPERS=$(XWP_OUTPUT_ROOT)\exe_mt" "CC_HELPERS=$(CC_HELPERS_EXE_MT)"
    @cd $(CURRENT_DIR)

# compile_all: compile main (without treesize etc.)
compile_all: idl helpers helpers_exe_mt
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir src
    @cd src
    $(MAKE) -nologo "SUBTARGET=all"
    @cd $(CURRENT_DIR)

# compile_really_all: compile really_all
compile_really_all: idl helpers helpers_exe_mt
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir src (REALLY_ALL)
    @cd src
    $(MAKE) -nologo "SUBTARGET=all" "REALLYALL=1"
    @cd $(CURRENT_DIR)

tools:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir tools
    @cd tools
    $(MAKE) -nologo "SUBTARGET=all" "MAINMAKERUNNING=YES"
    $(COPY) $(MODULESDIR)\repclass.exe $(XWPRUNNING)\bin
    $(COPY) $(MODULESDIR)\wpsreset.exe $(XWPRUNNING)\bin
    @cd ..

doc:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir doc
    @cd doc
    $(MAKE) -nologo "SUBTARGET=all" "MAINMAKERUNNING=YES"
    @cd ..

xwpsecurity:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir src\xwpsec_ring0
    @cd src\xwpsec_ring0
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..\..
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir src\XWPShell
    @cd src\XWPShell
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..\..

$(XWP_LANG_CODE):
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(XWP_LANG_CODE)
    @cd $(XWP_LANG_CODE)
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES" $(SUBMAKE_PASS_STRING)
    @cd ..

nls: $(XWP_LANG_CODE)

# LINKER PSEUDOTARGETS
# --------------------

link: $(XWPRUNNING)\bin\xfldr.dll \
      $(XWPRUNNING)\bin\xwpres.dll \
      $(XWPRUNNING)\plugins\xcenter\diskfree.dll \
      $(XWPRUNNING)\plugins\xcenter\monitors.dll \
      $(XWPRUNNING)\plugins\xcenter\winlist.dll \
      $(XWPRUNNING)\plugins\xcenter\sentinel.dll \
      $(XWPRUNNING)\plugins\xcenter\ipmon.dll \
#      $(XWPRUNNING)\plugins\xcenter\xwHealth.dll \ disabled V1.0.1 (2003-02-01) [umoeller]
      $(XWPRUNNING)\plugins\xcenter\sample.dll \
      $(XWPRUNNING)\plugins\drvdlgs\d_cdfs.dll \
#      $(XWPRUNNING)\bin\xwphook.dll \
#      $(XWPRUNNING)\bin\xwpdaemn.exe
#      $(XWPRUNNING)\bin\xwpfonts.fon
#      $(XWPRUNNING)\bin\xdebug.dll

#
# generic inference rules for copying and stuff
#

{$(MODULESDIR)}.dll{$(XWPRUNNING)\plugins\xcenter}.dll:
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

{$(MODULESDIR)}.dll{$(XWPRUNNING)\plugins\drvdlgs}.dll:
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\drvdlgs
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\drvdlgs

# Finally, define rules for linking the target DLLs and EXEs
# This uses the $OBJS and $HLPOBJS macros defined at the top.
#
# The actual targets are the DLLs and EXEs in the XWorkplace
# installation directory. We create the target in bin\ first
# and then copy it thereto.

#
# Linking XFLDR.DLL
#
$(XWPRUNNING)\bin\xfldr.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\bin
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\bin
!ifdef DYNAMIC_TRACE
        @echo $(MAKEDIR)\makefile [$@]: Creating TRACE files for $(@B).dll
        maptsf $(@B).map /MAJOR=255 /LOGSTACK=32 /LOGRETURN > $(@B).tsf
        trcust $(@B).tsf /I /L=bin\$(@B).dll /node /M=$(@B).map
        @echo $(MAKEDIR)\makefile [$@]: Done creating TRACE files for $(@B).dll
        $(COPY) $(@B).tdf $(XWPRUNNING)\bin
        del $(@B).tdf
        $(COPY) TRC00FF.TFF $(DYNAMIC_TRACE):\OS2\SYSTEM\TRACE
        del TRC00FF.TFF
!endif

MODDEFFILE = \
!ifdef XWPLITE
src\shared\xwp_lite.def
!else
src\shared\xwp.def
!endif

# update DEF file
!if [@$(RUN_BLDLEVEL) src\shared\xwp.def include\bldlevel.h "$(XWPNAME) main module"]
!endif

$(MODULESDIR)\xfldr.dll: $(OBJS) $(HLPOBJS) $(MODDEFFILE) make\objects.in
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) @<<$(TEMP)\XFLDR.LNK
/OUT:$@ $(MODDEFFILE) $(OBJS) $(HLPOBJS) $(LIBS)
<<
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)
        tools\raisebld.cmd include\build.h

#
# Linking XWPRES.DLL
#

$(XWPRUNNING)\bin\xwpres.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\bin

# update DEF file
!if [@$(RUN_BLDLEVEL) src\shared\xwpres.def include\bldlevel.h "$(XWPNAME) resources module"]
!endif

$(MODULESDIR)\xwpres.dll: src\shared\xwpres.def $(OUTPUTDIR)\xwpres.res $(OUTPUTDIR)\dummyfont.obj
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK_ALWAYSPACK) /OUT:$@ src\shared\xwpres.def $(OUTPUTDIR)\dummyfont.obj
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RC) $(OUTPUTDIR)\xwpres.res $(@B).dll
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

# XCENTER PLUGINS LINKER PSEUDOTARGETS
# ------------------------------------

#
# Linking WINLIST.DLL
#

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\w_winlist.def include\bldlevel.h "$(XWPNAME) window-list plugin DLL"]
!endif

$(MODULESDIR)\winlist.dll: $(WINLISTOBJS) src\widgets\w_$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(WINLISTOBJS) src\widgets\w_$(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking DISKFREE.DLL
#
$(XWPRUNNING)\plugins\xcenter\diskfree.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\w_diskfree.def include\bldlevel.h "$(XWPNAME) diskfree plugin DLL"]
!endif

$(MODULESDIR)\diskfree.dll: $(DISKFREEOBJS) src\widgets\w_diskfree.def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(DISKFREEOBJS) src\widgets\w_diskfree.def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking ipmon.DLL
#
$(XWPRUNNING)\plugins\xcenter\ipmon.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\w_ipmon.def include\bldlevel.h "$(XWPNAME) IP monitor plugin DLL"]
!endif

$(MODULESDIR)\ipmon.dll: $(IPMONOBJS) src\widgets\w_$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(IPMONOBJS) src\widgets\w_$(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking MONITORS.DLL
#
$(XWPRUNNING)\plugins\xcenter\monitors.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\w_monitors.def include\bldlevel.h "$(XWPNAME) monitors plugin DLL"]
!endif

$(MODULESDIR)\monitors.dll: $(MONITOROBJS) src\widgets\w_$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(MONITOROBJS) src\widgets\w_$(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking sentinel.DLL
#
$(XWPRUNNING)\plugins\xcenter\sentinel.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\w_sentinel.def include\bldlevel.h "$(XWPNAME) memory sentinel plugin DLL"]
!endif

$(MODULESDIR)\sentinel.dll: $(SENTINELOBJS) src\widgets\w_$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(SENTINELOBJS) src\widgets\w_$(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking xwHealth.DLL
#
$(XWPRUNNING)\plugins\xcenter\xwHealth.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\xwHealth.def include\bldlevel.h "$(XWPNAME) xwHealth plugin DLL"]
!endif

$(MODULESDIR)\xwHealth.dll: $(HEALTHOBJS) src\widgets\$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ src\widgets\$(@B).def @<<
$(HEALTHOBJS)
<<
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking SAMPLE.DLL
#
$(XWPRUNNING)\plugins\xcenter\sample.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\plugins\xcenter
        $(COPY) $(MODULESDIR)\$(@B).sym $(XWPRUNNING)\plugins\xcenter

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\sample.def include\bldlevel.h "$(XWPNAME) sample plugin DLL"]
!endif

$(MODULESDIR)\sample.dll: $(SAMPLEOBJS) src\widgets\$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ src\widgets\$(@B).def @<<
$(SAMPLEOBJS)
<<
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

# DRIVER PLUGINS LINKER PSEUDOTARGETS
# -----------------------------------
#
# Linking D_CDFS.DLL
#

# update DEF file
!if [@$(RUN_BLDLEVEL) src\widgets\d_cdfs.def include\bldlevel.h "$(XWPNAME) CDFS.IFS driver plugin DLL"]
!endif

$(MODULESDIR)\d_cdfs.dll: $(D_CDFSOBJS) src\widgets\$(@B).def
        @echo $(MAKEDIR)\makefile [$@]: Linking $@
        $(LINK) /OUT:$@ $(D_CDFSOBJS) src\widgets\$(@B).def
!ifdef XWP_OUTPUT_ROOT_DRIVE
        @$(XWP_OUTPUT_ROOT_DRIVE)
!endif
        @cd $(MODULESDIR)
        $(RUN_MAPSYM) $(@B).map
!ifdef CVS_WORK_ROOT_DRIVE
        @$(CVS_WORK_ROOT_DRIVE)
!endif
        @cd $(CURRENT_DIR)

#
# Linking XWPFONTS.FON
#
$(XWPRUNNING)\bin\xwpfonts.fon: $(MODULESDIR)\$(@B).fon
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).fon $(XWPRUNNING)\bin

# update DEF file
!if [@$(RUN_BLDLEVEL) src\shared\xwpfonts.def include\bldlevel.h "$(XWPNAME) bitmap fonts"]
!endif

$(MODULESDIR)\xwpfonts.fon: $(XWP_OUTPUT_ROOT)\$(@B).res
        @echo $(MAKEDIR)\makefile [$@]: Linking $(MODULESDIR)\$(@B).fon
        $(LINK) /OUT:$(MODULESDIR)\$(@B).dll src\shared\$(@B).def
        @cd $(MODULESDIR)
# rename manually because otherwise the linker warns
#        del $(@B).fon
        ren $(@B).dll $(@B).fon
        $(RC) ..\$(@B).res $(@B).fon
        @cd $(CURRENT_DIR)

#
# XDEBUG.DLL
#
$(XWPRUNNING)\bin\xdebug.dll: $(MODULESDIR)\$(@B).dll
!ifdef XWP_UNLOCK_MODULES
        $(RUN_UNLOCK) $@
!endif
        $(COPY) $(MODULESDIR)\$(@B).dll $(XWPRUNNING)\bin

$(MODULESDIR)\xdebug.dll: src\shared\$(@B).def $(DEBUG_OBJS) $(HLPOBJS) $(XWP_OUTPUT_ROOT)\wpsh.obj
        $(LINK) /OUT:$(MODULESDIR)\$(@B).dll src\shared\$(@B).def $(DEBUG_OBJS) $(HLPOBJS) $(LIBS) $(XWP_OUTPUT_ROOT)\wpsh.obj

#
# Special target "dlgedit": this is not called by "all",
# but must be set on the NMAKE command line.
# Note that you need DLGEDIT.EXE from the Toolkit for this.
#

dlgedit:
# added (UM 99-10-24)
    @echo $(MAKEDIR)\makefile [$@]: Calling DLGEDIT.EXE
    @cd $(XWP_LANG_CODE)\dll
# rebuild RES file in bin
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES"
# copy RES file to 001\dll so dlgedit finds it
    @$(COPY) $(PROJECT_OUTPUT_DIR)\xfldr$(XWP_LANG_CODE).res
    @$(COPY) ..\..\include\dlgids.h
# invoke DLGEDIT
    dlgedit xfldr$(XWP_LANG_CODE).res
# move newly created RES file back to \bin
    @$(COPY) xfldr$(XWP_LANG_CODE).res ..\bin
    @del xfldr$(XWP_LANG_CODE).res
    @del dlgids.h
    @$(MAKE) -nologo all "MAINMAKERUNNING=YES"
    @cd ..\..

#
# Special target "release": this is not called by "all",
# but must be set on the NMAKE command line.
#

release: really_all
# 1) main dir
!ifndef XWPRELEASE
!error XWPRELEASE must be set before calling "make release". Terminating.
!endif
# create directories
!if [@md $(XWPRELEASE) 2> NUL]
!endif
!if [@md $(XWPRELEASE)\$(BLD_TYP) 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAIN) 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLS) 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLSDOC) 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAP) 2> NUL]
!endif
    @echo $(MAKEDIR)\makefile [$@]: Now copying files to $(XWPRELEASE)\$(BLD_TYP)
    @cd $(XWP_LANG_CODE)
    $(MAKE) -nologo release "MAINMAKERUNNING=YES"
    @cd ..
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).inf $(XWPRELEASE_NLSDOC)
!ifndef XWPLITE
    $(COPY) release\COPYING $(XWPRELEASE_MAIN)
    $(COPY) release\file_id.diz $(XWPRELEASE_MAIN)
#   $(COPY) $(XWP_LANG_CODE)\readme $(XWPRELEASE_NLSDOC)
    $(COPY) BUGS $(XWPRELEASE_NLSDOC)
    $(COPY) FEATURES $(XWPRELEASE_NLSDOC)
    $(COPY) cvs.txt $(XWPRELEASE_MAIN)
!endif
#
# 2) bin
#    a) kernel
!if [@md $(XWPRELEASE_MAIN)\bin 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLS)\bin 2> NUL]
!endif
!ifndef XWPLITE
    $(COPY) release\bin\alwssort.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\bm-lvm.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\bootmgr.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\defdetls.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\deficon.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\deftree.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\iconorm.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\icosmall.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\newobj.cmd $(XWPRELEASE_MAIN)\bin
!endif
    $(COPY) release\bin\packtree.cmd $(XWPRELEASE_MAIN)\bin
!ifndef XWPLITE
    $(COPY) release\bin\showall.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\xhelp.cmd $(XWPRELEASE_MAIN)\bin
    $(COPY) release\bin\xshutdwn.cmd $(XWPRELEASE_MAIN)\bin
#fixed wrong target path for xshutdwn.ico V1.0.0 (2002-08-18) [umoeller]
    $(COPY) release\bin\xshutdwn.ico $(XWPRELEASE_MAIN)\bin
!endif
#    $(COPY) release\bin\icons.dll
!ifndef XWPLITE
#    $(COPY) release\bin\files.txt
!endif
    $(COPY) $(MODULESDIR)\xfldr.dll $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xfldr.sym $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xwpres.dll $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xwphook.dll $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xwphook.sym $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\netscdde.exe $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\treesize.exe $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xfix.exe $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xfix.sym $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xwpdaemn.exe $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\xwpdaemn.sym $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\repclass.exe $(XWPRELEASE_MAIN)\bin
    $(COPY) $(MODULESDIR)\wpsreset.exe $(XWPRELEASE_MAIN)\bin
#    b) NLS
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).dll $(XWPRELEASE_NLS)\bin
#    b) mapfiles
    $(COPY) $(MODULESDIR)\*.map $(XWPRELEASE_MAP)
#
# 3) bootlogo
!ifndef XWPLITE
!if [@md $(XWPRELEASE_MAIN)\bootlogo 2> NUL]
!endif
    $(COPY) release\bootlogo\* $(XWPRELEASE_MAIN)\bootlogo
!endif
#
# 4) help
!if [@md $(XWPRELEASE_NLS)\help 2> NUL]
!endif
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).tmf $(XWPRELEASE_NLS)\help
    $(COPY) $(XWP_LANG_CODE)\misc\drvrs$(XWP_LANG_CODE).txt $(XWPRELEASE_NLS)\help
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\xfcls$(XWP_LANG_CODE).txt $(XWPRELEASE_NLS)\help
!endif
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).hlp $(XWPRELEASE_NLS)\help
!ifndef XWPLITE
# 5) themes
#!if [@md $(XWPRELEASE_MAIN)\icons 2> NUL]
#!endif
#    $(COPY) release\icons\* $(XWPRELEASE_MAIN)\icons
#!endif
!if [@md $(XWPRELEASE_MAIN)\themes 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAIN)\themes\warp3 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAIN)\themes\warp4 2> NUL]
!endif
    $(COPY) release\themes\warp3\* $(XWPRELEASE_MAIN)\themes\warp3
    $(COPY) release\themes\warp4\* $(XWPRELEASE_MAIN)\themes\warp4
!endif
# 6) install
!if [@md $(XWPRELEASE_MAIN)\install 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLS)\install 2> NUL]
!endif
#    $(COPY) release\install\xwpusers.acc $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\deinst.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\delobjs.cmd $(XWPRELEASE_MAIN)\install
!ifdef XWPLITE
    $(COPY) release\install\freshini_lite.cmd $(XWPRELEASE_MAIN)\install\freshini.cmd
!else
    $(COPY) release\install\freshini.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\od.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\soundoff.cmd $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\xwp.ico $(XWPRELEASE_MAIN)\install
    $(COPY) release\install\xwp_o.ico $(XWPRELEASE_MAIN)\install
!endif
#    $(COPY) release\install\xwpusers.xml $(XWPRELEASE_MAIN)\install
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\crobj$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install\crobj$(XWP_LANG_CODE).cmd
!else
    $(COPY) $(XWP_LANG_CODE)\misc\crobj$(XWP_LANG_CODE)_lite.cmd $(XWPRELEASE_NLS)\install\crobj$(XWP_LANG_CODE).cmd
!endif
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\instl$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install\instl$(XWP_LANG_CODE).cmd
!else
    $(COPY) $(XWP_LANG_CODE)\misc\instl$(XWP_LANG_CODE)_lite.cmd $(XWPRELEASE_NLS)\install\instl$(XWP_LANG_CODE).cmd
!endif
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\sound$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install
!endif
# 7) wav
!ifndef XWPLITE
!if [@md $(XWPRELEASE_MAIN)\wav 2> NUL]
!endif
    $(COPY) release\wav\* $(XWPRELEASE_MAIN)\wav
!endif
    @echo $(MAKEDIR)\makefile [$@]: Done copying files.
# 8) plugins
!if [@md $(XWPRELEASE_MAIN)\plugins 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAIN)\plugins\xcenter 2> NUL]
!endif
!ifndef XWPLITE
!if [@md $(XWPRELEASE_MAIN)\plugins\drvdlgs 2> NUL]
!endif
!endif
!ifndef XWPLITE
!endif
    $(COPY) $(MODULESDIR)\diskfree.dll $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\diskfree.sym $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\monitors.dll $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\monitors.sym $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\winlist.dll $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\winlist.sym $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\sentinel.dll $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\sentinel.sym $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\ipmon.dll $(XWPRELEASE_MAIN)\plugins\xcenter
    $(COPY) $(MODULESDIR)\ipmon.sym $(XWPRELEASE_MAIN)\plugins\xcenter
!ifndef XWPLITE
    $(COPY) $(MODULESDIR)\d_cdfs.dll $(XWPRELEASE_MAIN)\plugins\drvdlgs
    $(COPY) $(MODULESDIR)\d_cdfs.sym $(XWPRELEASE_MAIN)\plugins\drvdlgs
!endif
# 9) toolkit
!if [@md $(XWPRELEASE_MAIN)\toolkit 2> NUL]
!endif
!if [@md $(XWPRELEASE_MAIN)\toolkit\shared 2> NUL]
!endif
!ifndef XWPLITE
!if [@md $(XWPRELEASE_MAIN)\toolkit\config 2> NUL]
!endif
!endif
    $(COPY) src\widgets\miniwdgt.c $(XWPRELEASE_MAIN)\toolkit
    $(COPY) include\shared\center.h $(XWPRELEASE_MAIN)\toolkit\shared
    $(COPY) include\shared\helppanels.h $(XWPRELEASE_MAIN)\toolkit\shared\helpids.h
!ifndef XWPLITE
    $(COPY) include\config\drivdlgs.h $(XWPRELEASE_MAIN)\toolkit\config
!endif
    @echo $(MAKEDIR)\makefile [$@]: Done copying files.
# now go shrink executables V0.9.13 (2001-06-17) [umoeller]
!ifndef XWP_DEBUG
!ifdef LXLITEPATH
    $(LXLITEPATH)\lxlite \
$(XWPRELEASE_MAIN)\bin\*.dll \
$(XWPRELEASE_MAIN)\bin\*.exe \
$(XWPRELEASE_NLS)\bin\*.dll
    $(LXLITEPATH)\lxlite \
$(XWPRELEASE_MAIN)\plugins\xcenter\*.dll \
!endif
!endif

#
# Special target "release_nls": this is not called by "all",
# but must the set on the NMAKE command line.
#

release_nls: nls
!if [@md $(XWPRELEASE) 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLS) 2> NUL]
!endif
!if [@md $(XWPRELEASE_NLSDOC) 2> NUL]
!endif
    @echo $(MAKEDIR)\makefile [$@]: Now copying files to $(XWPRELEASE)
    @cd $(XWP_LANG_CODE)
    $(MAKE) -nologo release_nls "MAINMAKERUNNING=YES"
    @cd ..
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).inf $(XWPRELEASE_NLSDOC)
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).dll $(XWPRELEASE_NLS)\bin
!if [@md $(XWPRELEASE_NLS)\help 2> NUL]
!endif
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).tmf $(XWPRELEASE_NLS)\help
    $(COPY) $(XWP_LANG_CODE)\misc\drvrs$(XWP_LANG_CODE).txt $(XWPRELEASE_NLS)\help
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\xfcls$(XWP_LANG_CODE).txt $(XWPRELEASE_NLS)\help
!endif
    $(COPY) $(MODULESDIR)\xfldr$(XWP_LANG_CODE).hlp $(XWPRELEASE_NLS)\help
!if [@md $(XWPRELEASE_NLS)\install 2> NUL]
!endif
!ifndef XWPLITE
    $(COPY) $(XWP_LANG_CODE)\misc\crobj$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install\crobj$(XWP_LANG_CODE).cmd
    $(COPY) $(XWP_LANG_CODE)\misc\instl$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install\instl$(XWP_LANG_CODE).cmd
    $(COPY) $(XWP_LANG_CODE)\misc\sound$(XWP_LANG_CODE).cmd $(XWPRELEASE_NLS)\install
!else
    $(COPY) $(XWP_LANG_CODE)\misc\crobj$(XWP_LANG_CODE)_lite.cmd $(XWPRELEASE_NLS)\install\crobj$(XWP_LANG_CODE).cmd
    $(COPY) $(XWP_LANG_CODE)\misc\instl$(XWP_LANG_CODE)_lite.cmd $(XWPRELEASE_NLS)\install\instl$(XWP_LANG_CODE).cmd
!endif
    @echo $(MAKEDIR)\makefile [$@]: Done copying files.
!ifdef LXLITEPATH
    @echo $(MAKEDIR)\makefile [$@]: Now compacting files.
    $(LXLITEPATH)\lxlite \
$(XWPRELEASE_NLS)\bin\*.dll
    @echo $(MAKEDIR)\makefile [$@]: Done compacting files.
!endif

#
# Special target "wpi": this is not called by "all",
# but must be set on the NMAKE command line.
#

wpi: release
    @echo $(MAKEDIR)\makefile [$@]: Building WPI from $(XWPRELEASE)\$(BLD_TYP)
    makewpi.cmd $(XWPRELEASE)\$(BLD_TYP) $(XWP_LANG_CODE)

#
# Special target "wpi_nls": this is not called by "all",
# but must be set on the NMAKE command line.
#

wpi_nls: release_nls
    @echo $(MAKEDIR)\makefile [$@]: Building NLS WPI from $(XWPRELEASE)\$(BLD_TYP)
    @cd $(XWP_LANG_CODE)
    makewpi$(XWP_LANG_CODE).cmd $(XWPRELEASE)\$(BLD_TYP)
    @cd $(CURRENT_DIR)

clean:
    cleanup.cmd y $(BLD_TYP) $(XWP_LANG_CODE)
