#
#
#
#

# enable debug
deb      = Y

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
             $(p)instore$(e) $(p)extlib$(e) $(p)yaccsrc$(e) $(p)lexsrc$(e) $(p)wrappers$(e) $(p)options$(e) &
             $(p)doscmd$(e) $(p)extstack$(e) $(p)rexxbif$(e)

OFILES =      &
$(genfiles)          &
$(p)mt_notmt$(e)  &

DLOFILES =    &
$(genfiles)          &
$(p)$(THREADING_FILE)$(e) &
$(GCI_SHO)

RXOBJS    = $(OFILES) $(p)rexx$(e) $(p)nosaa$(e)
REGOBJS   = $(p)regina$(e)
EXCZOBJS  = $(p)execiser$(e)
LIBOBJS   = $(OFILES) $(p)client$(e) $(p)drexx$(e) $(p)rexxsaa$(e)
RXSTKOBJS = $(p)rxstack$(e) $(p)erexxbif$(e) $(p)eextstack$(e)
RXQUEOBJS = $(p)rxqueue$(e) $(p)erexxbif$(e) $(p)eextstack$(e)

DLLOBJS   = $(DLOFILES) $(p)client$(e) $(p)drexx$(e) $(p)rexxsaa$(e) $(GCI_SHO) //
DT1OBJS   = $(p)test1$(e)                                                       // .dlo
DT2OBJS   = $(p)test2$(e)                                                       //

!ifeq deb Y
ADD_COPT    += -d2
ADD_LINKOPT += DEBUG WATCOM ALL LIBRARY TCPIP32
!else
ADD_COPT    +=  -dNDEBUG -oneatx
ADD_LINKOPT += LIBRARY TCPIP32
!endif

ADD_COPT    += -sg -st -dOREXX_BINARY_COMPATIBLE -bt=os2 -4s -wx -zq -mf

srcfiles    = $(RXOBJS) # -dRXLIB
ADD_COPT    += -sg -st -bt=os2 -dFGC -dOS2 -i=$(MYDIR) -i=$(PATH)
ADD_LINKOPT +=
STUB        = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
OPTIONS     = caseexact

!include $(%ROOT)/mk/appsos2.mk
