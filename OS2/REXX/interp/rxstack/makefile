#
#
#
#


PROJ        = rxstack
TRGT        = $(PROJ).exe
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = quiet, stack=0x10000000, heapsize=40960
srcfiles    = $(p)rxstack$(e) $(p)eextstack$(e) $(p)erexxbif$(e) $(p)mygetopt$(e) $(p)LibSha1$(e)
ADD_COPT    = -i=$(MYDIR)..$(SEP)contrib $(THREADING) $(HAVE_GCI) # $(DYN_CFLAGS) #-bd
ADD_LINKOPT = lib $(BLD)lib$(SEP)rexx.lib,tcpip32.lib
# ALIASES     = main_=main

.c: $(SRC)

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)rxstack.obj:  $(SRC)rxstack.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@

$(PATH)LibSha1.obj:  $(SRC)contrib$(SEP)LibSha1.c $(SRC)contrib$(SEP)LibSha1.h
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -fr=$^*.err -fo=$^@ $[@
