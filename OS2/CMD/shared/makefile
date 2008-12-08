# $Id: makefile,v 1.3 2004/08/21 14:49:03 prokushev Exp $
#
# Makefile for Watcom/LIBC interface layer

DIRS = msg
PROJ = cmd_shared
ADD_COPT = -i=$(MYDIR)..$(SEP)include

# For performance/compatibility reasons, we build a "_System" and
# "_Optlink"-like libraries, corresponding to "-5r" and "-5s" options of
# Watcom compiler.
srcfiles   = $(p)cmd_ExecFSEntry$(e) $(p)cmd_Messages$(e) &
             $(p)cmd_QueryCurrentDisk$(e) $(p)cmd_QueryFSName$(e) $(p)cmd_ShowVolumeInfo$(e)
srcfiles16 = $(p)cmd_FSEntry16$(e)

TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS) $(OBJS16)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
