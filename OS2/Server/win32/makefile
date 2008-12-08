# Makefile for common shared libs

# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)lx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)ne$(SEP)ne$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)fixuplx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)loadobjlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)modlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)execlx$(e) &
# $(p)shared$(SEP)ixfmgr$(SEP)lx$(SEP)debuglx$(e) &

PROJ = os2server_arch
srcfiles = $(p)io$(e) $(p)mmap$(e) 
#           $(p)native_dynlink$(e)

ADD_COPT = -i=..$(SEP)include
TARGETS  = $(PATH)$(PROJ).lib # subdirs

!include $(%ROOT)/mk/libs.mk

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library
