#
#
#
#


PROJ        = regina
TRGT        = $(PROJ).exe
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = caseexact
srcfiles    = $(p)regina$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI) $(DYN_CFLAGS) # -bd
ADD_LINKOPT = lib $(BLD)lib$(SEP)$(PROJ).lib


!include $(%ROOT)tools/mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk


$(PATH)regina.obj: $(SRC)regina.c $(HFILES)
