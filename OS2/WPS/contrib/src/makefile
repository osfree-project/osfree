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
classes \
config \
filesys \
media \
shared \
startshut \
widgets \
xcenter \
# hook must be before daemon because we need the hook's import lib in the daemon
hook \
Daemon \
!ifdef REALLYALL
Treesize \
NetscapeDDE \
Xfix \
!ifdef XWPSECURITY
xwpsec_ring0 \
XWPShell
!endif
!endif

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

PARENTDIRNAME = src

classes:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

config:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

filesys:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

media:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

shared:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

startshut:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

widgets:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xcenter:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

Daemon:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

hook:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

# additional REALLYALL targets
Drvchk:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

Treesize:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

NetscapeDDE:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

Xfix:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

xwpsec_ring0:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)

XWPShell:
    @echo $(MAKEDIR)\makefile [$@]: Going for subdir $(PARENTDIRNAME)\$@
    @cd $@
    @$(MAKE) -nologo $(SUBTARGET) $(SUBMAKESTRING) "MAINMAKERUNNING=YES"
    @cd $(CURRENT_DIR)


