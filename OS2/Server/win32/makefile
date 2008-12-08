# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)lx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)ne$(SEP)ne$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)fixuplx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)loadobjlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)modlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)execlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)debuglx$(e) &

PROJ = os2server_arch
srcfiles = $(p)io$(e) &
           $(p)native_dynlink$(e)

ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libsos2.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
