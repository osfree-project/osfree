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
DEST     = ..$(SEP)build$(SEP)lib
!endif

!include $(%ROOT)/mk/all.mk

$(PATH)$(PROJ).lib:

# makes library $(library) from object files $(OBJS)
library: $(OBJS) $(LIBS)
 @$(SAY) LIB      $(PROJ).lib
!ifndef NODELETE
!ifeq UNIX TRUE
 -$(DC) $(library)
!else
 -@if exist $(library) $(DC) $(library) $(BLACKHOLE)
!endif
!endif
!ifdef OBJS
 @$(LIB) $(LIBOPT) $(library).tmp1 +$(OBJS) $(LOG)
!endif
!ifdef LIBS
 @$(LIB) $(LIBOPT) $(library).tmp1 +$(LIBS) $(LOG)
!endif
!ifdef OBJS16
 @$(LIB) $(LIBOPT) $(library).tmp2 +$(OBJS16) $(LOG)
 @$(LIB) $(LIBOPT) $(library)      +$(library).tmp1 +$(library).tmp2
!ifeq UNIX TRUE
 @$(DC) $(library).tmp1
 @$(DC) $(library).tmp2
!else
 @if exist $(library).tmp1 @$(DC) $(library).tmp1 $(BLACKHOLE)
 @if exist $(library).tmp2 @$(DC) $(library).tmp2 $(BLACKHOLE)
!endif
!else
 $(verbose)$(LIB) $(LIBOPT) $(library)      +$(library).tmp1
!ifeq UNIX TRUE
 @$(DC) $(library).tmp1
!else
 @if exist $(library).tmp1 @$(DC) $(library).tmp1 $(BLACKHOLE)
!endif
!endif

!endif
