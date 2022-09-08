#
#
#
#

PROJ     = rexx
TRGT     = $(PROJ).exe
DESC     = Regina REXX interpreter
DEST     = os2
STUB     = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
OPTIONS  = caseexact
srcfiles = &
 $(p)mygetopt$(e) &
 $(p)rexx$(e)  &
 $(p)funcs$(e) &
 $(p)builtin$(e) &
 $(p)error$(e) &
 $(p)variable$(e) &
 $(p)interprt$(e) &
 $(p)debug$(e) &
 $(p)dbgfuncs$(e) &
 $(p)memory$(e) &
 $(p)parsing$(e) &
 $(p)files$(e) &
 $(p)misc$(e) &
 $(p)unxfuncs$(e) &
 $(p)arxfuncs$(e) &
 $(p)os2funcs$(e) &
 $(p)cmsfuncs$(e) &
 $(p)shell$(e) &
 $(p)rexxext$(e) &
 $(p)stack$(e) &
# $(p)os2que$(e) &
 $(p)mktime$(e) &
 $(p)tracing$(e) &
 $(p)interp$(e) &
 $(p)cmath$(e) &
 $(p)convert$(e) &
 $(p)strengs$(e) &
 $(p)library$(e) &
 $(p)strmath$(e) &
 $(p)signals$(e) &
 $(p)macros$(e) &
 $(p)envir$(e) &
 $(p)$(THREADING_FILE)$(e) &
 $(p)instore$(e) &
 $(p)expr$(e) &
 $(p)yaccsrc$(e) &
 $(p)lexsrc$(e) &
 $(p)wrappers$(e) &
 $(p)options$(e) &
 $(p)os_os2$(e) &
 $(p)extstack$(e) &
 $(p)rexxbif$(e) &
 $(p)nosaa$(e)


!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)$(PROJ).exe: $(OBJS)

$(OBJS):: $(HFILES)

$(PATH)drexx.obj:: $(SRC)rexx.c $(HFILES)

$(PATH)client.obj:: $(SRC)client.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)rexxsaa.obj:: $(SRC)rexxsaa.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)yaccsrc.obj:: $(SRC)yaccsrc.c $(SRC)defs.h $(SRC)rexx.h

$(PATH)lexsrc.obj:: $(SRC)lexsrc.c $(SRC)yaccsrc.h $(SRC)rexx.h
