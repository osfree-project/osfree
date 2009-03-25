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

# makes library $(library) from object files $(OBJS)
library: $(OBJS)
 $(SAY) Creating library $(library)...
!ifndef NODELETE
 -@if exist $(library) @$(DC) $(library)
!endif
 @$(LIB) $(LIBOPT) $(library).tmp1 +$(OBJS) $(LOG)
!ifdef OBJS16
 @$(LIB) $(LIBOPT) $(library).tmp2 +$(OBJS16) $(LOG)
 @$(LIB) $(LIBOPT) $(library)      +$(library).tmp1 +$(library).tmp2
 @$(DC) $(library).tmp1
 @$(DC) $(library).tmp2
!else
 @$(LIB) $(LIBOPT) $(library)      +$(library).tmp1
 @$(DC) $(library).tmp1
!endif

!endif
