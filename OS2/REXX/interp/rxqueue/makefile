#
#
#
#


PROJ        = rxqueue
TRGT        = $(PROJ).exe
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = quiet, stack=0x10000000, heapsize=40960
srcfiles    = $(p)rxqueue$(e) $(p)erexxbif$(e) $(p)mygetopt$(e) $(p)eextstack$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI) # $(DYN_CFLAGS) #-bd
ADD_LINKOPT = lib $(BLD)lib$(SEP)rexx.lib,tcpip32.lib
# ALIASES     = main_=main

!include $(%ROOT)tools/mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)rxqueue.obj: $(SRC)rxqueue.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $(SRC)rxqueue.c

#$(PATH)rexxbif.obj: $(SRC)rexxbif.c $(HFILES)
# @$(SAY) CC       $^. $(LOG)
# @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $(SRC)rexxbif.c
