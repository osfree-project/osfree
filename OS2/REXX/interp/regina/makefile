#
#
#
#


PROJ        = regina
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = caseexact, quiet
OBJS        = $(PATH)regina.obj $(PATH)..$(SEP)implib$(SEP)$(PROJ).lib
ADD_COPT    = $(THREADING) $(HAVE_GCI) $(DYN_CFLAGS) -bd
#ADD_LINKOPT = lib regina.lib

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)regina.obj: $(SRC)regina.c $(HFILES)

qwe: .symbolic
 @echo $(COPT)
