# tools/mk/build_proj.mk
#
# Project name detection for the osFree build system.
#
# Included when PROJ is not yet known.
#
# This file derives the project name from the name of the current
# source directory, records it in $(PATH)$(dir4)/_proj.mk so that
# future invocations do not have to re-detect it, and then re-invokes
# wmake for the same directory with MAKEOPT extended by PROJ=$(dir4).
# On that second pass build.mk falls through to build2.mk.
#
# Must be set on input:
#   MAKEOPT - opaque options string, assembled by build.mk. This file
#             appends PROJ=$(dir4) to it and passes it verbatim to
#             every recursive wmake invocation.
#

!ifndef __build_project_mk__
!define __build_project_mk__

!ifndef PROJ

!include $(%ROOT)tools/mk/dirs.mk

# Hack to get called makefile name into __MAKEFILE__
#!inject $(__MAKEFILES__) __MAKEFILE__

all install: .SYMBOLIC gen_proj_name
prepall:     .SYMBOLIC prep
depsall:     .SYMBOLIC deps

# Extract the name of the current source directory into dir4.
# dir2 captures CWD with immediate expansion, dir3 strips the absolute
# path prefix, dir4 strips the trailing path separator.
# For now dir4 is empty. Change dir to parent to set correct value of dir4. Magic!
dir2=$+ $(CWD) $-
dir3=$(dir2:$(CWD)=)
dir4=$(dir3:$(SEP)=)

# pass some targets recursively
depsall prepall subdirs clean annotate annotate-write: .SYMBOLIC
 #change dir to set correct value of dir4. Magic!
 @cd ..
 # dir4 must be userd on a single line: if the expression is split
 # across lines, dir4 ends up empty instead of holding the last path
 # component. -f $(CWD)$(SEP)$(dir4)$(SEP)$(__MAKEFILE__)
 @cd $(dir4) && $(MAKE) -f $(CWD)$(SEP)$(dir4)$(SEP)makefile $(MAKEOPT) $^@ PROJ=$(dir4)
 @%quit

#generate project name
gen_proj_name: .SYMBOLIC
 #@$(REXX) mdhier.cmd $(PATH)
 #change dir to set correct value of dir4. Magic!
 @cd ..
 #Save project name for future usage (will not start project detection from build dir)
 # dir4 must be userd on a single line: if the expression is split
 # across lines, dir4 ends up empty instead of holding the last path
 # component. -f $(CWD)$(SEP)$(dir4)$(SEP)$(__MAKEFILE__)
 @cd $(dir4) && $(MAKE) -f $(CWD)$(SEP)$(dir4)$(SEP)makefile $(MAKEOPT) PROJ=$(dir4)
 @%quit

!endif

!endif
