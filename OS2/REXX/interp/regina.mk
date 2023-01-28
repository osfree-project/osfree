#
#
#
#

VERDOT = $(VER_DOT)
VERDATE = $(VER_DATE)

# enable debug
deb      = Y
MULTI    = Y


!ifeq MULTI N
THREADING =
THREADING_LINK =
THREADING_FILE = mt_notmt
!else
THREADING = -bm -dREGINA_MULTI
THREADING_LINK = -bm
THREADING_FILE = mt_os2
!endif
ADD_COPT  += -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2

!ifeq deb Y
ADD_COPT     = -d2 $(ADD_COPT)
ADD_LINKOPT  = debug all $(ADD_LINKOPT)
ADD_LINKOPT  = debug all library tcpip32 $(ADD_LINKOPT)
# ADD_LINKOPT  = debug all $(ADD_LINKOPT)
!else
ADD_COPT     =  -dNDEBUG -oneatx $(ADD_COPT)
ADD_LINKOPT  =  $(ADD_LINKOPT)
ADD_LINKOPT  =  library tcpip32  $(ADD_LINKOPT)
# ADD_LINKOPT  =  $(ADD_LINKOPT)
!endif

UNI2H = 1
# NOLIBS = 1

# stack calling convention
C = s

!include $(%ROOT)tools/mk/appsos2_cmd.mk
!include $(MYDIR)..$(SEP)port$(SEP)regina.mk

SRC = $(PORT_BASE)

HFILES = $(SRC)rexx.h $(SRC)defs.h $(SRC)extern.h $(SRC)regina_t.h $(SRC)configur.h $(SRC)strengs.h
GCIHEADERS = $(SRC)gci$(SEP)gci.h $(SRC)gci$(SEP)embedded.h $(SRC)gci$(SEP)gci_convert.h

HAVE_GCI = -dHAVE_GCI -i=$(SRC)gci
DYN_CFLAGS = -dDYNAMIC

ADD_COPT       =  -4s -wx -wcd=202 -zq -mf $(THREADING) &
                  -sg -st -dOREXX_BINARY_COMPATIBLE -bt=os2 -fi=$(MYDIR)..$(SEP)reginaver.h &
                  -dOS2 -i=$(PATH) -i=$(SRC) -i=$(SRC).. $(ADD_COPT)

$(PATH)client.$(O): $(SRC)client.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)rexxsaa.$(O): $(SRC)rexxsaa.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)yaccsrc.$(O): $(SRC)yaccsrc.c $(SRC)defs.h $(SRC)rexx.h
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dYYMAXDEPTH=10000 -fr=$^*.err -fo=$^@ $(SRC)yaccsrc.c $(LOG)

$(PATH)drexx.obj: $(SRC)rexx.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dRXLIB -fr=$^*.err -fo=$^@ $(SRC)rexx.c

$(PATH)eextstack.obj: $(SRC)extstack.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $(SRC)extstack.c

$(PATH)erexxbif.obj: $(SRC)rexxbif.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $(SRC)rexxbif.c

.c: $(SRC)

.c: $(MYDIR)..

.c: $(SRC)gci

.h: $(SRC)

.h: $(MYDIR)..

.h: $(SRC)gci
