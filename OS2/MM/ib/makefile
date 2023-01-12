# 

PROJ = ib
TRGT = $(PROJ).exe
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

#.c: $(MYDIR)contrib

#.h: $(MYDIR)contrib$(SEP)include
