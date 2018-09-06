#
#
#
#

SRC = $(%ROOT)os2$(SEP)REXX$(SEP)interp$(SEP)

!include $(SRC)regina.ver

VERDOT = $(VER_DOT)
VERDATE = $(VER_DATE)

# enable debug
deb      = Y
MULTI    = Y

DYN_CFLAGS = -dDYNAMIC

!ifeq MULTI N
THREADING =
THREADING_LINK =
THREADING_FILE = mt_notmt
!else
THREADING = -bm -dREGINA_MULTI
THREADING_LINK = -bm
THREADING_FILE = mt_os2
!endif

HFILES = $(SRC)rexx.h $(SRC)defs.h $(SRC)extern.h $(SRC)regina_t.h $(SRC)configur.h $(SRC)strings.h
GCIHEADERS = $(SRC)gci$(SEP)gci.h $(SRC)gci$(SEP)embedded.h $(SRC)gci$(SEP)gci_convert.h

HAVE_GCI = -dHAVE_GCI -i=$(MYDIR)gci

!ifeq deb Y
ADD_COPT     = -d2 $(ADD_COPT)
ADD_LINKOPT  = debug all $(ADD_LINKOPT)
# ADD_LINKOPT  = debug all library tcpip32 $(ADD_LINKOPT)
!else
ADD_COPT     =  -dNDEBUG -oneatx $(ADD_COPT)
ADD_LINKOPT  =  $(ADD_LINKOPT)
# ADD_LINKOPT  =  library tcpip32  $(ADD_LINKOPT)
!endif

ADD_COPT       =  -4s -wx -wcd=202 -zq -mf $(THREADING) &
                  -dREGINA_VERSION_DATE=$(VER_DATE) -dREGINA_VERSION_MAJOR="$(VER_MAJOR)" &
                  -dREGINA_VERSION_MINOR="$(VER_MINOR)" -dREGINA_VERSION_SUPP="$(VER_SUPP)" &
                  -dREGINA_VERSION_RELEASE="$(VER_RELEASE)" -dREGINA_BITS=32 &
                  -sg -st -dOREXX_BINARY_COMPATIBLE -bt=os2 &
                  -dOS2 -i=$(PATH) -i=$(MYDIR) -i=$(MYDIR).. $(ADD_COPT)
# UNI2H = 1
# NOLIBS = 1

!include $(%ROOT)/mk/appsos2_cmd.mk

.c: $(SRC)

.c: $(MYDIR)

.c: $(MYDIR)..

.c: $(MYDIR)..$(SEP)gci

.h: $(SRC)

.h: $(MYDIR)

.h: $(MYDIR)..

.h: $(MYDIR)..$(SEP)gci

$(PATH)client.$(O): $(SRC)client.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)rexxsaa.$(O): $(SRC)rexxsaa.c $(HFILES) $(SRC)rexxsaa.h $(SRC)rxiface.h

$(PATH)yaccsrc.$(O): $(SRC)yaccsrc.c $(SRC)defs.h $(SRC)rexx.h
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dYYMAXDEPTH=10000 -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)drexx.obj: $(MYDIR)..$(SEP)rexx.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dRXLIB -fr=$^*.err -fo=$^@ $[@

$(PATH)eextstack.obj:  $(SRC)extstack.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@

$(PATH)erexxbif.obj:  $(SRC)rexxbif.c $(HFILES)
 @$(SAY) CC       $^. $(LOG)
 @$(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@
