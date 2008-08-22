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
 -@if exist $(library) @$(DC) $(library)
 @$(LIB) $(LIBOPT) $(library) +$(OBJS) $(LOG)

!endif
