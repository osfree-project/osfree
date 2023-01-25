#
#
#
#


PROJ        = pmregina1
PROJ1       = pmregina
TRGT        = $(PROJ1).exe
DESC        = PM Regina REXX interface
DEST        = os2
OPTIONS     = caseexact
srcfiles    = $(p)regina$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI)
ADD_LINKOPT = lib $(PATH)..$(SEP)implib$(SEP)regina.lib
#RESOURCE    = $(PATH)reginaos2.res
WINDOWCOMPAT = 1
UNI2H        = 1
inc =  -i=$(MYDIR)..$(SEP)contrib

!include $(%ROOT)tools/mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

.rc: $(MYDIR)..$(SEP)contrib

.ico: $(MYDIR)..$(SEP)contrib

TARGETS = $(PATH)$(PROJ1).exe # subdirs
#TARGET = all

$(PATH)regina.obj: $(SRC)regina.c $(HFILES)

$(PATH)$(PROJ1).exe: $(PATH)$(PROJ).exe $(MYDIR)..$(SEP)contrib$(SEP)reginaos2.rc
 @$(SAY) RESCMP   $^. $(LOG)
 $(RC) -bt=os2 -i=$(%WATCOM)$(SEP)h$(SEP)os2 $(inc) $]@ $[@ -fe=$@ -fo=$(PATH)reginaos2.res
