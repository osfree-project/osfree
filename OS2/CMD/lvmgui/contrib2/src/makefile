#
# Makefile for the src\ directory.
#

#
#   SUBTARGET is defined by the main makefile to be one of:
#
#       -- all: this will invoke "nmake all" on the subdirs.
#
#       -- dep: this will invoke "nmake dep" on the subdirs.
#

# Say hello to yourself.
!if [@echo +++++ Entering $(MAKEDIR)]
!endif

# store current directory so we can change back later
CURRENT_DIR = $(MAKEDIR)

SUBDIRS = \
cppbase \
helpers \
libbz2

# "all" gets called from ..\makefile when either
# "nmake all" or "nmake really_all" is running.
# In that case, SUBTARGET is "all" also so we
# run "nmake all" on the subdirs.
all: $(SUBDIRS)
    @echo ----- Leaving $(MAKEDIR)

# "dep" gets called from ..\makefile when
# "nmake dep" is running.
# In that case, SUBTARGET is "dep" also so we
# run "nmake dep" on the subdirs.
dep: $(SUBDIRS)
    @echo ----- Leaving $(MAKEDIR)

!ifdef RUNDEPONLY
SUBMAKESTRING = "NOINCLUDEDEPEND=1"
!else
SUBMAKESTRING = "OUTPUTDIR_HELPERS=$(OUTPUTDIR_HELPERS)" "CC_HELPERS=$(CC_HELPERS)"
!endif

PARENTDIRNAME = src

cppbase:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

libbz2:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

helpers:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

jpeg:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

# special targets to build only a subset of the helpers

cp:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\helpers (cp)
    @cd helpers
    @$(MAKE) -nologo cp $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)


