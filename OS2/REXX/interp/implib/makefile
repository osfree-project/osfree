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


!include $(%ROOT)tools/mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

TARGETS = $(PATH)$(PROJ).lib

$(PATH)..$(SEP)dll$(SEP)$(PROJ).dll: $(PATH)..$(SEP)dll$(SEP)$(PROJ).lnk

.lnk: $(PATH)..$(SEP)dll

$(PATH)$(PROJ).lib: $(PATH)..$(SEP)dll$(SEP)$(PROJ).dll # $(OBJS)
 @$(SAY) LIB      $^.
 @$(LIB) $(LIBOPT) $^@ +$[@
