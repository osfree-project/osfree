#
#
#
#


PROJ        = threader
TRGT        = $(PROJ).exe
DESC        = Regina REXX interpreter
DEST        = os2
OPTIONS     = quiet,stack=0x10000000,heapsize=40960
srcfiles    = $(p)threader$(e)
ADD_COPT    = -dOS2_THREADS $(THREADING) $(HAVE_GCI)
ADD_LINKOPT = lib $(PATH)..$(SEP)implib$(SEP)regina.lib,tcpip32.lib

!include $(%ROOT)mk/dirs.mk
!include $(MYDIR)..$(SEP)regina.mk

$(PATH)threader.obj:: $(SRC)threader.c $(HFILES)
 