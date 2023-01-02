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

!ifdef LIBS
#$(LIBS) $(OBJS):: $(MYDIR)makefile
!else ifdef OBJS
#$(LIBS) $(OBJS):: $(MYDIR)makefile
!endif

TARGETS = $(PATH)$(PROJ).lib

# makes library $(library) from object files $(OBJS)
library: $(OBJS) $(LIBS) $(MYDIR)makefile .symbolic
 @$(SAY) LIB      $(PROJ).lib
!ifndef NODELETE
!ifeq UNIX TRUE
 $(verbose)$(DC) $(library)
!else
 $(verbose)@if exist $(library) $(DC) $(library) $(BLACKHOLE)
!endif
!endif
!ifdef OBJS
 $(verbose)$(LIB) $(LIBOPT) $(library).tmp1 +$(OBJS) $(LOG2)
!endif
!ifdef LIBS
 $(verbose)$(LIB) $(LIBOPT) $(library).tmp1 +$(LIBS) $(LOG2)
!endif
!ifdef OBJS16
 $(verbose)$(LIB) $(LIBOPT) $(library).tmp2 +$(OBJS16) $(LOG2)
 $(verbose)$(LIB) $(LIBOPT) $(library)      +$(library).tmp1 +$(library).tmp2 $(LOG2)
!ifeq UNIX TRUE
 $(verbose)$(DC) $(library).tmp1
 $(verbose)$(DC) $(library).tmp2
!else
 @if exist $(library).tmp1 @$(DC) $(library).tmp1 $(BLACKHOLE)
 @if exist $(library).tmp2 @$(DC) $(library).tmp2 $(BLACKHOLE)
!endif
!else
 $(verbose)$(LIB) $(LIBOPT) $(library)      +$(library).tmp1 $(LOG2)
!ifeq UNIX TRUE
 $(verbose)$(DC) $(library).tmp1
!else
 $(verbose)if exist $(library).tmp1 $(verbose)$(DC) $(library).tmp1 $(BLACKHOLE)
!endif
!endif

!endif
