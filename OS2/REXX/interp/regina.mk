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

LIBFILE=rexx.lib
DLLBASE=regina
DLLFILE=$(DLLBASE).dll
IMPFILE=$(DLLBASE).lib

DLLTEST1=test1.dll
DLLTEST2=test2.dll

!ifeq MULTI N
THREADING =
THREADING_LINK =
THREADING_FILE = mt_notmt
!else
THREADING = -bm -dREGINA_MULTI
THREADING_LINK = -bm
THREADING_FILE = mt_os2
!endif

#TARGETS  = $(PATH)rexx.exe # $(PATH)$(LIBFILE) $(PATH)regina.exe $(PATH)test1.dll &
#           $(PATH)test2.dll $(PATH)execiser.exe $(PATH)rxstack.exe # $(PATH)rxqueue.exe

HFILES = $(SRC)rexx.h $(SRC)defs.h $(SRC)extern.h $(SRC)regina_t.h $(SRC)configur.h $(SRC)strings.h
GCIHEADERS = $(SRC)gci$(SEP)gci.h $(SRC)gci$(SEP)embedded.h $(SRC)gci$(SEP)gci_convert.h

#
# GCI is default
#
!ifeq GCI N
GCI_SHO =
HAVE_GCI =
!else
GCI_SHO = $(p)gci_call$(e) $(p)gci_convert$(e) $(p)gci_execute$(e) $(p)gci_helper$(e) &
          $(p)gci_prepare$(e) $(p)gci_reginabridge$(e) $(p)gci_rxfuncdefine$(e) &
          $(p)gci_tree$(e)
HAVE_GCI = -dHAVE_GCI -i=$(MYDIR)gci
!endif
             # $(p)dbg$(e)
genfiles =   $(p)funcs$(e) $(p)builtin$(e) $(p)error$(e) $(p)variable$(e) $(p)interprt$(e) &
             $(p)debug$(e) $(p)dbgfuncs$(e) $(p)memory$(e) $(p)parsing$(e) $(p)files$(e) $(p)misc$(e)  &
             $(p)unxfuncs$(e) $(p)arxfuncs$(e) $(p)os2funcs$(e) $(p)cmsfuncs$(e) $(p)shell$(e) $(p)rexxext$(e) &
             $(p)stack$(e) $(p)tracing$(e) $(p)interp$(e) $(p)cmath$(e) $(p)convert$(e) $(p)strings$(e) &
             $(p)library$(e) $(p)strmath$(e) $(p)signals$(e) $(p)macros$(e) $(p)envir$(e) $(p)expr$(e)  &
             $(p)instore$(e) $(p)yaccsrc$(e) $(p)lexsrc$(e) $(p)wrappers$(e) $(p)options$(e) &
             $(p)doscmd$(e) $(p)extstack$(e) $(p)rexxbif$(e) # $(p)extlib$(e)

OFILES =      &
$(genfiles)          &
$(p)mt_notmt$(e)

DLOFILES =    &
$(genfiles)          &
$(p)$(THREADING_FILE)$(e) &
$(GCI_SHO)

REGOBJS   = $(p)regina$(e)
EXCZOBJS  = $(p)execiser$(e)
LIBOBJS   = $(OFILES) $(p)client$(e) $(p)drexx$(e) $(p)rexxsaa$(e)
RXSTKOBJS = $(p)rxstack$(e) $(p)erexxbif$(e) $(p)eextstack$(e)
RXQUEOBJS = $(p)rxqueue$(e) $(p)erexxbif$(e) $(p)eextstack$(e)

DLLOBJS   = $(DLOFILES) $(p)client$(e) $(p)drexx$(e) $(p)rexxsaa$(e) $(GCI_SHO)
DT1OBJS   = $(p)test1$(e)
DT2OBJS   = $(p)test2$(e)

!ifeq deb Y
ADD_COPT     = -d2 $(ADD_COPT)
ADD_LINKOPT  = debug all library tcpip32 $(ADD_LINKOPT)
!else
ADD_COPT     =  -dNDEBUG -oneatx $(ADD_COPT)
ADD_LINKOPT  =  library tcpip32  $(ADD_LINKOPT)
!endif

ADD_COPT       =  -4s -wx -zq -mf $(THREADING) &
                  -dREGINA_VERSION_DATE=$(VER_DATE) -dREGINA_VERSION_MAJOR="$(VER_MAJOR)" &
                  -dREGINA_VERSION_MINOR="$(VER_MINOR)" -dREGINA_VERSION_SUPP="$(VER_SUPP)" &
                  -sg -st -dOREXX_BINARY_COMPATIBLE -bt=os2 &
                  -dOS2 -i=$(PATH) -i=$(MYDIR) -i=$(MYDIR).. $(ADD_COPT)


!include $(%ROOT)/mk/appsos2.mk

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
 @$(CC)  $(COPT) -dYYMAXDEPTH=10000 -fr=$^*.err -fo=$^@ $[@ $(LOG)

$(PATH)drexx.obj: $(MYDIR)..$(SEP)rexx.c $(HFILES)
 $(CC) $(COPT) -dRXLIB -fr=$^*.err -fo=$^@ $[@

$(PATH)eextstack.obj:  $(SRC)extstack.c $(HFILES)
 $(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@

$(PATH)erexxbif.obj:  $(SRC)rexxbif.c $(HFILES)
 $(CC) $(COPT) -dEXTERNAL_TO_REGINA  -fr=$^*.err -fo=$^@ $[@
