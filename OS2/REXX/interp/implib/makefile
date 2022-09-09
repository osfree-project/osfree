#
#
#
#


PROJ        = regina
TRGT        = $(PROJ).lib
DESC        = Regina REXX import library
DEST        = ..$(SEP)..$(SEP)build$(SEP)lib
OBJS        = $(PATH)..$(SEP)dll$(SEP)$(PROJ).dll
#OPTIONS     = caseexact, quiet



#TARGETS = $(PATH)$(PROJ).lib



!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)..$(SEP)dll$(SEP)$(PROJ).dll:: $(PATH)..$(SEP)dll$(SEP)$(PROJ).lnk

.lnk: $(PATH)..$(SEP)dll

$(PATH)$(PROJ).lib: $(PATH)..$(SEP)dll$(SEP)$(PROJ).dll # $(OBJS)
 @$(SAY) LIB      $^.
 @$(LIB) $(LIBOPT) $^@ +$[@
