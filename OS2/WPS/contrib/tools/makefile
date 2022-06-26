#
# Makefile for the tools\ directory.
#       NOTE: "tools" only gets recursed to by the main makefile
#       in "really_all" mode.
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
repclass \
wpsreset \
ximgview \
xlvm \
xmakeini \
xmlview \
xpstat

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
!endif

PARENTDIRNAME = tools

repclass:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

wpsreset:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

ximgview:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xlvm:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xmakeini:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xmlview:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xpstat:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

