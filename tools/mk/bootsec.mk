#
# make macros
# for building bootsectors
#

!ifndef __bootsec_mk__
!define __bootsec_mk__

32_BITS = 0

SUF = .bin

!ifndef DEST
DEST = boot$(SEP)sectors
!endif

!include $(%ROOT)/tools/mk/bootseq.mk

!ifndef TARGETS
TARGETS = $(PATH)$(PROJ).bin
!endif

!ifndef PROJ0
PROJ0 = PROJ
!endif

$(PATH)$(PROJ).bin: $(PATH)$(PROJ).lnk
 @$(SAY) LINK     $^. $(LOG)
 @$(verbose)$(LINKER) $(LINKOPT) @$[@ $(LOG2)

$(PATH)$(PROJ0).$(O): $(MYDIR)makefile

$(PATH)$(PROJ).lnk: $(PATH)$(PROJ0).$(O) $(MYDIR)makefile .always
 @%create $^@
 @%append $^@ SYSTEM dos com
 @%append $^@ OPTION MAP=$^*.wmp
 @%append $^@ OPTION NODEFAULTLIBS
 @%append $^@ NAME $^*.bin
 @%append $^@ PATH $(PATH)
 @%append $^@ FILE $(PATH)$(PROJ0).$(O)

!endif
