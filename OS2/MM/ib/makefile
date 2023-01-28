# 

PROJ1 = ib
PROJ = ib1
#TRGT = $(PROJ).exe
DESC = Image Browser
srcfiles = $(p)image$(e)
inc = -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(PORT_BASE)
ADD_COPT += $(inc)
ADD_LINKOPT  = lib mmpm2
PM           = 1
#IMPORTS=WinGetFrameTreePPSize PMWIN.972, &
#       WinGetFrameTreePPs PMWIN.973, &
#       DOSQUERYMODFROMEIP DOSCALL1.360 
#UNI2H = 1
DEST = mmos2

!include $(%ROOT)tools/mk/appsos2_pm.mk
!include $(MYDIR)ib.mk


TARGETS = $(PATH)$(PROJ1).exe

.SUFFIXES: .dlg

.rc: $(PORT_BASE)

.c: $(PORT_BASE)

.h: $(PORT_BASE)

.dlg: $(PORT_BASE)

.ico: $(PORT_BASE)

$(PATH)$(PROJ1).exe: $(PORT_BASE)image.rc $(PATH)$(PROJ).exe
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -bt=os2 $(inc) $[@ $]@ -fo=$(PATH)image.res -fe=$@
