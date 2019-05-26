#
# A Makefile for lvm.lib
# (c) osFree project,
# author, date
#

PROJ     = devmapper
srcfiles = $(p)libdm-file$(e) $(p)libdm-deptree$(e) $(p)libdm-string$(e) $(p)libdm-report$(e) $(p)libdm-common$(e) 
ADD_COPT = -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 &
           -i=$(MYDIR) -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)ioctl -bd
TARGETS  = $(PATH)$(PROJ).lib
DEST     = none

!include $(%ROOT)/mk/libsos2.mk

OBJS += $(PATH)datastruct$(SEP)datastruct.lib $(PATH)mm$(SEP)mm.lib $(PATH)regex$(SEP)regex.lib $(PATH)ioctl$(SEP)ioctl.lib

$(TARGETS): $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(TARGETS) library

.c: $(MYDIR)

.c: $(MYDIR)datastruct

.c: $(MYDIR)mm

.c: $(MYDIR)regex

.c: $(MYDIR)ioctl
