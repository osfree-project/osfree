#
#
#
#


PROJ        = execiser
TRGT        = $(PROJ).exe
DESC        = Regina REXX interpreter
DEST        = os2
#OPTIONS     = caseexact
srcfiles    = $(p)execiser$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI)
ADD_LINKOPT = lib $(PATH)..$(SEP)implib$(SEP)regina.lib

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)execiser.obj: $(SRC)execiser.c $(HFILES)
