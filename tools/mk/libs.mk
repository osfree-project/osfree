# tools/mk/libs.mk
#
# make macros for building
# static libraries
#

!ifndef __libs_mk__
!define __libs_mk__

!ifndef 32_BITS
32_BITS = 1
!endif

!ifndef DEST
DEST     = ..$(SEP)..$(SEP)build$(SEP)lib
!endif

!include $(%ROOT)/tools/mk/all.mk

$(PATH)$(PROJ).lib:

!ifdef ADDLIBS
COMMANDS= +$(ADDLIBS)
!endif

!ifdef OBJS
COMMANDS=$(COMMANDS) +$(OBJS)
!endif

!ifdef OBJS16
COMMANDS=$(COMMANDS) +$(OBJS16)
!endif

TARGETS = $(PATH)$(PROJ).lib

!ifndef DISABLE_LIB

$(TARGETS): $(OBJS) $(OBJS16) $(ADDLIBS) $(LIB_DEPS) $(MYDIR)makefile
	$(verbose)$(SAY) LIB      $(PROJ).lib
!ifndef NODELETE
!ifeq UNIX TRUE
	$(verbose)$(DC) $(TARGETS)
!else
	$(verbose)if exist $(TARGETS) $(DC) $(TARGETS) $(BLACKHOLE)
!endif
!endif
	$(verbose)$(LIB) $(LIBOPT) $(TARGETS) $(COMMANDS) $(LOG2)

!endif

!endif
