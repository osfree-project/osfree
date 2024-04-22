# Makefile for Watcom/LIBC interface layer

PROJ = cmd_shared16
TRGT = $(PROJ).lib
ADD_COPT = -i=$(MYDIR)..$(SEP)..$(SEP)include
NOLIBS      = 1
UNI2H = 0
32_BITS = 0
NOLIBS = 1

srcfiles   =  $(p)cmd_ExecFSEntry$(e) $(p)cmd_Messages$(e) $(p)cmd_setcurrentdisk$(e) &
             $(p)cmd_QueryCurrentDisk$(e) $(p)cmd_QueryFSName$(e) $(p)cmd_ShowVolumeInfo$(e)

!include $(%ROOT)tools/mk/libsos2v1.mk

TARGETS = $(PATH)$(PROJ).lib subdirs

$(PATH)$(PROJ).lib: $(OBJS) $(OBJS16)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

.c: $(MYDIR)..
