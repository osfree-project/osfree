#
# make macros for building
# static libraries
#

!ifndef 32_BITS
32_BITS = 1
!endif
#ADD_COPT = -d__WATCOM__ -d__OS2__ -i=. -i=$(%ROOT)$(SEP)include

!ifndef DEST
DEST     = ..$(SEP)build$(SEP)lib
!endif

!include $(%ROOT)/mk/all.mk

# makes library $(library) from object files $(OBJS)
library: .SYMBOLIC
 $(SAY) Creating library $(library)...
 -@if exist $(library) @$(DC) $(library)
 @$(LIB) $(LIBOPT) $(library) +$(OBJS) $(LOG)
