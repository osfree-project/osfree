#
#
#
#


PROJ        = regina
DESC        = Regina REXX import library
DEST        =
OBJS        = $(PATH)..$(SEP)dll$(SEP)$(PROJ).dll
OPTIONS     = caseexact, quiet

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

TARGETS = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 $(LIB) $(LIBOPT) $^@ +$[@

qwe: .symbolic
 @echo $(OBJS)
