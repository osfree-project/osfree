#
#
#
#


PROJ        = pmregina
DESC        = PM Regina REXX interface
DEST        = os2
OPTIONS     = caseexact
srcfiles    = $(p)regina$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI)
ADD_LINKOPT = lib $(PATH)..$(SEP)implib$(SEP)regina.lib
RESOURCE    = $(PATH)reginaos2.res

!define WINDOWCOMPAT

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)regina.obj: $(SRC)regina.c $(HFILES)
