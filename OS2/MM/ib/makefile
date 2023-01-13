# 

PROJ1 = ib
PROJ = ib1
#TRGT = $(PROJ).exe
DESC = Image Browser
srcfiles = $(p)image$(e)
ADD_COPT = -ecs -dNDEBUG -i=$(MYDIR)contrib$(SEP)include
ADD_LINKOPT = lib clib3r,lvm,helpers,acpi32,mmpm2
DEST = mmos2
#IMPORTS=WinGetFrameTreePPSize PMWIN.972, &
#       WinGetFrameTreePPs PMWIN.973, &
#       DOSQUERYMODFROMEIP DOSCALL1.360 
#UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_pm.mk

TARGETS = $(PATH)$(PROJ1).exe

.rc: $(MYDIR)contrib

.c: $(MYDIR)contrib

.h: $(MYDIR)contrib

$(PATH)$(PROJ1).exe: $(MYDIR)contrib$(SEP)image.rc $(PATH)$(PROJ).exe
 @$(SAY) RESCMP   $^. $(LOG)
 @$(RC) -bt=os2 $(inc) $[@ $]@ -fo=$(PATH)image.res -fe=$@
