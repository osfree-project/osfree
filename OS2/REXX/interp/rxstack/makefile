#
#
#
#


PROJ        = rxstack
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = quiet, stack=0x10000000, heapsize=40960
srcfiles    = $(p)rxstack$(e) $(p)eextstack$(e) $(p)erexxbif$(e)
ADD_COPT    = $(THREADING) $(HAVE_GCI) $(DYN_CFLAGS) #-bd
ADD_LINKOPT = lib rexx.lib,tcpip32.lib
# ALIASES     = main_=main

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)rxstack.obj:  $(SRC)rxqueue.c $(HFILES)
 $(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@
