# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

# 

PROJ = os2server_shared
srcfiles = $(p)modmgr$(e) &
$(p)token$(e) &
$(p)ixfmgr$(e) &
$(p)memmgr$(e) &
$(p)cfgparser$(e) &
$(p)processlx$(e) &
$(p)ixfmgr_lx_fixuplx$(e) &
$(p)ixfmgr_lx_loadobjlx$(e) &
$(p)ixfmgr_lx_execlx$(e) &
$(p)ixfmgr_lx$(e) &
$(p)ixfmgr_lx_debuglx$(e) &
$(p)ixfmgr_lx_modlx$(e) &
$(p)ixfmgr_ne$(e)

ADD_COPT = -i=..$(SEP)include -i$(%WATCOM)$(SEP)lh -za
TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libs.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
