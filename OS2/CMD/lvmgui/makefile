# $Id: makefile,v 1.1 2004/08/16 06:27:05 prokushev Exp $

PROJ = lvmgui
TRGT = $(PROJ).exe
DESC = Enhanced OW assembler
srcfiles = $(p)lvmpm$(e) &
$(p)lvm_ctls$(e) &
$(p)logging$(e) &
$(p)disk$(e) &
$(p)partition$(e) &
$(p)volume$(e) &
$(p)utils$(e) &
$(p)airboot$(e) &
$(p)bootmgr$(e)

DIRS = contrib2 cmd
ADD_COPT = -ecs -dNDEBUG -i=$(MYDIR)contrib$(SEP)include
ADD_LINKOPT = lib clib3r,lvm,helpers,acpi32
IMPORTS=WinGetFrameTreePPSize PMWIN.972, &
       WinGetFrameTreePPs PMWIN.973, &
       DOSQUERYMODFROMEIP DOSCALL1.360 
#UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_pm.mk

.c: $(MYDIR)contrib

.h: $(MYDIR)contrib$(SEP)include
